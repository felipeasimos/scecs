// Sparse set C ECS (SCECS)

typedef struct {
	unsigned int index;
	unsigned int version;
} Entity;

#define PAGE_SIZE 4096
#define PAGE_SIZE_MASK PAGE_SIZE - 1

typedef struct {
	// 1 - based indices (0 value means NULL)
	unsigned int dense_idxs[PAGE_SIZE];
	unsigned int len;
} SparseSetPage;

typedef struct {
	SparseSetPage* pages;
	unsigned int pages_len;
	unsigned int *dense;
	unsigned int dense_len;
	unsigned int dense_capacity;
} SparseSet;

int SS_getPageIndex(unsigned int sparse_index) {
	// 12 is the number of bits in PAGE_SIZE_MASK
	return sparse_index >> 12;
}

int SS_getPageInnerIndex(unsigned int sparse_index) {
	return sparse_index & PAGE_SIZE_MASK;
}

SparseSetPage* SS_getPage(SparseSet* set, unsigned sparse_index) {
	unsigned int page_index = SS_getPageIndex(sparse_index);
	SparseSetPage* page = NULL;
	if(page_index >= set->pages_len) {
		return NULL;
	}
	if(page_index < set->pages_len) {
		page = &set->pages[page_index];
		return page;
	}
	return NULL;
}

SparseSetPage* SS_getOrCreatePage(SparseSet* set, unsigned sparse_index) {
	SparseSetPage* page = SS_getPage(set, sparse_index);
	if(!page) {
		unsigned int page_index = SS_getPageIndex(sparse_index);
		unsigned int diff = page_index - set->pages_len + 1;
		set->pages = (SparseSetPage*)realloc(set->pages, (set->pages_len + diff) * sizeof(SparseSetPage));
		for(unsigned int i = 0; i < diff; i++) {
			set->pages[i + set->pages_len].len = 0;
		}
		set->pages_len += diff;
		page = &set->pages[set->pages_len-1];
		page->len = 0;
	}
	return page;
}

int SS_contains(SparseSet* set, unsigned int sparse_index) {
	SparseSetPage* page = SS_getPage(set, sparse_index);
	if(!page) return 0;
	unsigned int page_inner_index = SS_getPageInnerIndex(sparse_index);
	if(page_inner_index >= page->len) return 0;
	return page->dense_idxs[page_inner_index];
}

// return whether we had to reallocate
int SS_add(SparseSet *set, unsigned int sparse_index) {
	SparseSetPage* page = SS_getOrCreatePage(set, sparse_index);
	unsigned int page_inner_index = SS_getPageInnerIndex(sparse_index);
	page->dense_idxs[page_inner_index] = set->dense_len + 1;
	if(set->dense_len == set->dense_capacity) {

		unsigned int new_capacity = (set->dense_capacity << 1) + 1;
		set->dense = (unsigned int*)realloc(set->dense, (new_capacity) * sizeof(unsigned int));

		set->dense_len += 1;
		set->dense_capacity = new_capacity;
		set->dense[set->dense_len-1] = sparse_index;
		return 1;
	}
	set->dense[set->dense_len-1] = sparse_index;
	return 0;
}

int SS_remove(SparseSet *set, unsigned int sparse_index) {
	// set index in sparse location to 0
	SparseSetPage* page = SS_getPage(set, sparse_index);
	if(!page) return 0;
	unsigned int page_inner_index = SS_getPageInnerIndex(sparse_index);
	if(page_inner_index >= page->len) return 0;
	unsigned int dense_index = page->dense_idxs[page_inner_index] - 1;
	if(dense_index >= set->dense_len) return 0;
	page->dense_idxs[page_inner_index] = 0;

	// swap remove dense 
	sparse_index = set->dense[dense_index] = set->dense[set->dense_len-1];
	page = SS_getPage(set, sparse_index);
	page_inner_index = SS_getPageInnerIndex(sparse_index);
	page->dense_idxs[page_inner_index] = dense_index + 1;
	set->dense_len -= 1;
	return dense_index;
}

