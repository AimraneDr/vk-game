#include "ecs/ecs.h"

#include <collections/DynamicArray.h>
#include "core/debugger.h"
#include "components/Hierarchy.h"

// TODO: check if group out of bound before operation

void ecs_register_system_to_group(Scene *s, System *sys, SystemGroup group)
{
    s = s? s : ecs_get_active_scene();
    DynamicArray_Push(s->systemGroups[group], *sys);
}

bool entity_match_system_signature(Mask eSignature, Mask sSignature)
{
    return (eSignature & sSignature) == sSignature;
}

u8 getSmallestPoolIndex(Scene *s, Mask m)
{
    s = s? s : ecs_get_active_scene();

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
    s = s? s : ecs_get_active_scene();
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

void ecs_systems_initialize(GameState *gState, Scene* scene)
{
    ecs_systems_start_group(gState, scene, SYSTEM_GROUP_GAME);
}
void ecs_systems_pre_update(GameState *gState, Scene* scene)
{
    ecs_systems_pre_update_group(gState, scene, SYSTEM_GROUP_GAME);
}
void ecs_systems_update(GameState *gState, Scene* scene)
{
    ecs_systems_update_group(gState, scene, SYSTEM_GROUP_GAME);
}
void ecs_systems_post_update(GameState *gState, Scene* scene)
{
    ecs_systems_post_update_group(gState, scene, SYSTEM_GROUP_GAME);
}
void ecs_systems_shutdown(GameState *gState, Scene* scene)
{
    ecs_systems_destroy_group(gState, scene, SYSTEM_GROUP_GAME);
}

void ecs_systems_start_entity(GameState *gState, Scene* scene, EntityID entity)
{
    scene = scene ? scene : ecs_get_active_scene();
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
                    if (sys->callbacks.startEntity)
                        sys->callbacks.startEntity(sys->state, gState, entity);
                }
            }
        }
    }
}

/// helper function

/// @brief start from roots and go down to leafs
/// @param sysState
/// @param gState
/// @param func
/// @param roots
void hierarchy_execute(void *sysState, GameState *gState, Scene* scene, SystemEntityFunc func, EntityID *roots)
{
    scene = scene ? scene : ecs_get_active_scene();

    u16 len = DynamicArray_Length(roots);
    for (u16 i = 0; i < len; i++)
    {
        if (func)
            func(sysState, gState, roots[i]);
        Hierarchy *h = GET_COMPONENT(scene, roots[i], Hierarchy);
        if (!h || DynamicArray_Length(h->children) == 0)
            continue;
        hierarchy_execute(sysState, gState, scene, func, h->children);
    }
}

/// @brief start from leaf and go up to root
/// @param sysState
/// @param gState
/// @param func
/// @param leaf
void hierarchy_execute_reversed(void *sysState, GameState *gState, Scene* scene, SystemEntityFunc func, EntityID leaf)
{
    scene = scene ? scene : ecs_get_active_scene();

    if (func)
        func(sysState, gState, leaf);

    Hierarchy *h = GET_COMPONENT(scene, leaf, Hierarchy);
    if (!h || h->parent == INVALID_ENTITY)
        return;
    hierarchy_execute_reversed(sysState, gState, scene, func, h->parent);
}

