#include "core/ecs/ecs.h"

#include <collections/LinkedList.h>
#include <collections/DynamicArray.h>

void ecs_init(World *out)
{
    out->freeEntities = LinkedList_create();
    out->entitiesCount = 0;
    //TODO : Edit the dynamic array implementation to accept an optional max size
    // in our case, setting a max capacity for the dArray indures that we are not allocating more memory than needed
    // since MAX_ENTITIES is known, we do not need the array expand beyond it because the new size won't be used
    out->systems = DynamicArray_Create(System);
    for (u16 i = 0; i < MAX_ENTITIES; i++)
    {
        out->EntitiesSignatures[i] = 0;
        LinkedListAppend(&out->freeEntities, &i);
    }
    for(u8 i=0; i< MAX_COMPONENT_TYPES; i++){
        out->pools[i] = (ComponentPool){0};
    }
}

void enc_clean_component_pool(World* w, ComponentPool* p){
    for(u16 i=0; i < DynamicArray_Length(p->components); i++){
        EntityID e = MAX_ENTITIES; 
        DynamicArray_Pop(p->dense,&e);
        DynamicArray_Pop(p->components,0);
        p->sparse[e] = INVALID_ENTITY;
    }
}
void ecs_shutdown(World *world)
{
    //clean pools
    for(u8 i=0; i < MAX_COMPONENT_TYPES; i++){
        enc_clean_component_pool(world, &world->pools[i]);
    }
    LinkedList_clean(&world->freeEntities);
    DynamicArray_Destroy(world->systems);
}