#include "core/ecs/ecs.h"

#include <collections/DynamicArray.h>

/// @brief the returned ComponentType needs to be stored so accessing its instance is possible
/// @param world
/// @param componentId
/// @param size
/// @return
ComponentType ecs_register_component(World *world, Mask componentId, u16 size)
{
    static u8 componentTypesCount = 0;
    world->pools[componentTypesCount] = (ComponentPool){
        .components = __DynamicArray_create(1, size),
        .componentSize = size,
        .dense = DynamicArray_Create(EntityID),
        .sparse = {INVALID_ENTITY}};
    componentTypesCount++;
    return (u64)1 << (componentTypesCount - 1);
}

void *ecs_get_component(World *w, EntityID e, ComponentType t)
{
    if (!ecs_entity_has_component(w, e, t))
        return 0;
    ComponentPool p = w->pools[__builtin_ffsll(t) - 1];
    return &p.components[p.sparse[e]];
};

void ecs_add_component(World *w, EntityID e, ComponentType t, void *component)
{
    ComponentPool p = w->pools[__builtin_ffsll(t) - 1];
    p.sparse[e] = DynamicArray_Length(p.components);
    DynamicArray_Push(p.dense, e);
    DynamicArray_Push(p.components, component);
}

void ecs_remove_component(World *w, EntityID e, ComponentType t)
{
    ComponentPool p = w->pools[__builtin_ffsll(t) - 1];
    DynamicArray_PopAt(p.dense, p.sparse[e], 0);
    DynamicArray_PopAt(p.components, p.sparse[e], 0);
    p.sparse[e] = -1;
}

void ecs_remove_all_components(World *w, EntityID e)
{
    Mask s = w->EntitiesSignatures[e];
    while (s) {
        // find the first 1 bit from the right
        int pos = __builtin_ffsll(s) - 1;

        Mask c = (Mask)1 << pos;
        ecs_remove_component(w, e, c);

        // Clear this bit from signature
        s &= ~c;
    }
    w->EntitiesSignatures[e] = s;
}