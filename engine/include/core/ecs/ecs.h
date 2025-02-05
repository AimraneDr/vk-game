#ifndef ECS_H
#define ECS_H

#include "./ecs_types.h"
#include "./details/entity.h"
#include "./details/component.h"
#include "./details/system.h"

void ecs_init(World* out);
void ecs_shutdown(World* world);

#endif //ECS_H