#ifndef ECS_SYSTEM_H
#define ECS_SYSTEM_H

#include "./../ecs_types.h"
#include "game.h"

#define ecs_register_system(scene_ptr, system_ptr) ecs_register_system_to_group(scene_ptr, system_ptr, SYSTEM_GROUP_GAME)

API void ecs_register_system_to_group(Scene* s, System *sys, SystemGroup group);

//runtime invokes
API void ecs_systems_initialize(GameState* gState, Scene* scene);
API void ecs_systems_pre_update(GameState* gState, Scene* scene);
API void ecs_systems_update(GameState* gState, Scene* scene);
API void ecs_systems_post_update(GameState* gState, Scene* scene);
API void ecs_systems_shutdown(GameState* gState, Scene* scene);

/// @brief runs at the start of entity's life
void ecs_systems_start_entity(GameState* gState, Scene* scene, EntityID entity);

API void ecs_systems_start_group(GameState* gState, Scene* scene, SystemGroup group);
API void ecs_systems_pre_update_group(GameState *gState, Scene* scene, SystemGroup group);
API void ecs_systems_update_group(GameState* gState, Scene* scene, SystemGroup group);
API void ecs_systems_post_update_group(GameState *gState, Scene* scene, SystemGroup group);
API void ecs_systems_destroy_group(GameState* gState, Scene* scene, SystemGroup group);

#endif //ECS_SYSTEM_H