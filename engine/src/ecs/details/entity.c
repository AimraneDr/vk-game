#include "ecs/ecs.h"

#include <collections/DynamicArray.h>
#include "core/debugger.h"

EntityID newEntity(Scene* s){
    if (s->freeEntitiesCount == 0) {
        return INVALID_ENTITY;
    }
    EntityID new = s->freeEntities[--s->freeEntitiesCount];
    u16 i = DynamicArray_Length(s->rootEntities.dense);
    s->rootEntities.sparse[new] = i;
    DynamicArray_Push(s->rootEntities.dense, new);
    return new;
}

void destroyEntity(Scene* s, EntityID e){
    DynamicArray_PopAt(s->rootEntities.dense, s->rootEntities.sparse[e], 0);
    if (s->freeEntitiesCount < MAX_ENTITIES) {
        s->freeEntities[s->freeEntitiesCount++] = e;
    }
    ecs_remove_all_components(s,e);
}
bool ecs_entity_has_component(Scene* s, EntityID e, ComponentType t)
{
    return (s->EntitiesSignatures[e] & t) == t;
}