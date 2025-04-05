#ifndef UI_MANAGER_COMPONENT_H
#define UI_MANAGER_COMPONENT_H

#include "engine_defines.h"
#include "components/UI/ui_types.h"
#include "components/UI/details/container.h"
#include <math/mathTypes.h>

API UI_Element ui_create_element(UI_Style s);
API void ui_destroy_element(UI_Element* e, Renderer* r);

///////////////////////
///     Getters     ///
///////////////////////
API Vec2 ui_element_get_final_size(UI_Element* elem);

API void ui_canvas_padding(Vec4 pad);
API void ui_canvas_positon(Vec2 pos);
API void ui_canvas_size(Vec2 size);
API void ui_canvas_resolution(u32 pixels_per_point);
API void ui_canvas_flush();

API void ui_open_element(EntityID elem);
API void ui_close_element(void);


#endif //UI_MANAGER_COMPONENT_H