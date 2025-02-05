#include "engine/core/ecs/ecs.h"

#include "collections/LinkedList.h"
#include "engine/core/debugger.h"

EntityID newEntity(World* w){
    if(w->freeEntities.length == 0){
        LOG_ERROR("Can't create more entities, you reached the max count of entities allowed!");
    }
    w->entitiesCount++;
    return (EntityID)LinkedList_pop(&w->freeEntities)->data; 
}

void destroyEntity(World* w, EntityID e){
    LinkedListPush(&w->freeEntities, &e);
    w->entitiesCount--;
    ecs_remove_all_components(w,e);
}

bool ecs_entity_has_component(World *w, EntityID e, ComponentType t)
{
    return w->EntitiesSignatures[e] && t == t;
}