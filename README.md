## SCECS

> Sparse set C ECS

Simple single header sparse set ECS library with good DX made possible by the use of the X macro trick

## How to use it

1. In your code, list your components in a `#define COMPONENTS` block filled with `COMPONENT`s like this and then `#include` the header afterwards:

```
// component type definitions ...

#define COMPONENTS \
	COMPONENT(Position) \
	COMPONENT(Rect) \
	COMPONENT(Velocity) \
	COMPONENT(Kind) \
	COMPONENT(Color)

#includ "scecs.h"
```

2. Setup `World` init and deinit calls:

```
World world;
WORLD_INIT(&world);

// code

WORLD_DEINIT(&world);
```

3. Now you are ready to use the ECS in your code:

* **Create** new entities with `entityCreate(World* world)`
* **Add** new components with the `Add` functions: ex.: `PositionAdd(World* world, Entity entt, Position pos)`
* **Remove** components with the `Remove` functions: ex.: `PositionRemove(World* world, Entity entt)`
* **Get** a pointer to the component value using `Get` functions: ex.: `PositionGet(World* world, Entity entt)`
* Check if an entity **Has** a component using the `Has` functions: ex.: `PositionHas(World* world, Entity entt)`

* **Iterate** over entities with a certain component by using the `EntityIterator` functions:

```
EntityIterator rects = RectEntityIterator(&world);
Entity entt;
while(entityNext(&rects, &entt)) { // returns 0 when iteration is over
   // entt is the current id
}
```
