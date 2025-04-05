#include "ecs/ecs.h"

#include <collections/DynamicArray.h>
#include "core/debugger.h"

EntityID newEntity(Scene* s){
    s = s? s : ecs_get_active_scene();

    if (s->freeEntitiesCount == 0) {
        return INVALID_ENTITY;
    }
    EntityID new = s->freeEntities[--s->freeEntitiesCount];
    u16 root_idx = DynamicArray_Length(s->rootEntities.dense);
    s->rootEntities.sparse[new] = root_idx;
    DynamicArray_Push(s->rootEntities.dense, new);

    u16 leaf_idx = DynamicArray_Length(s->leafEntities.dense);
    s->leafEntities.sparse[new] = leaf_idx;
    DynamicArray_Push(s->leafEntities.dense, new);

    return new;
}

void destroyEntity(Scene* s, EntityID e){
    s = s? s : ecs_get_active_scene();

    if(s->rootEntities.sparse[e] != INVALID_ENTITY){
        DynamicArray_PopAt(s->rootEntities.dense, s->rootEntities.sparse[e], 0);
        //apply the shifting to the sparse
        for(u16 i = s->rootEntities.sparse[e]; i < DynamicArray_Length(s->rootEntities.dense); i++)s->rootEntities.sparse[s->rootEntities.dense[i]]-=1;
        s->rootEntities.sparse[e] = INVALID_ENTITY;
    }
    if(s->leafEntities.sparse[e] != INVALID_ENTITY){
        DynamicArray_PopAt(s->leafEntities.dense, s->leafEntities.sparse[e], 0);
        //apply the shifting to the sparse
        for(u16 i = s->leafEntities.sparse[e]; i < DynamicArray_Length(s->leafEntities.dense); i++)s->leafEntities.sparse[s->leafEntities.dense[i]]-=1;
        s->leafEntities.sparse[e] = INVALID_ENTITY;
    }

    if (s->freeEntitiesCount < MAX_ENTITIES) {
        s->freeEntities[s->freeEntitiesCount++] = e;
    }
    ecs_remove_all_components(s,e);
}
bool ecs_entity_has_component(Scene* s, EntityID e, ComponentType t)
{
    s = s? s : ecs_get_active_scene();
    return (s->EntitiesSignatures[e] & t) == t;
}