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
#include <stdio.h>

Entity setupPlayer(World* world) {
	Entity player_entt = entityCreate(world);
	Position initial_player_pos = {
		.x = GetScreenWidth() / 2,
		.y = GetScreenHeight() / 2,
	};
	PositionAdd(world, player_entt, initial_player_pos);
	KindAdd(world, player_entt, Player);
	Rect r = {
		.x = 10.0,
		.y = 10.0,
	};
	RectAdd(world, player_entt, r);
}

int main() {
	InitWindow(400, 300, "SCECS");
	World world;
	WORLD_INIT(&world);
	Entity player_entt = setupPlayer(&world);

	while(!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);



		DrawFPS(1, 1);
		EndDrawing();
	}

	CloseWindow();

	// Entity entt = ENTITY_CREATE(&world);
	// Position new_pos = {
	// 	.x = 1.2,
	// 	.y = 2.3,
	// };
	// PositionAdd(&world, entt, new_pos);
	// printf("position added\n");
	// Position* just_added = PositionGet(&world, entt);
	// printf("just added x: %f, y: %f\n", just_added->x, just_added->y);
	// PositionRemove(&world, entt);
	// // PositionAdd(&world, entt, new_pos);
	// entityDestroy(&world, entt);
	WORLD_DEINIT(&world);
}
