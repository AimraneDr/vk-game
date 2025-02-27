#include "ecs/ecs.h"

#include <collections/DynamicArray.h>
#include <stdlib.h>
#include "core/debugger.h"
#include <string/str.h>


/// @brief the returned ComponentType needs to be stored so accessing its instance is possible
/// @param scene
/// @param size
/// @return
void ecs_register_component(Scene *scene, const char* name, u16 size)
{
    if(scene->componentTypesCount >= MAX_COMPONENT_TYPES){
        LOG_ERROR("cannot register component, you hit the max component types count");
        return;
    }
    ComponentPool* pool = &scene->pools[scene->componentTypesCount];
    pool->components = __DynamicArray_create(DARRAY_INITIAL_CAPACITY, size);
    pool->componentSize = size;
    pool->dense = DynamicArray_Create(EntityID);
    pool->sparse = malloc(sizeof(u16) * MAX_ENTITIES);
    pool->type = 1 << scene->componentTypesCount;
    for(u16 i=0; i < MAX_ENTITIES; i++){
        pool->sparse[i]= INVALID_ENTITY;
    }
    scene->componentNames[scene->componentTypesCount] = str_new(name);
    scene->componentTypesCount++;
}

ComponentType ecs_get_component_type(Scene* scene, const char* name) {
    for (u8 i = 0; i < scene->componentTypesCount; i++) {
        if (str_compare_val(scene->componentNames[i], name) == 0) {
            return (u64)1 << i;
        }
    }
    return 0;
}

void *ecs_get_component(Scene* s, EntityID e, ComponentType t)
{
    if (!ecs_entity_has_component(s, e, t))
        return 0;
    ComponentPool* p = &s->pools[__builtin_ffsll(t) - 1];
    u16 index = p->sparse[e];
    if(index == INVALID_ENTITY || index >= DynamicArray_Length(p->components))
        return 0;
    return (char*)p->components + (index * p->componentSize);
};

void ecs_add_component(Scene* s, EntityID e, ComponentType t, void *component)
{
    if((s->EntitiesSignatures[e] & t) == t){
        LOG_ERROR("entity already has component");
        return;
    }
    ComponentPool* p = &s->pools[__builtin_ffsll(t) - 1];
    p->sparse[e] = DynamicArray_Length(p->components);
    DynamicArray_Push(p->dense, e);
    p->components = __DynamicArray_push(p->components, component);
    s->EntitiesSignatures[e] |= t;
}

void ecs_remove_component(Scene* s, EntityID e, ComponentType t)
{
    ComponentPool* p = &s->pools[__builtin_ffsll(t) - 1];
    DynamicArray_PopAt(p->dense, p->sparse[e], 0);
    DynamicArray_PopAt(p->components, p->sparse[e], 0);
    p->sparse[e] = -1;
}

void ecs_remove_all_components(Scene* s, EntityID e)
{
    Mask m = s->EntitiesSignatures[e];
    while (m) {
        // find the first 1 bit from the right
        int pos = __builtin_ffsll(m) - 1;

        Mask c = (Mask)1 << pos;
        ecs_remove_component(s, e, c);

        // Clear this bit from signature
        m &= ~c;
    }
    s->EntitiesSignatures[e] = m;
}