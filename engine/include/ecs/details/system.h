#ifndef ECS_SYSTEM_H
#define ECS_SYSTEM_H

#include "./../ecs_types.h"

#define ecs_register_system(scene_ptr, system_ptr) ecs_register_system_to_group(scene_ptr, system_ptr, SYSTEM_GROUP_GAME)

API void ecs_register_system_to_group(Scene* s, System *sys, SystemGroup group);

//runtime invokes
API void ecs_systems_initialize(Scene* s);
API void ecs_systems_update(Scene* s, f32 deltatime);
API void ecs_systems_shutdown(Scene* s);


API void ecs_systems_start_group(Scene* s, SystemGroup group);
API void ecs_systems_update_group(Scene* s, f32 deltatime, SystemGroup group);
API void ecs_systems_destroy_group(Scene* s, SystemGroup group);

#endif //ECS_SYSTEM_H