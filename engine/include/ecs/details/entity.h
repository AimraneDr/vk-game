#ifndef ECS_ENTITY_H
#define ECS_ENTITY_H

#include "./../ecs_types.h"
#include "engine_defines.h"

API EntityID newEntity(Scene* s);
API bool ecs_entity_has_component(Scene* s, EntityID e, ComponentType t);
API void destroyEntity(Scene* s, EntityID e);

#endif //ECS_ENTITY_H