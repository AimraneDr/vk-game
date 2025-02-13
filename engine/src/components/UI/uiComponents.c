#include "components/UI/uiComponents.h"

#include <collections/DynamicArray.h>
#include "core/debugger.h"

//TODO: costumize the dynamic array for the ui.

void ui_createRootElement(UI_Manager* manager){
    manager->root = (UI_Element){0};
    manager->root.children = DynamicArray_Create(UI_Element);
    LOG_DEBUG("root element created");
}

u32 ui_elementChildrenCount(UI_Element* element){
    return DynamicArray_Length(element->children);
}

void ui_appendChild(UI_Element* parent, UI_Element* child){
    DynamicArray_Push(parent->children, *child);
}