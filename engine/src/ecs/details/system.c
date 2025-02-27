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

void ecs_systems_initialize(GameState* gState){
    ecs_systems_start_group(gState, SYSTEM_GROUP_GAME);
}
void ecs_systems_update(GameState* gState){
    ecs_systems_update_group(gState, SYSTEM_GROUP_GAME);
}
void ecs_systems_shutdown(GameState* gState){
    ecs_systems_destroy_group(gState, SYSTEM_GROUP_GAME);
}

void ecs_systems_start_group(GameState* gState, SystemGroup group)
{
    Scene* s = &gState->scene;
    System* systems = s->systemGroups[group];
    u32 sysCount = DynamicArray_Length(systems);
    for (u16 i = 0; i < sysCount; i++)
    {
        System *sys = &systems[i];
        
        if(sys->start) sys->start(sys->state, gState);

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
            if(sys->startEntity) sys->startEntity(sys->state, gState, targets[j]);
        }

        DynamicArray_Destroy(targets);
    }
}

void ecs_systems_update_group(GameState* gState, SystemGroup group)
{
    Scene* scene = &gState->scene;
    System* systems = scene->systemGroups[group];
    for (u16 i = 0; i < DynamicArray_Length(systems); i++)
    {
        System *sys = &systems[i];
        if(sys->update) sys->update(sys->state, gState);

        u8 poolIndex = getSmallestPoolIndex(scene, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }
        ComponentPool *pool = &scene->pools[poolIndex];

        EntityID *targets = getTargetedEntities(scene, sys->Signature, pool);
        // Process collected entities

        for (u32 j = 0; j < DynamicArray_Length(targets); j++)
        {
            if(sys->updateEntity) sys->updateEntity(sys->state, gState, targets[j]);
        }

        DynamicArray_Destroy(targets);
        targets = 0;
    }
}

void ecs_systems_destroy_group(GameState* gState, SystemGroup group)
{
    Scene* scene = &gState->scene;
    System* systems = scene->systemGroups[group];
    for (u16 i = 0; i < DynamicArray_Length(systems); i++)
    {
        System *sys = &systems[i];
        u8 poolIndex = getSmallestPoolIndex(scene, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }
        ComponentPool *pool = &scene->pools[poolIndex];
        EntityID *targets = getTargetedEntities(scene, sys->Signature, pool);
        // Process collected entities
        for (u32 j = 0; j < DynamicArray_Length(targets); j++)
        {
            if(sys->destroyEntity) sys->destroyEntity(sys->state, gState, targets[j]);
        }

        if(sys->destroy) sys->destroy(sys->state, gState);

        DynamicArray_Destroy(targets);
    }
}