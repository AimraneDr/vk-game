#include "ecs/ecs.h"

#include <collections/DynamicArray.h>
#include "components/Hierarchy.h"
#include <stdlib.h>
#include "core/events.h"

EVENT_CALLBACK(onComponentAdded){
    EntityID e = eContext.u16[0];
    ecs_systems_start_entity(listener, e);
}

void ecs_init(GameState* gState){
    Scene* out = &gState->scene;
    out->freeEntities = malloc(MAX_ENTITIES * sizeof(EntityID));
    out->freeEntitiesCount = MAX_ENTITIES;
    out->rootEntities.dense = DynamicArray_Create(EntityID);
    for (u32 i = 0; i < MAX_ENTITIES; i++) {
        out->rootEntities.sparse[i] = INVALID_ENTITY;
        out->freeEntities[i] = i;
        out->EntitiesSignatures[i] = 0;
        out->oldEntitiesSignatures[i] = 0;
    }

    for(u8 i=0; i< MAX_COMPONENT_TYPES; i++){
        out->pools[i] = (ComponentPool){0};
    }

    for(u8 i=0; i<MAX_SYSTEM_GROUPS; i++){
        out->systemGroups[i] = DynamicArray_Create(System);
    }

    subscribe_to_event(EVENT_TYPE_COMPONENT_ADDED, &(EventListener){.callback=onComponentAdded, .listener=gState});
}
void ecs_update(Scene* s){
    for (u32 i = 0; i < MAX_ENTITIES; i++) {
        s->oldEntitiesSignatures[i] = s->EntitiesSignatures[i];
    }
}
void ecs_shutdown(Scene* s){
    for(u8 i=0; i<MAX_SYSTEM_GROUPS; i++){
        DynamicArray_Destroy(s->systemGroups[i]);
    }

    DynamicArray_Destroy(s->rootEntities.dense);
    free(s->freeEntities);
    s->freeEntities = 0;
    s->freeEntitiesCount = 0;
}


void ecs_add_child(Scene* s, EntityID parent, EntityID child){
    //pop child from root if it is one
    if(s->rootEntities.sparse[child] != INVALID_ENTITY){
        DynamicArray_PopAt(s->rootEntities.dense, s->rootEntities.sparse[child], 0);
        s->rootEntities.sparse[child] = INVALID_ENTITY;
    }

    Hierarchy* parentH = GET_COMPONENT(s, parent, Hierarchy);
    if(!parentH){
        Hierarchy h = {
            .parent = INVALID_ENTITY,
            .children = DynamicArray_Create(EntityID),
            .outdated = false
        };
        parentH = ADD_COMPONENT(s, parent, Hierarchy, &h);
    }

    DynamicArray_Push(parentH->children, child);

    // Ensure child has a HierarchyComponent
    Hierarchy* childH = GET_COMPONENT(s, child, Hierarchy);
    if (!childH) {
        Hierarchy h = {
            .parent = parent,
            .children = DynamicArray_Create(EntityID),
            .outdated = false
        };
        childH = ADD_COMPONENT(s, child, Hierarchy, &h);
    } else {
        Hierarchy* oldParent = GET_COMPONENT(s, childH->parent, Hierarchy);
        if(oldParent){
            for(u16 i=0; i<DynamicArray_Length(oldParent->children); i++){
                if(oldParent->children[i] == child){
                    DynamicArray_PopAt(oldParent->children, i, 0);
                }
            }
        }
        childH->parent = parent;
    }
}

void ecs_remove_child(Scene* s, EntityID parent, EntityID child){
    Hierarchy* parentH = GET_COMPONENT(s, parent, Hierarchy);
    if(!parentH)return;

    u32 childrenCount = DynamicArray_Length(parentH->children);
    for(u32 i=0; i< childrenCount; i++){
        if(parentH->children[i] == child){
            DynamicArray_PopAt(parentH->children, i, 0);
            break;
        }
    }

    Hierarchy* childH = GET_COMPONENT(s, child, Hierarchy);
    if(childH){
        //TODO: remove all sub children
        childH->parent = INVALID_ENTITY;
    }
}