#pragma once

#include "ecs/ecs_types.h"
#include "components/camera.h"

/// @brief this system is responsible for calculating the ui and updating its logic
/// @param scene 
/// @return system instance 
System UI_manager_get_system_ref(Scene* scene);