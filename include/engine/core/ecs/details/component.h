#ifndef ECS_COMPONENT_H
#define ECS_COMPONENT_H

#include "./../ecs_types.h"

ComponentType ecs_register_component(World *world, Mask componentId, u16 size);
void *ecs_get_component(World *w, EntityID e, ComponentType t);
void ecs_add_component(World *w, EntityID e, ComponentType t, void *component);
void ecs_remove_component(World *w, EntityID e, ComponentType t);
void ecs_remove_all_components(World *w, EntityID e);

#endif //ECS_COMPONENT_H