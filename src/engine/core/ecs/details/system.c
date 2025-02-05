#include "engine/core/ecs/ecs.h"

#include "collections/DynamicArray.h"
#include "engine/core/debugger.h"

void ecs_register_system(World *w, System *s)
{
    DynamicArray_Push(w->systems, s);
}

bool entity_match_system_signature(Mask eSignature, Mask sSignature)
{
    return eSignature & sSignature == sSignature;
}

u8 getSmallestPoolIndex(World *w, Mask s)
{
    u8 smallestPool = MAX_COMPONENT_TYPES;
    u32 smaallestSize = MAX_ENTITIES;
    while (s)
    {
        // find the first 1 bit from the right
        u8 pos = __builtin_ffsll(s) - 1;

        u32 size = DynamicArray_Length(w->pools[pos].components);
        if (size < smaallestSize)
        {
            smallestPool = pos;
            smaallestSize = size;
        }
        // Clear this bit from signature
        s &= ~(1 << pos);
    }
    return smallestPool;
}
EntityID *getTargetedEntities(World *w, Mask systemSignature, ComponentPool *pool)
{
    EntityID *matchedEntities = DynamicArray_Create(EntityID);
    for (u32 j = 0; j < DynamicArray_Length(pool->dense); j++)
    {
        EntityID entity = pool->dense[j];
        if (entity_match_system_signature(w->EntitiesSignatures[entity], systemSignature))
        {
            DynamicArray_Push(matchedEntities, entity);
        }
    }
    return matchedEntities;
}

void ecs_systems_initialize(World *w)
{
    for (u16 i = 0; i < DynamicArray_Length(w->systems); i++)
    {
        System *s = &w->systems[i];
        u8 poolIndex = getSmallestPoolIndex(w, s->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
        }
        ComponentPool *pool = &w->pools[poolIndex];
        // //  Option1
        // for(u32 i =0; i<DynamicArray_Length(pool->dense); i++){
        //     if(entity_match_system_signature(w->EntitiesSignatures[pool->dense[i]], s->Signature))
        //     {
        //         s->init(pool->dense[i]);
        //     }
        // }

        //  Option 2

        EntityID *targets = getTargetedEntities(w, s->Signature, pool);
        // Process collected entities
        for (u32 j = 0; j < DynamicArray_Length(targets); j++)
        {
            s->init(targets[j]);
        }

        DynamicArray_Destroy(targets);
    }
}

void ecs_systems_update(World *w)
{
    for (u16 i = 0; i < DynamicArray_Length(w->systems); i++)
    {
        System *s = &w->systems[i];
        u8 poolIndex = getSmallestPoolIndex(w, s->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
        }
        ComponentPool *pool = &w->pools[poolIndex];
        // //  Option1
        // for(u32 i =0; i<DynamicArray_Length(pool->dense); i++){
        //     if(entity_match_system_signature(w->EntitiesSignatures[pool->dense[i]], s->Signature))
        //     {
        //         s->update(pool->dense[i]);
        //     }
        // }

        //  Option 2

        EntityID *targets = getTargetedEntities(w, s->Signature, pool);
        // Process collected entities
        for (u32 j = 0; j < DynamicArray_Length(targets); j++)
        {
            s->update(targets[j]);
        }

        DynamicArray_Destroy(targets);
    }
}

void ecs_systems_shutdown(World *w)
{
    for (u16 i = 0; i < DynamicArray_Length(w->systems); i++)
    {
        System *s = &w->systems[i];
        u8 poolIndex = getSmallestPoolIndex(w, s->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
        }
        ComponentPool *pool = &w->pools[poolIndex];
        // //  Option1
        // for(u32 i =0; i<DynamicArray_Length(pool->dense); i++){
        //     if(entity_match_system_signature(w->EntitiesSignatures[pool->dense[i]], s->Signature))
        //     {
        //         s->shutdown(pool->dense[i]);
        //     }
        // }

        //  Option 2

        EntityID *targets = getTargetedEntities(w, s->Signature, pool);
        // Process collected entities
        for (u32 j = 0; j < DynamicArray_Length(targets); j++)
        {
            s->shutdown(targets[j]);
        }

        DynamicArray_Destroy(targets);
    }
}