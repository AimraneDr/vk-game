#include "ecs/ecs.h"

#include <collections/DynamicArray.h>
#include "components/Hierarchy.h"
#include <stdlib.h>
#include "core/events.h"

static Scene* activeScene;

EVENT_CALLBACK(onComponentAdded){
    EntityID e = eContext.u16[0];
    ecs_systems_start_entity(listener, sender ? sender : ecs_get_active_scene(), e);
}

void ecs_init(GameState* gState){
    Scene* out = &gState->scene;
    out->freeEntities = malloc(MAX_ENTITIES * sizeof(EntityID));
    out->freeEntitiesCount = MAX_ENTITIES;
    out->rootEntities.dense = DynamicArray_Create(EntityID);
    out->leafEntities.dense = DynamicArray_Create(EntityID);
    for (u32 i = 0; i < MAX_ENTITIES; i++) {
        out->rootEntities.sparse[i] = INVALID_ENTITY;
        out->leafEntities.sparse[i] = INVALID_ENTITY;
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
    s = s? s : activeScene;
    for (u32 i = 0; i < MAX_ENTITIES; i++) {
        s->oldEntitiesSignatures[i] = s->EntitiesSignatures[i];
    }
}
void ecs_shutdown(Scene* s){
    s = s? s : activeScene;
    for(u8 i=0; i<MAX_SYSTEM_GROUPS; i++){
        DynamicArray_Destroy(s->systemGroups[i]);
    }

    DynamicArray_Destroy(s->rootEntities.dense);
    DynamicArray_Destroy(s->leafEntities.dense);
    free(s->freeEntities);
    s->freeEntities = 0;
    s->freeEntitiesCount = 0;
}

void ecs_set_active_scene(Scene* s){
    activeScene = s;
}

Scene* ecs_get_active_scene(){
    return activeScene;
}


/// @brief update the connection between parent and child
/// @param s 
/// @param parent if INVALID_ENTITY : make the child a root entity
/// @param child if INVALID_ENTITY : make all the child a root entity and the parent a leaf entity 
void ecs_move_entity(Scene* s, EntityID parent, EntityID child){
    s = s? s : activeScene;

    //pop child from root if it is one
    if(s->rootEntities.sparse[child] != INVALID_ENTITY){
        DynamicArray_PopAt(s->rootEntities.dense, s->rootEntities.sparse[child], 0);
        s->rootEntities.sparse[child] = INVALID_ENTITY;
    }
    if(parent && s->leafEntities.sparse[parent] != INVALID_ENTITY){
        DynamicArray_PopAt(s->leafEntities.dense, s->leafEntities.sparse[parent], 0);
        s->leafEntities.sparse[parent] = INVALID_ENTITY;
    }

    Hierarchy* parentH = GET_COMPONENT(s, parent, Hierarchy);
    if(!parentH){
        Hierarchy h = {
            .parent = INVALID_ENTITY,
            .children = DynamicArray_Create(EntityID),
            .outdated = false,
            .depth_level = 0
        };
        parentH = ADD_COMPONENT(s, parent, Hierarchy, &h);
    }

    DynamicArray_Push(parentH->children, child);

    // Ensure child has a HierarchyComponent
    Hierarchy* childH = GET_COMPONENT(s, child, Hierarchy);
    if (!childH) {
        // add hierarchy component to child
        Hierarchy h = {
            .parent = parent,
            .children = DynamicArray_Create(EntityID),
            .outdated = false,
            .depth_level = parentH->depth_level + 1
        };
        childH = ADD_COMPONENT(s, child, Hierarchy, &h);
    } else {
        //if child has parent : move the child to the new parent
        Hierarchy* oldParent = GET_COMPONENT(s, childH->parent, Hierarchy);
        if(oldParent){
            for(u16 i=0; i<DynamicArray_Length(oldParent->children); i++){
                if(oldParent->children[i] == child){
                    DynamicArray_PopAt(oldParent->children, i, 0);
                }
            }
            //if no children are left make a leaf 
            if(DynamicArray_Length(oldParent->children) ==0){
                u16 idx = DynamicArray_Length(s->leafEntities.dense);
                s->leafEntities.sparse[childH->parent] = idx;
                DynamicArray_Push(s->leafEntities.dense, childH->parent);
            }
        }
        childH->parent = parent;
        childH->depth_level = parentH->depth_level + 1;
    }
}