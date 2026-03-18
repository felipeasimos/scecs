#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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


#define COMPONENTS \
	COMPONENT(Position) \
	COMPONENT(Rect) \
	COMPONENT(Velocity) \

#include "scecs.h"

int main() {
	World world;
	WORLD_INIT(&world);
	printf("world created\n");
	Entity entt = ENTITY_CREATE(&world);
	printf("entity created\n");
	Position new_pos = {
		.x = 1.2,
		.y = 2.3,
	};
	ENTITY_ADD(&world, entt, Position, new_pos);
	printf("position added\n");
	Position* just_added = ENTITY_GET(&world, entt, Position);
	printf("just added x: %f, y: %f\n", just_added->x, just_added->y);
	ENTITY_REMOVE(&world, entt, Position);
	WORLD_DEINIT(&world);
}
