#ifndef ECS_TYPES_H
#define ECS_TYPES_H

#include "data_types.h"
#include <string/str_types.h>

#define MAX_ENTITIES (u16)10000
#define INVALID_ENTITY 0xFFFF
#define MAX_COMPONENT_TYPES 64

typedef u16 EntityID;
typedef u64 Mask;
typedef Mask ComponentType;

typedef enum SystemProperty_e{
    SYSTEM_PROPERTY_HIERARCHY_START = 1<<0,
    SYSTEM_PROPERTY_HIERARCHY_PRE_UPDATE = 1<<1,
    SYSTEM_PROPERTY_HIERARCHY_UPDATE = 1<<2,
    SYSTEM_PROPERTY_HIERARCHY_POST_UPDATE = 1<<3,
    SYSTEM_PROPERTY_HIERARCHY_DESTROY = 1<<4,
    SYSTEM_PROPERTY_HIERARCHY_PROCESS = 1<<5,
}SystemProperty;

typedef enum SystemGroup_e{
    SYSTEM_GROUP_RENDERING,
    SYSTEM_GROUP_GAME,
    MAX_SYSTEM_GROUPS
}SystemGroup;

typedef struct SparseSet_t{
    u16 sparse[MAX_ENTITIES];
    EntityID* dense;
}SparseSet;

typedef struct ComponentPool{
    u16* sparse;
    EntityID* dense;         //DynamicArray
    void* components;   //DynamicArray
    u16 componentSize;
    ComponentType type;
}ComponentPool;

typedef struct Scene_t Scene;

typedef void (*SystemFunc)(void* sys_state, void* game_state); 
typedef void (*SystemEntityFunc)(void* sys_state, void* game_state, EntityID e); 

typedef struct System_t{
    const Mask Signature;
    SystemProperty properties;
    void* state;
    
    //functions
    SystemFunc start;
    SystemFunc preUpdate;
    SystemFunc update;
    SystemFunc postUpdate;
    SystemFunc destroy;
    SystemEntityFunc startEntity;
    SystemEntityFunc preUpdateEntity;
    SystemEntityFunc updateEntity;
    SystemEntityFunc postUpdateEntity;
    SystemEntityFunc destroyEntity;
}System;

//TODO: rename to : World, ecs_manager, 
typedef struct Scene_t{
    SparseSet rootEntities; //sparse set
    EntityID* freeEntities; //dynamic list
    u32 freeEntitiesCount;     
    System* systemGroups[MAX_SYSTEM_GROUPS];        //array of dynamic lists
    ComponentPool pools[MAX_COMPONENT_TYPES];
    Mask EntitiesSignatures[MAX_ENTITIES];
    Mask oldEntitiesSignatures[MAX_ENTITIES];

    String componentNames[MAX_COMPONENT_TYPES]; // Track component names
    u8 componentTypesCount;
}Scene;

#endif //ECS_TYPES_H