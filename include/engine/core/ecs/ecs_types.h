#ifndef ECS_TYPES_H
#define ECS_TYPES_H

#include "engine/data_types.h"
#include "collections/LinkedList.h"

#define MAX_ENTITIES (u16)-1
#define INVALID_ENTITY (u16)-1
#define MAX_COMPONENT_TYPES 64

typedef u16 EntityID;
typedef u64 Mask;
typedef Mask ComponentType;

typedef struct ComponentPool{
    u16 sparse[MAX_ENTITIES];
    u16* dense;         //DynamicArray
    void* components;   //DynamicArray
    u16 componentSize;
}ComponentPool;

typedef struct System{
    const Mask Signature;
    
    //functions
    void (*init)(EntityID e);
    void (*earlyUpdate)(EntityID e);
    void (*update)(EntityID e);
    void (*lateUpdate)(EntityID e);
    void (*shutdown)(EntityID e);
}System;

typedef struct World{
    Mask EntitiesSignatures[MAX_ENTITIES];
    ComponentPool pools[MAX_COMPONENT_TYPES];
    LinkedList freeEntities;
    u16 entitiesCount;
    /// @brief DynamicArray of systems
    System* systems;
}World;


#endif //ECS_TYPES_H