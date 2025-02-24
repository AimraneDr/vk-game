#include "ecs/ecs.h"

#include <collections/DynamicArray.h>
#include <stdlib.h>

void ecs_init(Scene* out){
    out->freeEntities = malloc(MAX_ENTITIES * sizeof(EntityID));
    out->freeEntitiesCount = MAX_ENTITIES;
    for (u32 i = 0; i < MAX_ENTITIES; i++) {
        out->freeEntities[i] = i;
    }

    for(u8 i=0; i< MAX_COMPONENT_TYPES; i++){
        out->pools[i] = (ComponentPool){0};
    }

    for(u8 i=0; i<MAX_SYSTEM_GROUPS; i++){
        out->systemGroups[i] = DynamicArray_Create(System);
    }
}

void ecs_shutdown(Scene* s){
    for(u8 i=0; i<MAX_SYSTEM_GROUPS; i++){
        DynamicArray_Destroy(s->systemGroups[i]);
    }
    free(s->freeEntities);
    s->freeEntities = 0;
    s->freeEntitiesCount = 0;
}