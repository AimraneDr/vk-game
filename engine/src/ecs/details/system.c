#include "ecs/ecs.h"

#include <collections/DynamicArray.h>
#include "core/debugger.h"
#include "components/Hierarchy.h"

// TODO: check if group out of bound before operation

void ecs_register_system_to_group(Scene *s, System *sys, SystemGroup group)
{
    DynamicArray_Push(s->systemGroups[group], *sys);
}

bool entity_match_system_signature(Mask eSignature, Mask sSignature)
{
    return (eSignature & sSignature) == sSignature;
}

u8 getSmallestPoolIndex(Scene *s, Mask m)
{
    u8 smallestPool = MAX_COMPONENT_TYPES;
    u32 smaallestSize = MAX_ENTITIES;
    while (m)
    {
        // find the first 1 bit from the right
        u8 pos = __builtin_ffsll(m) - 1;

        if (s->pools[pos].components)
        {
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
EntityID *getTargetedEntities(Scene *s, Mask systemSignature, EntityID *entities)
{
    EntityID *matchedEntities = DynamicArray_Create(EntityID);
    for (u32 j = 0; j < DynamicArray_Length(entities); j++)
    {
        EntityID entity = entities[j];
        if (entity_match_system_signature(s->EntitiesSignatures[entity], systemSignature))
        {
            DynamicArray_Push(matchedEntities, entity);
        }
    }
    return matchedEntities;
}

void ecs_systems_initialize(GameState *gState)
{
    ecs_systems_start_group(gState, SYSTEM_GROUP_GAME);
}
void ecs_systems_pre_update(GameState *gState)
{
    ecs_systems_pre_update_group(gState, SYSTEM_GROUP_GAME);
}
void ecs_systems_update(GameState *gState)
{
    ecs_systems_update_group(gState, SYSTEM_GROUP_GAME);
}
void ecs_systems_post_update(GameState *gState)
{
    ecs_systems_post_update_group(gState, SYSTEM_GROUP_GAME);
}
void ecs_systems_shutdown(GameState *gState)
{
    ecs_systems_destroy_group(gState, SYSTEM_GROUP_GAME);
}

void ecs_systems_start_entity(GameState *gState, EntityID entity)
{
    Scene *scene = &gState->scene;
    for (u8 i = 0; i < MAX_SYSTEM_GROUPS; i++)
    {
        System *systems = scene->systemGroups[i];
        for (u16 i = 0; i < DynamicArray_Length(systems); i++)
        {
            System *sys = &systems[i];
            // old entity signature did not match the systems
            if ((scene->oldEntitiesSignatures[entity] & sys->Signature) != sys->Signature)
            {
                // if new entity's signature matches the system's
                if ((scene->EntitiesSignatures[entity] & sys->Signature) == sys->Signature)
                {
                    if (sys->startEntity)
                        sys->startEntity(sys->state, gState, entity);
                }
            }
        }
    }
}

///helper function
void hierarchy_execute(void *sysState, GameState *gState, SystemEntityFunc func, EntityID *roots)
{
    u16 len = DynamicArray_Length(roots);
    for (u16 i = 0; i < len; i++)
    {
        if (func)
            func(sysState, gState, roots[i]);
        Hierarchy *h = GET_COMPONENT(&gState->scene, roots[i], Hierarchy);
        if (!h || DynamicArray_Length(h->children) == 0)
            continue;
        hierarchy_execute(sysState, gState, func, h->children);
    }
}

void ecs_systems_start_group(GameState *gState, SystemGroup group)
{
    Scene *scene = &gState->scene;
    System *systems = scene->systemGroups[group];
    u32 sysCount = DynamicArray_Length(systems);
    for (u16 i = 0; i < sysCount; i++)
    {
        System *sys = &systems[i];

        if (sys->start)
            sys->start(sys->state, gState);
        if(!sys->startEntity)
            continue;
        
        u8 poolIndex = getSmallestPoolIndex(scene, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }

        ComponentPool *pool = &scene->pools[poolIndex];
        EntityID *targets = 0;
        if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_START | SYSTEM_PROPERTY_HIERARCHY_PROCESS))
        {
            // hierarchy update
            targets = getTargetedEntities(scene, sys->Signature, gState->scene.rootEntities.dense);
            hierarchy_execute(sys->state, gState, sys->startEntity, targets);
        }
        else
        {
            targets = getTargetedEntities(scene, sys->Signature, pool->dense);
            // Process collected entities
            for (u32 j = 0; j < DynamicArray_Length(targets); j++)
            {    
                sys->startEntity(sys->state, gState, targets[j]);
            }
        }

        if(targets) DynamicArray_Destroy(targets);
        targets = 0;
    }
}

void ecs_systems_pre_update_group(GameState *gState, SystemGroup group)
{

    Scene *scene = &gState->scene;
    System *systems = scene->systemGroups[group];
    for (u16 i = 0; i < DynamicArray_Length(systems); i++)
    {
        System *sys = &systems[i];
        if (sys->preUpdate)
            sys->preUpdate(sys->state, gState);
        if (!sys->preUpdateEntity)
            continue;

        u8 poolIndex = getSmallestPoolIndex(scene, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }
        ComponentPool *pool = &scene->pools[poolIndex];

        EntityID *targets = 0;
        if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_PRE_UPDATE | SYSTEM_PROPERTY_HIERARCHY_PROCESS))
        {
            // hierarchy update
            targets = getTargetedEntities(scene, sys->Signature, gState->scene.rootEntities.dense);
            hierarchy_execute(sys->state, gState, sys->preUpdateEntity, targets);
        }
        else
        {
            // linear update
            targets = getTargetedEntities(scene, sys->Signature, pool->dense);
            for (u32 j = 0; j < DynamicArray_Length(targets); j++)
            {
                sys->preUpdateEntity(sys->state, gState, targets[j]);
            }
        }
        if(targets) DynamicArray_Destroy(targets);
        targets = 0;
    }
}