void ecs_systems_start_group(GameState *gState, Scene* scene, SystemGroup group)
{
    scene = scene ? scene : ecs_get_active_scene();

    System *systems = scene->systemGroups[group];
    u32 sysCount = DynamicArray_Length(systems);
    for (u16 i = 0; i < sysCount; i++)
    {
        System *sys = &systems[i];

        if (sys->callbacks.start)
            sys->callbacks.start(sys->state, gState);
        if (!sys->callbacks.startEntity)
            continue;

        u8 poolIndex = getSmallestPoolIndex(scene, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }

        ComponentPool *pool = &scene->pools[poolIndex];
        EntityID *targets = 0;
        if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_START | SYSTEM_PROPERTY_HIERARCHY_START_REVERSED | SYSTEM_PROPERTY_HIERARCHY_PROCESS | SYSTEM_PROPERTY_HIERARCHY_PROCESS_REVERSED))
        {
            // hierarchy update
            if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_START | SYSTEM_PROPERTY_HIERARCHY_PROCESS))
            {
                targets = getTargetedEntities(scene, sys->Signature, gState->scene.rootEntities.dense);
                hierarchy_execute(sys->state, gState, scene, sys->callbacks.startEntity, targets);
            }
            if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_START_REVERSED | SYSTEM_PROPERTY_HIERARCHY_PROCESS_REVERSED))
            {
                if(sys->info) (*sys->info) |= SYSTEM_INFO_REVERSE_CALLBACK;
                targets = getTargetedEntities(scene, sys->Signature, gState->scene.leafEntities.dense);
                for (u32 e_i = 0; e_i < DynamicArray_Length(targets); e_i++)
                {
                    hierarchy_execute_reversed(sys->state, gState, scene, sys->callbacks.startEntity, targets[e_i]);
                }
                if(sys->info) (*sys->info) &= (~SYSTEM_INFO_REVERSE_CALLBACK);
            }
        }
        else
        {
            targets = getTargetedEntities(scene, sys->Signature, pool->dense);
            // Process collected entities
            for (u32 j = 0; j < DynamicArray_Length(targets); j++)
            {
                sys->callbacks.startEntity(sys->state, gState, targets[j]);
            }
        }

        if (targets)
            DynamicArray_Destroy(targets);
        targets = 0;
    }
}

void ecs_systems_pre_update_group(GameState *gState, Scene* scene, SystemGroup group)
{
    scene = scene ? scene : ecs_get_active_scene();

    System *systems = scene->systemGroups[group];
    for (u16 i = 0; i < DynamicArray_Length(systems); i++)
    {
        System *sys = &systems[i];
        if (sys->callbacks.preUpdate)
            sys->callbacks.preUpdate(sys->state, gState);
        if (!sys->callbacks.preUpdateEntity)
            continue;

        u8 poolIndex = getSmallestPoolIndex(scene, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }
        ComponentPool *pool = &scene->pools[poolIndex];

        EntityID *targets = 0;
        if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_PRE_UPDATE | SYSTEM_PROPERTY_HIERARCHY_PRE_UPDATE_REVERSED | SYSTEM_PROPERTY_HIERARCHY_PROCESS | SYSTEM_PROPERTY_HIERARCHY_PROCESS_REVERSED))
        {
            // hierarchy update
            if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_PRE_UPDATE | SYSTEM_PROPERTY_HIERARCHY_PROCESS))
            {
                targets = getTargetedEntities(scene, sys->Signature, gState->scene.rootEntities.dense);
                hierarchy_execute(sys->state, gState, scene, sys->callbacks.preUpdateEntity, targets);
            }
            if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_PRE_UPDATE_REVERSED | SYSTEM_PROPERTY_HIERARCHY_PROCESS_REVERSED))
            {
                if(sys->info) (*sys->info) |= SYSTEM_INFO_REVERSE_CALLBACK;
                targets = getTargetedEntities(scene, sys->Signature, gState->scene.leafEntities.dense);
                for (u32 e_i = 0; e_i < DynamicArray_Length(targets); e_i++)
                {
                    hierarchy_execute_reversed(sys->state, gState, scene, sys->callbacks.preUpdateEntity, targets[e_i]);
                }
                if(sys->info) (*sys->info) &= (~SYSTEM_INFO_REVERSE_CALLBACK);
            }
        }
        else
        {
            // linear update
            targets = getTargetedEntities(scene, sys->Signature, pool->dense);
            for (u32 j = 0; j < DynamicArray_Length(targets); j++)
            {
                sys->callbacks.preUpdateEntity(sys->state, gState, targets[j]);
            }
        }
        if (targets)
            DynamicArray_Destroy(targets);
        targets = 0;
    }
}

