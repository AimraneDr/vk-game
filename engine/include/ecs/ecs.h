#ifndef ECS_H
#define ECS_H

#include "./ecs_types.h"
#include "./details/component.h"
#include "./details/entity.h"
#include "./details/system.h"

void ecs_init(GameState* gState);
void ecs_update(Scene* s);
void ecs_shutdown(Scene* s);

/// @brief set the active scene
API void ecs_set_active_scene(Scene* s);

/// @return the active scene 
API Scene* ecs_get_active_scene();
API void ecs_move_entity(Scene* s, EntityID parent, EntityID child);

#endif //ECS_H