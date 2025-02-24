#ifndef ECS_H
#define ECS_H

#include "./ecs_types.h"
#include "./details/component.h"
#include "./details/entity.h"
#include "./details/system.h"

void ecs_init(Scene* out);
void ecs_shutdown(Scene* s);
#endif //ECS_H