#ifndef UI_MANAGER_COMPONENT_H
#define UI_MANAGER_COMPONENT_H

#include "engine_defines.h"
#include "components/UI/ui_types.h"
#include "components/UI/details/container.h"

API void ui_createRootElement(UI_Manager* manager);
API u32 ui_elementChildrenCount(UI_Element* element);
API void ui_appendChild(UI_Element* parent, UI_Element* child);

#endif //UI_MANAGER_COMPONENT_H