#include "components/UI/uiComponents.h"

#include "core/debugger.h"

//TODO: encapsulate in a local memory management system
#include <stdlib.h>

typedef enum ChildrenArrayProperty_e{
    Capacity,
    Length,
    MaxProperties
}ChildrenArrayProperty;

UI_Element* createChildrenArray();
void destroyChildrenArray(UI_Element* arr);
u64 getChildrenArrayLength(UI_Element* arr);
void setChildrenArrayLength(UI_Element* arr, u64 new_len);
u64 getChildrenArrayCapacity(UI_Element* arr);
UI_Element* resizeChildrenArray(UI_Element* arr);
void pushChild(UI_Element** arr, UI_Element* child);
void popChild(UI_Element* arr, UI_Element* out);
void popChildAt(UI_Element* arr, u32 i, UI_Element* out);
void swapChildren(UI_Element* arr, u32 i0, u32 i1);


u32 ui_elementChildrenCount(UI_Element* element){
    return getChildrenArrayLength(element->children);
}

void ui_appendChild(UI_Element* parent, UI_Element* child){
    pushChild(parent->children, child);
}

UI_Element ui_createElement(UI_Style s){
    UI_Element new = {
        .parent = 0,
        .children = createChildrenArray(),
        .style = s,
        .order = 0,
        .renderer = {0}
    };
    // pushChild(&parent->children, &new);
    return new;
}

void ui_destroyElement(UI_Element* e, Renderer* r){
    // destroy sub childre arrays
    // TODO: make the darray use arenas so destroying all sub elements can be done in one step
    if(e->children != 0){
        for(i32 i=getChildrenArrayLength(e->children)-1; i >= 0; i--){
            ui_destroyElement(&e->children[i], r);
        }
    }
    // pop it from parent
    if(e->parent != 0){
        if(getChildrenArrayLength(e->parent->children)-1 == e->order){
            popChild(e->parent->children, 0);
        }else popChildAt(e->parent->children, e->order, 0);
    }

    //destroy this element
    //free mem if allocated
    if(e->renderer.indicesCount != 0){
        vkDeviceWaitIdle(r->device);
        e->renderer.indicesCount = 0;
        if(e->renderer.vertexBuffer != 0) vkDestroyBuffer(r->device, e->renderer.vertexBuffer, 0);
        if(e->renderer.vertexBufferMemory != 0) vkFreeMemory(r->device, e->renderer.vertexBufferMemory, 0);
        if(e->renderer.indexBuffer != 0) vkDestroyBuffer(r->device, e->renderer.indexBuffer, 0);
        if(e->renderer.indexBufferMemory != 0) vkFreeMemory(r->device, e->renderer.indexBufferMemory, 0);
    }
}

//internal children DynamicArray
UI_Element* createChildrenArray(){
    u64* arr = malloc(sizeof(u64) * MaxProperties);
    arr[Capacity] = 0;
    arr[Length] = 0;
    return (UI_Element*)(arr + MaxProperties);
}
void destroyChildrenArray(UI_Element* arr){
    u64* header = (u64*)arr - MaxProperties;
    free(header);
}
u64 getChildrenArrayLength(UI_Element* arr){
    u64* header = (u64*)arr - MaxProperties;
    return header[Length];
}
void setChildrenArrayLength(UI_Element* arr, u64 new_len){
    u64* header = (u64*)arr - MaxProperties;
    header[Length] = new_len;
}
u64 getChildrenArrayCapacity(UI_Element* arr){
    u64* header = (u64*)arr - MaxProperties;
    return header[Capacity];
}
UI_Element* resizeChildrenArray(UI_Element* arr){
    u64 headerSize = sizeof(u64*) * MaxProperties;
    u64 newSize = (getChildrenArrayCapacity(arr) * 2) + 1; // handles when capacity is 0
    u64* new = malloc(headerSize + newSize * sizeof(UI_Element));
    new[Capacity] = newSize;
    new[Length] = getChildrenArrayLength(arr);
    memcpy((void*)(new+MaxProperties), arr, new[Length] * sizeof(UI_Element));
    destroyChildrenArray(arr);
    return (UI_Element*)(new+MaxProperties);
}
void pushChild(UI_Element** arr, UI_Element* child){
    u64 len = getChildrenArrayLength(*arr);
    u64 cap = getChildrenArrayCapacity(*arr);
    
    if(len == cap) *arr = resizeChildrenArray(*arr);

    memcpy(&(*arr)[len], child, sizeof(UI_Element));
    setChildrenArrayLength(*arr, len+1);
}
void popChild(UI_Element* arr, UI_Element* out){
    u64 len = getChildrenArrayLength(arr);    
    setChildrenArrayLength(arr, len-1);
    if(out != 0) memcpy(out, &arr[len-1], sizeof(UI_Element));
}
void popChildAt(UI_Element* arr, u32 i, UI_Element* out){
    u64 len = getChildrenArrayLength(arr);
    if (i >= len){ 
        LOG_ERROR("Index out of Childre Array's Bounds");
        return;
    }
    if (out != NULL) memcpy(out, &arr[i], sizeof(UI_Element));
    if (i < len - 1) {
        u64 bytes_to_move = (len - i - 1) * sizeof(UI_Element);
        memmove(&arr[i], &arr[i + 1], bytes_to_move);
    }
    setChildrenArrayLength(arr, len - 1);
}
void swapChildren(UI_Element* arr, u32 i0, u32 i1){
    u64 len = getChildrenArrayLength(arr);
    if(i0 >= len || i1 >= len){
        LOG_ERROR("Index out of Childre Array's Bounds");
        return;
    }
    UI_Element temp = arr[i0];
    memcpy(&arr[i0], &arr[i1], sizeof(UI_Element));
    memcpy(&arr[i1], &temp, sizeof(UI_Element));
}