void ecs_systems_update_group(GameState *gState,Scene* scene, SystemGroup group)
{
    scene = scene ? scene : ecs_get_active_scene();

    System *systems = scene->systemGroups[group];
    for (u16 i = 0; i < DynamicArray_Length(systems); i++)
    {
        System *sys = &systems[i];
        if (sys->callbacks.update)
            sys->callbacks.update(sys->state, gState);
        if (!sys->callbacks.updateEntity)
            continue;

        u8 poolIndex = getSmallestPoolIndex(scene, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }
        ComponentPool *pool = &scene->pools[poolIndex];

        EntityID *targets = 0;
        if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_UPDATE | SYSTEM_PROPERTY_HIERARCHY_UPDATE_REVERSED | SYSTEM_PROPERTY_HIERARCHY_PROCESS | SYSTEM_PROPERTY_HIERARCHY_PROCESS_REVERSED))
        {
            // hierarchy update
            // parent -> child
            if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_UPDATE | SYSTEM_PROPERTY_HIERARCHY_PROCESS))
            {
                targets = getTargetedEntities(scene, sys->Signature, gState->scene.rootEntities.dense);
                hierarchy_execute(sys->state, gState, scene, sys->callbacks.updateEntity, targets);
            }
            // child -> parent
            if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_UPDATE_REVERSED | SYSTEM_PROPERTY_HIERARCHY_PROCESS_REVERSED))
            {
                if(sys->info) (*sys->info) |= SYSTEM_INFO_REVERSE_CALLBACK;
                targets = getTargetedEntities(scene, sys->Signature, gState->scene.leafEntities.dense);
                for (u32 e_i = 0; e_i < DynamicArray_Length(targets); e_i++)
                {
                    hierarchy_execute_reversed(sys->state, gState, scene, sys->callbacks.updateEntity, targets[e_i]);
                }
                if(sys->info) (*sys->info) &= (~SYSTEM_INFO_REVERSE_CALLBACK);
            }
        }
        else
        {
            // linear update
            targets = getTargetedEntities(scene, sys->Signature, pool->dense);
            for (u32 j = 0; j < DynamicArray_Length(targets); j++)
            {
                sys->callbacks.updateEntity(sys->state, gState, targets[j]);
            }
        }
        if (targets)
            DynamicArray_Destroy(targets);
        targets = 0;
    }
}

void ecs_systems_post_update_group(GameState *gState, Scene* scene, SystemGroup group)
{
    scene = scene ? scene : ecs_get_active_scene();

    System *systems = scene->systemGroups[group];
    for (u16 i = 0; i < DynamicArray_Length(systems); i++)
    {
        System *sys = &systems[i];
        if (sys->callbacks.postUpdate)
            sys->callbacks.postUpdate(sys->state, gState);
        if (!sys->callbacks.postUpdateEntity)
            continue;

        u8 poolIndex = getSmallestPoolIndex(scene, sys->Signature);
        if (poolIndex == MAX_COMPONENT_TYPES)
        {
            LOG_ERROR("Could not fint any of the component types from system signature!");
            return;
        }
        ComponentPool *pool = &scene->pools[poolIndex];

        EntityID *targets = 0;
        if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_POST_UPDATE | SYSTEM_PROPERTY_HIERARCHY_POST_UPDATE_REVERSED | SYSTEM_PROPERTY_HIERARCHY_PROCESS | SYSTEM_PROPERTY_HIERARCHY_PROCESS_REVERSED))
        {
            // hierarchy update
            // parent -> child
            if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_POST_UPDATE | SYSTEM_PROPERTY_HIERARCHY_PROCESS))
            {
                targets = getTargetedEntities(scene, sys->Signature, gState->scene.rootEntities.dense);
                hierarchy_execute(sys->state, gState, scene, sys->callbacks.postUpdateEntity, targets);
            }
            // child -> parent
            if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_POST_UPDATE_REVERSED | SYSTEM_PROPERTY_HIERARCHY_PROCESS_REVERSED))
            {
                if(sys->info) (*sys->info) |= SYSTEM_INFO_REVERSE_CALLBACK;
                targets = getTargetedEntities(scene, sys->Signature, gState->scene.leafEntities.dense);
                for (u32 e_i = 0; e_i < DynamicArray_Length(targets); e_i++)
                {
                    hierarchy_execute_reversed(sys->state, gState, scene, sys->callbacks.postUpdateEntity, targets[e_i]);
                }
                if(sys->info) (*sys->info) &= (~SYSTEM_INFO_REVERSE_CALLBACK);
            }
        }
        else
        {
            // linear update
            targets = getTargetedEntities(scene, sys->Signature, pool->dense);
            for (u32 j = 0; j < DynamicArray_Length(targets); j++)
            {
                sys->callbacks.postUpdateEntity(sys->state, gState, targets[j]);
            }
        }
        if (targets)
            DynamicArray_Destroy(targets);
        targets = 0;
    }
}

