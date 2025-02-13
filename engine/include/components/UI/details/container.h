#ifndef UI_DETAILS_CONTAINER_H
#define UI_DETAILS_CONTAINER_H

#include "engine_defines.h"
#include "components/UI/ui_types.h"

API UI_Element* ui_create_container(UI_Element* parent, Transform2D transform, UI_Style style, Renderer* r);

#endif //UI_DETAILS_CONTAINER_H