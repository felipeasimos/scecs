#include "raylib.h"

typedef struct {
	float x;
	float y;
} Position;

typedef struct {
	float x;
	float y;
} Rect;

typedef struct {
	float x;
	float y;
} Velocity;

typedef struct {} RedEnemy;
typedef struct {} Player;
typedef struct {} YellowEnemy;
typedef struct {
	unsigned int counter;
} Timer;


#define COMPONENTS \
	COMPONENT(Position) \
	COMPONENT(Rect) \
	COMPONENT(Velocity) \
	COMPONENT(Player) \
	COMPONENT(RedEnemy) \
	COMPONENT(YellowEnemy) \
	COMPONENT(Color) \
	COMPONENT(Timer)

#include "scecs.h"
#include <time.h>
#include <math.h>

#define FPS 120
#define RECT_SIZE 10.0

Entity createPlayer(World* world) {
	Entity player_entt = entityCreate(world);
	Position initial_player_pos = {
		.x = GetScreenWidth() / 2.0,
		.y = GetScreenHeight() / 2.0,
	};
	PositionAdd(world, player_entt, initial_player_pos);
	Player p = {};
	PlayerAdd(world, player_entt, p);
	Rect r = {
		.x = RECT_SIZE,
		.y = RECT_SIZE,
	};
	ColorAdd(world, player_entt, BLUE);
	RectAdd(world, player_entt, r);
	return player_entt;
}

void createEnemy(World* world) {
	Entity enemy_entt = entityCreate(world);
	Position initial_player_pos = {
		.x = ((float)rand() / (float)RAND_MAX) * GetScreenWidth(),
		.y = ((float)rand() / (float)RAND_MAX) * GetScreenHeight(),
	};
	PositionAdd(world, enemy_entt, initial_player_pos);
	YellowEnemy p = {};
	YellowEnemyAdd(world, enemy_entt, p);
	Rect r = {
		.x = RECT_SIZE,
		.y = RECT_SIZE,
	};
	ColorAdd(world, enemy_entt, YELLOW);
	RectAdd(world, enemy_entt, r);
	Velocity v = {
		.x = ((float)rand() / (float)RAND_MAX),
		.y = ((float)rand() / (float)RAND_MAX),
	};

	VelocityAdd(world, enemy_entt, v);
	Timer timer = {
		.counter = FPS * 3,
	};
	TimerAdd(world, enemy_entt, timer);
}

void renderingSystem(World* world) {
	EntityIterator rects = RectEntityIterator(world);
	Entity entt;
	while(entityNext(&rects, &entt)) {
		if(PositionHas(world, entt) && ColorHas(world, entt)) {
			Position pos = *PositionGet(world, entt);
			Rect rect = *RectGet(world, entt);
			Color color = *ColorGet(world, entt);
			DrawRectangle(pos.x, pos.y, rect.x, rect.y, color);
		}
	}
}

void velocitySystem(World* world) {
	EntityIterator rects = VelocityEntityIterator(world);
	Entity entt;
	while(entityNext(&rects, &entt)) {
		if(PositionHas(world, entt) && RectHas(world, entt)) {
			Velocity* v = VelocityGet(world, entt);
			Position* p = PositionGet(world, entt);
			Rect r = *RectGet(world, entt);
			p->x += v->x;
			p->y += v->y;

			// handle window border collisions
			int w = GetScreenWidth();
			int h = GetScreenHeight();
			if(r.x + p->x >= w) {
				v->x = -fabsf(v->x);
				p->x = w - r.x;
			} else if(p->x <= 0) {
				v->x = fabsf(v->x);
				p->x = 0;
			} else if(p->y + r.y >= GetScreenHeight()) {
				v->y = -fabsf(v->y);
				p->y = h - r.y;
			} else if(p->y <= 0) {
				v->y = fabsf(v->y);
				p->y = 0;
			}
		}
	}
}

void collisionSystem(World* world) {
	EntityIterator p_iter = PlayerEntityIterator(world);
	Entity player_entt;
	assert(entityNext(&p_iter, &player_entt));

	EntityIterator rects = RectEntityIterator(world);
	Entity entt;
	while(entityNext(&rects, &entt)) {
		if(PositionHas(world, entt) && RedEnemyHas(world, entt)) {
			Rect r1 = *RectGet(world, entt);
			Position p1 = *PositionGet(world, entt);
			Rect r2 = *RectGet(world, player_entt);
			Position p2 = *PositionGet(world, player_entt);
			if(p1.x + r1.x < p2.x) continue;
			if(p1.y + r1.y < p2.y) continue;
			if(p1.x > p2.x + r2.x) continue;
			if(p1.y > p2.y + r2.y) continue;
			entityDestroy(world, entt);
			createEnemy(world);
		}
	}
}

void timerSystem(World* world) {
	EntityIterator iter = TimerEntityIterator(world);
	Entity entt;
	while(entityNext(&iter, &entt)) {
		Timer* timer = TimerGet(world, entt);
		timer->counter -= 1;
		if(!timer->counter) {
			TimerRemove(world, entt);
			if(YellowEnemyHas(world, entt) && ColorHas(world, entt)) {
				YellowEnemyRemove(world, entt);
				RedEnemy r = {};
				RedEnemyAdd(world, entt, r);
				Color* color = ColorGet(world, entt);
				*color = RED;
			}
		}
	}
}

void movePlayerSystem(World* world) {
	EntityIterator iter = PlayerEntityIterator(world);
	Entity player_entt;
	assert(entityNext(&iter, &player_entt));
}

int main() {
	srand(time(0));
	World world;
	WORLD_INIT(&world);
	SetTargetFPS(FPS);
	InitWindow(400, 300, "SCECS");

	unsigned int num_enemies = 1024;
	createPlayer(&world);
	for(unsigned int i = 0; i < num_enemies; i++) {
		createEnemy(&world);
	}


	while(!WindowShouldClose()) {

		movePlayerSystem(&world);
		velocitySystem(&world);
		collisionSystem(&world);
		timerSystem(&world);

		BeginDrawing();
		ClearBackground(BLACK);

		renderingSystem(&world);

		DrawFPS(1, 1);
		EndDrawing();
	}


	CloseWindow();

	WORLD_DEINIT(&world);
}