void ecs_systems_destroy_group(GameState *gState, Scene* scene, SystemGroup group)
{
    scene = scene ? scene : ecs_get_active_scene();

    System *systems = scene->systemGroups[group];
    for (u16 i = 0; i < DynamicArray_Length(systems); i++)
    {
        System *sys = &systems[i];
        if (sys->callbacks.destroyEntity)
        {
            u8 poolIndex = getSmallestPoolIndex(scene, sys->Signature);
            if (poolIndex == MAX_COMPONENT_TYPES)
            {
                LOG_ERROR("Could not fint any of the component types from system signature!");
                return;
            }
            ComponentPool *pool = &scene->pools[poolIndex];

            EntityID *targets = 0;
            if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_DESTROY | SYSTEM_PROPERTY_HIERARCHY_DESTROY_REVERSED | SYSTEM_PROPERTY_HIERARCHY_PROCESS | SYSTEM_PROPERTY_HIERARCHY_PROCESS_REVERSED))
            {
                // hierarchy update
                if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_DESTROY | SYSTEM_PROPERTY_HIERARCHY_PROCESS))
                {
                    targets = getTargetedEntities(scene, sys->Signature, gState->scene.rootEntities.dense);
                    hierarchy_execute(sys->state, gState, scene, sys->callbacks.destroyEntity, targets);
                }
                if (sys->properties & (SYSTEM_PROPERTY_HIERARCHY_DESTROY_REVERSED | SYSTEM_PROPERTY_HIERARCHY_PROCESS_REVERSED))
                {
                    targets = getTargetedEntities(scene, sys->Signature, gState->scene.leafEntities.dense);
                    if(sys->info) (*sys->info) |= SYSTEM_INFO_REVERSE_CALLBACK;
                    for (u32 e_i = 0; e_i < DynamicArray_Length(targets); e_i++)
                    {
                        hierarchy_execute_reversed(sys->state, gState, scene, sys->callbacks.destroyEntity, targets[e_i]);
                    }
                    if(sys->info) (*sys->info) &= (~SYSTEM_INFO_REVERSE_CALLBACK);
                }
            }
            else
            {
                targets = getTargetedEntities(scene, sys->Signature, pool->dense);
                // Process collected entities
                for (u32 j = 0; j < DynamicArray_Length(targets); j++)
                {
                    if (sys->callbacks.destroyEntity)
                        sys->callbacks.destroyEntity(sys->state, gState, targets[j]);
                }
            }
            if (targets)
                DynamicArray_Destroy(targets);
            targets = 0;
        }

        if (sys->callbacks.destroy)
            sys->callbacks.destroy(sys->state, gState);
    }
}