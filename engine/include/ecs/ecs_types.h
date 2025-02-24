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

typedef enum SystemGroup_e{
    SYSTEM_GROUP_RENDERING,
    SYSTEM_GROUP_GAME,
    MAX_SYSTEM_GROUPS
}SystemGroup;

typedef struct ComponentPool{
    u16* sparse;
    u16* dense;         //DynamicArray
    void* components;   //DynamicArray
    u16 componentSize;
}ComponentPool;

typedef struct Scene_t Scene;

typedef struct System_t{
    const Mask Signature;
    void* state;
    
    //functions
    void (*start)(void* _state, Scene* scene);
    void (*startEntity)(void* _state, Scene* scene, EntityID e);
    void (*preUpdate)(void* _state, Scene* scene, f32 deltatime);
    void (*preUpdateEntity)(void* _state, Scene* scene, EntityID e, f32 deltatime);
    void (*update)(void* _state, Scene* scene, f32 deltatime);
    void (*updateEntity)(void* _state, Scene* scene, EntityID e, f32 deltatime);
    void (*postUpdate)(void* _state, Scene* scene, f32 deltatime);
    void (*postUpdateEntity)(void* _state, Scene* scene, EntityID e, f32 deltatime);
    void (*destroy)(void* _state, Scene* scene);
    void (*destroyEntity)(void* _state, Scene* scene, EntityID e);
}System;

//TODO: rename to : World, ecs_manager, 
typedef struct Scene_t{
    EntityID* freeEntities; //dynamic list
    u32 freeEntitiesCount;     
    System* systemGroups[MAX_SYSTEM_GROUPS];        //array of dynamic lists
    ComponentPool pools[MAX_COMPONENT_TYPES];
    Mask EntitiesSignatures[MAX_ENTITIES];

    String componentNames[MAX_COMPONENT_TYPES]; // Track component names
    u8 componentTypesCount;
}Scene;

#endif //ECS_TYPES_H