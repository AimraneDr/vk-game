#ifndef HIERARCHY_COMPONENT_H
#define HIERARCHY_COMPONENT_H
#include "ecs/ecs_types.h"

typedef struct Hierarchy_t{
    EntityID parent;
    EntityID* children;
    bool outdated;
    u32 depth_level;
}Hierarchy;


#endif //HIERARCHY_COMPONENT_H