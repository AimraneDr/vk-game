#ifndef ECS_ENTITY_H
#define ECS_ENTITY_H

#include "./../ecs_types.h"

EntityID newEntity(World* w);
bool ecs_entity_has_component(World *w, EntityID e, ComponentType t);
void destroyEntity(World* w, EntityID e);

#endif //ECS_ENTITY_H