#ifndef UI_MANAGER_COMPONENT_H
#define UI_MANAGER_COMPONENT_H

#include "engine_defines.h"
#include "components/UI/ui_types.h"
#include "components/UI/details/container.h"

API UI_Element ui_createElement(UI_Style s);
API void ui_destroyElement(UI_Element* e, Renderer* r);

#endif //UI_MANAGER_COMPONENT_H