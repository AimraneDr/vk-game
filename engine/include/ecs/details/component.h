#ifndef ECS_COMPONENT_H
#define ECS_COMPONENT_H

#include "./../ecs_types.h"
#include "engine_defines.h"

#define REGIATER_COMPONENT(scene_ptr, T) ecs_register_component(scene_ptr, #T, sizeof(T))
#define ADD_COMPONENT(scene_ptr, entity, T, ptr) ecs_add_component(scene_ptr, entity, ecs_get_component_type(scene_ptr, #T), ptr)
#define GET_COMPONENT(scene_ptr, entity, T) ecs_get_component(scene_ptr, entity, ecs_get_component_type(scene_ptr, #T))
#define COMPONENT_TYPE(scene_ptr, T) ecs_get_component_type(scene_ptr, #T)

API void ecs_register_component(Scene *scene, const char* name, u16 size);
API ComponentType ecs_get_component_type(Scene* scene, const char* name);
API void *ecs_get_component(Scene* s, EntityID e, ComponentType t);
API void *ecs_add_component(Scene* s, EntityID e, ComponentType t, void *component);
API void ecs_remove_component(Scene* s, EntityID e, ComponentType t);
API void ecs_remove_all_components(Scene* s, EntityID e);

#endif //ECS_COMPONENT_H