#include "ecs/ecs.h"

#include <collections/DynamicArray.h>
#include "core/debugger.h"

//TODO: check if group out of bound before operation

void ecs_register_system_to_group(Scene* s, System *sys, SystemGroup group)
{
    DynamicArray_Push(s->systemGroups[group], *sys);
}

bool entity_match_system_signature(Mask eSignature, Mask sSignature)
{
    return (eSignature & sSignature) == sSignature;
}

u8 getSmallestPoolIndex(Scene* s, Mask m)
{
    u8 smallestPool = MAX_COMPONENT_TYPES;
    u32 smaallestSize = MAX_ENTITIES;
    while (m)
    {
        // find the first 1 bit from the right
        u8 pos = __builtin_ffsll(m) - 1;

        if(s->pools[pos].components){
            u32 size = DynamicArray_Length(s->pools[pos].components);
            if (size < smaallestSize)
            {
                smallestPool = pos;
                smaallestSize = size;
            }
        }
        // Clear this bit from signature
        m &= ~(1 << pos);
    }
    return smallestPool;
}
EntityID *getTargetedEntities(Scene* s, Mask systemSignature, ComponentPool *pool)
{
    EntityID *matchedEntities = DynamicArray_Create(EntityID);
    for (u32 j = 0; j < DynamicArray_Length(pool->dense); j++)
    {
        EntityID entity = pool->dense[j];
        if (entity_match_system_signature(s->EntitiesSignatures[entity], systemSignature))
        {
            DynamicArray_Push(matchedEntities, entity);
        }
    }
    return matchedEntities;
}

void ecs_systems_initialize(Scene* s){
    ecs_systems_start_group(s, SYSTEM_GROUP_GAME);
}
void ecs_systems_update(Scene* s, f32 deltatime){
    ecs_systems_update_group(s, deltatime, SYSTEM_GROUP_GAME);
}
void ecs_systems_shutdown(Scene* s){
    ecs_systems_destroy_group(s, SYSTEM_GROUP_GAME);
}

void ecs_systems_start_group(Scene* s, SystemGroup group)
{
    System* systems = s->systemGroups[group];
    u32 sysCount = DynamicArray_Length(systems);
    for (u16 i = 0; i < sysCount; i++)
    {
        System *sys = &systems[i];
        
        if(sys->start) sys->start(sys->state, s);

        u8 poolIndex = getSmallestPoolIndex(s, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }
        
        ComponentPool *pool = &s->pools[poolIndex];
        EntityID *targets = getTargetedEntities(s, sys->Signature, pool);
        // Process collected entities
        for (u32 j = 0; j < DynamicArray_Length(targets); j++)
        {
            if(sys->startEntity) sys->startEntity(sys->state, s, targets[j]);
        }

        DynamicArray_Destroy(targets);
    }
}

void ecs_systems_update_group(Scene* s, f32 deltatime, SystemGroup group)
{
    System* systems = s->systemGroups[group];
    for (u16 i = 0; i < DynamicArray_Length(systems); i++)
    {
        System *sys = &systems[i];
        if(sys->update) sys->update(sys->state, s, deltatime);

        u8 poolIndex = getSmallestPoolIndex(s, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }
        ComponentPool *pool = &s->pools[poolIndex];

        EntityID *targets = getTargetedEntities(s, sys->Signature, pool);
        // Process collected entities

        for (u32 j = 0; j < DynamicArray_Length(targets); j++)
        {
            if(sys->updateEntity) sys->updateEntity(sys->state, s, targets[j], deltatime);
        }

        DynamicArray_Destroy(targets);
        targets = 0;
    }
}

void ecs_systems_destroy_group(Scene* s, SystemGroup group)
{
    System* systems = s->systemGroups[group];
    for (u16 i = 0; i < DynamicArray_Length(systems); i++)
    {
        System *sys = &systems[i];
        u8 poolIndex = getSmallestPoolIndex(s, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }
        ComponentPool *pool = &s->pools[poolIndex];
        EntityID *targets = getTargetedEntities(s, sys->Signature, pool);
        // Process collected entities
        for (u32 j = 0; j < DynamicArray_Length(targets); j++)
        {
            if(sys->destroyEntity) sys->destroyEntity(sys->state, s, targets[j]);
        }

        if(sys->destroy) sys->destroy(sys->state, s);

        DynamicArray_Destroy(targets);
    }
}