void ecs_systems_update_group(GameState *gState, SystemGroup group)
{
    Scene *scene = &gState->scene;
    System *systems = scene->systemGroups[group];
    for (u16 i = 0; i < DynamicArray_Length(systems); i++)
    {
        System *sys = &systems[i];
        if (sys->update)
            sys->update(sys->state, gState);
        if (!sys->updateEntity)
            continue;

        u8 poolIndex = getSmallestPoolIndex(scene, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }
        ComponentPool *pool = &scene->pools[poolIndex];

        EntityID *targets = 0;
        if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_UPDATE | SYSTEM_PROPERTY_HIERARCHY_PROCESS))
        {
            // hierarchy update
            targets = getTargetedEntities(scene, sys->Signature, gState->scene.rootEntities.dense);
            hierarchy_execute(sys->state, gState, sys->updateEntity, targets);
        }
        else
        {
            // linear update
            targets = getTargetedEntities(scene, sys->Signature, pool->dense);
            for (u32 j = 0; j < DynamicArray_Length(targets); j++)
            {
                if (sys->updateEntity)
                    sys->updateEntity(sys->state, gState, targets[j]);
            }
        }
        if(targets) DynamicArray_Destroy(targets);
        targets = 0;
    }
}


void ecs_systems_post_update_group(GameState *gState, SystemGroup group)
{

    Scene *scene = &gState->scene;
    System *systems = scene->systemGroups[group];
    for (u16 i = 0; i < DynamicArray_Length(systems); i++)
    {
        System *sys = &systems[i];
        if (sys->postUpdate)
            sys->postUpdate(sys->state, gState);
        if (!sys->postUpdateEntity)
            continue;

        u8 poolIndex = getSmallestPoolIndex(scene, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }
        ComponentPool *pool = &scene->pools[poolIndex];

        EntityID *targets = 0;
        if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_POST_UPDATE | SYSTEM_PROPERTY_HIERARCHY_PROCESS))
        {
            // hierarchy update
            targets = getTargetedEntities(scene, sys->Signature, gState->scene.rootEntities.dense);
            hierarchy_execute(sys->state, gState, sys->postUpdateEntity, targets);
        }
        else
        {
            // linear update
            targets = getTargetedEntities(scene, sys->Signature, pool->dense);
            for (u32 j = 0; j < DynamicArray_Length(targets); j++)
            {
                sys->postUpdateEntity(sys->state, gState, targets[j]);
            }
        }
        if(targets) DynamicArray_Destroy(targets);
        targets = 0;
    }
}

void ecs_systems_destroy_group(GameState *gState, SystemGroup group)
{
    Scene *scene = &gState->scene;
    System *systems = scene->systemGroups[group];
    for (u16 i = 0; i < DynamicArray_Length(systems); i++)
    {
        System *sys = &systems[i];
        if (sys->destroyEntity)
        {
            u8 poolIndex = getSmallestPoolIndex(scene, sys->Signature);
            if (poolIndex == MAX_COMPONENT_TYPES)
            {
                LOG_ERROR("Could not fint any of the component types from system signature!");
                return;
            }
            ComponentPool *pool = &scene->pools[poolIndex];

            EntityID *targets = 0;
            if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_DESTROY | SYSTEM_PROPERTY_HIERARCHY_PROCESS))
            {
                // hierarchy update
                targets = getTargetedEntities(scene, sys->Signature, gState->scene.rootEntities.dense);
                hierarchy_execute(sys->state, gState, sys->destroyEntity, targets);
            }
            else
            {
                targets = getTargetedEntities(scene, sys->Signature, pool->dense);
                // Process collected entities
                for (u32 j = 0; j < DynamicArray_Length(targets); j++)
                {
                    if (sys->destroyEntity)
                        sys->destroyEntity(sys->state, gState, targets[j]);
                }
            }
            if(targets) DynamicArray_Destroy(targets);
            targets = 0;
        }

        if (sys->destroy)
            sys->destroy(sys->state, gState);

    }
}