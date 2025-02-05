#ifndef ECS_SYSTEM_H
#define ECS_SYSTEM_H

#include "./../ecs_types.h"

void ecs_register_system();
void ecs_systems_initialize();
void ecs_systems_update();
void ecs_systems_shutdown();

#endif //ECS_SYSTEM_H