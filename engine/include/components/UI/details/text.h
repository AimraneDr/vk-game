#pragma once

#include "engine_defines.h"
#include "components/UI/ui_types.h"

API UI_Element ui_create_text(UI_Style style, const char* text);
API void ui_set_text(UI_Element* elem, const char* text);
API void ui_set_text_font(UI_Element* elem, const char* fontName);
