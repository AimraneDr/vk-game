#ifndef ECS_H
#define ECS_H

#include "./ecs_types.h"
#include "./details/component.h"
#include "./details/entity.h"
#include "./details/system.h"

void ecs_init(GameState* gState);
void ecs_update(Scene* s);
void ecs_shutdown(Scene* s);

API void ecs_add_child(Scene* s, EntityID parent, EntityID child);
API void ecs_remove_child(Scene* s, EntityID parent, EntityID child);

#endif //ECS_H