void SS_deinit(SparseSet *set) {
	free(set->pages);
	free(set->dense);
}

void SS_init(SparseSet* set) {
	set->dense = 0;
	set->dense_len = set->dense_capacity = 0;
	set->pages = 0;
	set->pages_len = 0;
}


#define SPARSE_SET(component) \
	typedef struct { \
		SparseSet set; \
		component* data; \
	} SparseSet ## component; \
	void component ## Add(SparseSet ## component* set, component data, unsigned int sparse_index) { \
		if(!SS_add(&set->set, sparse_index)) return; \
		set->data = (component*)realloc(set->data, (set->set.dense_capacity) * sizeof(component)); \
		set->data[set->set.dense_len - 1] = data; \
	} \
	void component ## Remove(SparseSet ## component* set, unsigned int sparse_index) { \
		unsigned int dense_index = SS_remove(&set->set, sparse_index); \
		set->data[dense_index] = set->data[set->set.dense_len]; \
	} \
	component* component ## Get(SparseSet ## component* set, unsigned int sparse_index) { \
		unsigned int dense_index = SS_contains(&set->set, sparse_index); \
		return &set->data[dense_index]; \
	} \
	void component ## Init(SparseSet ## component* set) { \
		SS_init(&set->set); \
		set->data = 0; \
	} \
	void component ## Deinit(SparseSet ## component* set) { \
		SS_deinit(&set->set); \
		free(set->data); \
	}


// declare sparse set types
#undef COMPONENT
#define COMPONENT(component) SPARSE_SET(component);

COMPONENTS

// declare sparse set fields
#undef COMPONENT
#define COMPONENT(component) SparseSet ## component SparseSet ## component;

typedef struct {
	COMPONENTS

	Entity* ids;
	unsigned int ids_len;
	unsigned int* free_list;
	unsigned int free_list_len;
	unsigned int free_list_capacity;
} World;

void worldInit(World* world) {
	world->ids_len = 0;
	world->free_list_len = world->free_list_capacity = 0;
#undef COMPONENT
#define COMPONENT(component) component ## Init (&(world-> SparseSet ## component));
	COMPONENTS
}

void worldDeinit(World* world) {
#undef COMPONENT
#define COMPONENT(component) component ## Deinit (&(world-> SparseSet ## component));
	COMPONENTS
}


Entity entityCreate(World* world) {
	if(world->free_list_len) {
		unsigned int free_list_index = world->free_list[world->free_list_len-1];
		Entity new_entt = {
			.index = free_list_index,
			.version = world->ids[free_list_index].version,
		};
		world->ids[free_list_index].version = 0;
		return new_entt;
	}
	unsigned int new_index = world->ids_len;
	world->ids_len += 1;
	Entity new_entt = {
		.index = new_index,
		.version = 0,
	};
	return new_entt;
}

int entityValid(World* world, Entity entt) {
	if(entt.index >= world->ids_len) {
		return 0;
	}
	return world->ids[entt.index].version == entt.version;
}

#define WORLD_INIT(world_ptr) worldInit(world_ptr)
#define WORLD_DEINIT(world_ptr) worldDeinit(world_ptr)

#define ENTITY_CREATE(world_ptr) entityCreate(world_ptr)

#define ENTITY_VALID(world_ptr, entt) entityValid(world_ptr, entt)

#define ENTITY_GET(world_ptr, entt, component) component ## Get(&(world_ptr)->SparseSet ## component, entt.index)

#define ENTITY_GET_ASSERT(world_ptr, entt, component) entityValid(world_ptr, entt) ; component ## Get(&(world_ptr)->(SparseSet ## component), entt.index)

#define ENTITY_ADD(world_ptr, entt, component, data) component ## Add(&(world_ptr)-> SparseSet ## component, data, entt.index)

#define ENTITY_REMOVE(world_ptr, entt, component) component ## Remove(&(world_ptr)->(SparseSet ## component), entt.index)
#define ENTITY_REMOVE_ASSERT(world_ptr, entt, component) entityValid(world_ptr, entt) ; component ## Remove(&(world_ptr)->(SparseSet ## component), entt.index)
