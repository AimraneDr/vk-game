#pragma once

#include "engine_defines.h"
#include "data_types.h"
#include "ecs/ecs_types.h"
#include <math/mathTypes.h>

Vec4 ui_builder_canvas_padding_get(void);
void ui_builder_canvas_padding_set(Vec4 pad);
Vec2 ui_builder_canvas_positon_get();
void ui_builder_canvas_positon_set(Vec2 pos);
Vec2 ui_builder_canvas_size_get();
void ui_builder_canvas_size_set(Vec2 size);
u32 ui_builder_canvas_resolution_get();
void ui_builder_canvas_resolution_set(u32 pixels_per_point);
void ui_builder_canvas_flush();

API void ui_builder_reset_tree(void);
API void ui_builder_open_element(EntityID elem);
API void ui_builder_close_element();

/// @brief uses the virtual tree constructed by opening and closing elements to calculate the look of the UI
API void ui_builder_build();