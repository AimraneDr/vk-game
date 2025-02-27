#include "ecs/ecs.h"

#include <collections/LinkedList.h>
#include "core/debugger.h"

EntityID newEntity(Scene* s){
    if (s->freeEntitiesCount == 0) {
        return INVALID_ENTITY;
    }
    return s->freeEntities[--s->freeEntitiesCount];
}

void destroyEntity(Scene* s, EntityID e){
    if (s->freeEntitiesCount < MAX_ENTITIES) {
        s->freeEntities[s->freeEntitiesCount++] = e;
    }
    ecs_remove_all_components(s,e);
}

bool ecs_entity_has_component(Scene* s, EntityID e, ComponentType t)
{
    return (s->EntitiesSignatures[e] & t) == t;
}