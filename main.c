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

typedef enum {
	Player,
	Enemy,
	Bullet,
} Kind;


#define COMPONENTS \
	COMPONENT(Position) \
	COMPONENT(Rect) \
	COMPONENT(Velocity) \
	COMPONENT(Kind) \
	COMPONENT(Color)

#include "scecs.h"
#include <time.h>
#include <math.h>

#define RECT_SIZE 10.0

Entity setupPlayer(World* world) {
	Entity player_entt = entityCreate(world);
	Position initial_player_pos = {
		.x = GetScreenWidth() / 2.0,
		.y = GetScreenHeight() / 2.0,
	};
	PositionAdd(world, player_entt, initial_player_pos);
	KindAdd(world, player_entt, Player);
	Rect r = {
		.x = RECT_SIZE,
		.y = RECT_SIZE,
	};
	ColorAdd(world, player_entt, BLUE);
	RectAdd(world, player_entt, r);
	return player_entt;
}

void setupEnemy(World* world) {
	Entity enemy_entt = entityCreate(world);
	Position initial_player_pos = {
		.x = ((float)rand() / (float)RAND_MAX) * GetScreenWidth(),
		.y = ((float)rand() / (float)RAND_MAX) * GetScreenHeight(),
	};
	PositionAdd(world, enemy_entt, initial_player_pos);
	KindAdd(world, enemy_entt, Enemy);
	Rect r = {
		.x = RECT_SIZE,
		.y = RECT_SIZE,
	};
	ColorAdd(world, enemy_entt, RED);
	RectAdd(world, enemy_entt, r);
	Velocity v = {
		.x = ((float)rand() / (float)RAND_MAX),
		.y = ((float)rand() / (float)RAND_MAX),
	};

	VelocityAdd(world, enemy_entt, v);
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

int main() {
	srand(time(0));
	World world;
	WORLD_INIT(&world);
	SetTargetFPS(120);
	InitWindow(400, 300, "SCECS");

	unsigned int num_enemies = 10;
	Entity player_entt = setupPlayer(&world);
	for(unsigned int i = 0; i < num_enemies; i++) {
		setupEnemy(&world);
	}


	while(!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);

		renderingSystem(&world);
		velocitySystem(&world);

		DrawFPS(1, 1);
		EndDrawing();
	}


	CloseWindow();

	WORLD_DEINIT(&world);
}
