#include "components/UI/details/builder.h"

#include "core/debugger.h"

#include "components/UI/ui_types.h"
#include "components/Hierarchy.h"
#include "ecs/ecs.h"
#include <collections/DynamicArray.h>
#include <math/vec2.h>
#include <math/mathUtils.h>

typedef enum UIPass
{
    UI_PASS_SIZE_CORRECTION,
    UI_PASS_SIZES,
    UI_PASS_FULL_SIZES,
    UI_PASS_POSITIONS
} UIPass;

static UIPass current_pass = UI_PASS_SIZE_CORRECTION;

typedef struct UI_Element_State_t
{
    struct
    {
        f32 width, height;
    } final;
    Vec2 remaining_size;

    u32 children_count;
    u32 remaining_children;
    // points to where to draw next sub element
    Vec2 cursor;
    UISize sizeing;
} UI_Element_State;

typedef struct UI_Canvas_t
{
    Vec2 size;
    Vec2 remainingSize;
    Vec2 position;
    u32 pixelsPerPoint;
    Vec2 cursor;
    UILayout layout;
    Vec4 padding;
    u32 remainingChildren;
    u32 childrenCount;
    Vec2 gap;
    UISize sizeing;
} UI_Canvas;

static UI_Canvas canvas = {0};

typedef struct ElementNode
{
    u32 parent_idx;
    u32 this_idx;
    u32 node_level;
    EntityID this;
    u32 *children;
} ElementNode;

static ElementNode *elements_tree = 0;
static u32 current_node_idx = 0;

#define ELEM_STATE(elem) ((UI_Element_State *)((elem)->internal))

void correctElementParentSizing(UI_Element *elem, UI_Element *parent);
void calculateElementSizing_oppening(UI_Element *elem, UI_Element *parent);
void calculateElementSizing_closer(UI_Element *elem, UI_Element *parent);
void calculateElementFullSizing(UI_Element* elem, UI_Element* parent);
void calculateElementPosition(UI_Element *elem, Transform2D *t, UI_Element *parent);

void openElement(EntityID elem);
void closeElement();

// static EntityID *open_elements = 0;
// static EntityID *waitlist = 0;

void ui_run_pass(UIPass pass);

Vec4 ui_builder_canvas_padding_get(void)
{
    return canvas.padding;
}
void ui_builder_canvas_padding_set(Vec4 pad)
{
    canvas.padding = pad;
}
Vec2 ui_builder_canvas_positon_get()
{
    return canvas.position;
}
void ui_builder_canvas_positon_set(Vec2 pos)
{
    canvas.position = pos;
}
Vec2 ui_builder_canvas_size_get()
{
    return canvas.size;
}
void ui_builder_canvas_size_set(Vec2 size)
{
    canvas.size = size;
}
u32 ui_builder_canvas_resolution_get()
{
    return canvas.pixelsPerPoint;
}
void ui_builder_canvas_resolution_set(u32 pixels_per_point)
{
    canvas.pixelsPerPoint = pixels_per_point;
}
void ui_builder_canvas_flush()
{
    canvas.childrenCount = 0;
    canvas.cursor = vec2_add(canvas.position, vec2_new(canvas.padding.left, canvas.padding.top));
}

Vec2 ui_element_get_final_size(UI_Element *elem)
{
    return vec2_new(ELEM_STATE(elem)->final.width, ELEM_STATE(elem)->final.height);
}

ElementNode *getNode(u32 idx)
{
    return &elements_tree[idx];
}

void ui_builder_reset_tree(void)
{
    // If the tree already exists, free its memory.
    if (elements_tree) {
        // Free children arrays of each node.
        u32 count = DynamicArray_Length(elements_tree);
        for (u32 i = 0; i < count; i++) {
            ElementNode *node = getNode(i);
            if (node->children) {
                DynamicArray_Destroy(node->children);
                node->children = 0;
            }
        }
        // Clear the tree dynamic array.
        DynamicArray_Clear(elements_tree);
    } else {
        // Create a new tree if it doesn't exist.
        elements_tree = DynamicArray_Create(ElementNode);
    }

    // Create the root node (canvas).
    ElementNode canvas = {
        .node_level = 0,
        .parent_idx = 0,    // For the root, you might set this to 0 or an invalid value.
        .this_idx = 0,
        .this = INVALID_ENTITY,
        .children = 0
    };
    DynamicArray_Push(elements_tree, canvas);
    current_node_idx = 0;
}

void ui_builder_open_element(EntityID elem)
{
    ElementNode *current_node = getNode(current_node_idx);

    // add node to current node
    if (!current_node->children)
    {
        current_node->children = DynamicArray_Create(u32);
    }
    u32 new_idx = DynamicArray_Length(elements_tree);
    ElementNode new_node = {
        .node_level = current_node->node_level + 1,
        .parent_idx = current_node->this_idx,
        .this_idx = new_idx,
        .this = elem,
        .children = 0};
    DynamicArray_Push(current_node->children, new_idx);
    DynamicArray_Push(elements_tree, new_node);
    current_node_idx = new_idx;
}

void ui_builder_close_element()
{
    // move to current ptr to parent node
    if (current_node_idx == 0)
        return;
    ElementNode *current_node = getNode(current_node_idx);
    current_node_idx = current_node->parent_idx;
}

void ui_builder_build()
{
    /*
    1 - size correction
        updating the size type of parent elements for a full elem to also be full if parent is fit
    */
    ui_run_pass(UI_PASS_SIZE_CORRECTION);

    /*
    2 - fit sizes calculations
    */
    ui_run_pass(UI_PASS_SIZES);

    /*
    3 - full sizes calculations
    */
    ui_run_pass(UI_PASS_FULL_SIZES);

    /*
    4 - position calculations
    */
    ui_run_pass(UI_PASS_POSITIONS);

    
}

///////////////////////////////
///////////////////////////////
/////      INTERNAL     ///////
///////////////////////////////
///////////////////////////////

void _ui_open_close_tree_branch(ElementNode *node)
{
    openElement(node->this);
    if(node->children){
        for (u32 i = 0; i < DynamicArray_Length(node->children); i++)
        {
            if (DynamicArray_Length(elements_tree) > node->children[i])
            {
                current_node_idx = node->children[i];
                _ui_open_close_tree_branch(&elements_tree[current_node_idx]);
            }
        }
    }
    closeElement();
    current_node_idx = current_node_idx > 0 ? elements_tree[current_node_idx].parent_idx : 0;
}

void ui_run_pass(UIPass pass)
{
    // travers through the logical tree
    // opeening elements and closing them while claculating the pass
    current_pass = CLAMP(pass, UI_PASS_SIZE_CORRECTION, UI_PASS_POSITIONS);
    current_node_idx = 0;
    _ui_open_close_tree_branch(&elements_tree[current_node_idx]);
}

void openElement(EntityID elem)
{
    if (current_node_idx == 0)
    {
        // root / canvas
        canvas.childrenCount = DynamicArray_Length(getNode(current_node_idx)->children);
        canvas.remainingChildren = canvas.childrenCount;
        // temp
        canvas.sizeing = UI_SIZE_FULL;
        canvas.remainingSize = vec2_sub(canvas.size, vec2_new(canvas.padding.right+canvas.padding.left, canvas.padding.top + canvas.padding.bottom));
        // skip
        return;
    }

    UI_Element *current = GET_COMPONENT(0, elem, UI_Element);
    if (!current->internal)
    {
        current->internal = malloc(sizeof(UI_Element_State));
    }
    ELEM_STATE(current)->children_count = getNode(current_node_idx)->children ? DynamicArray_Length(getNode(current_node_idx)->children) : 0;
    
    ElementNode *currentN = getNode(current_node_idx);
    EntityID parentE = getNode(currentN->parent_idx)->this;
    UI_Element * parent =  parentE == INVALID_ENTITY ? 0 : GET_COMPONENT(0, parentE, UI_Element);
    
    switch (current_pass)
    {
    case UI_PASS_SIZE_CORRECTION:
        ELEM_STATE(current)->sizeing = current->style.size;
        break;
    case UI_PASS_SIZES:
        ELEM_STATE(current)->remaining_children = ELEM_STATE(current)->children_count;
        ELEM_STATE(current)->final.width = current->style.padding.left + current->style.padding.right;
        ELEM_STATE(current)->final.height = current->style.padding.top + current->style.padding.bottom;
        calculateElementSizing_oppening(current, parent);
        break;
    case UI_PASS_FULL_SIZES:{

        calculateElementFullSizing(current, parent);
    }
        break;
    case UI_PASS_POSITIONS:{

        // initial advancement for the current element
        ELEM_STATE(current)->cursor = vec2_new(current->style.padding.left, current->style.padding.top);
        // advance the parent/canvas cursor
        if (parent)
        {
            ELEM_STATE(parent)->cursor = vec2_add(ELEM_STATE(parent)->cursor, vec2_new(current->style.margin.left, current->style.margin.top));
        }
        else
        {
            canvas.cursor = vec2_add(canvas.cursor, vec2_new(current->style.margin.left, current->style.margin.top));
        }
        Transform2D *t = GET_COMPONENT(0, current->id, Transform2D);
        calculateElementPosition(current, t, parent);
    }
        break;
    }
}

void closeElement()
{
    if (current_node_idx == 0)
    {
        // root / canvas
        // skip
        return;
    }

    ElementNode *currentNode = getNode(current_node_idx);
    EntityID current_entity = currentNode->this;

    EntityID parent_entity = current_entity != INVALID_ENTITY && currentNode->node_level > 0 ? getNode(currentNode->parent_idx)->this : INVALID_ENTITY;

    UI_Element *current = GET_COMPONENT(0, current_entity, UI_Element);
    UI_Element *parent = (parent_entity != INVALID_ENTITY) ? GET_COMPONENT(0, parent_entity, UI_Element) : 0;
    //
    switch (current_pass)
    {
    case UI_PASS_SIZE_CORRECTION:
        correctElementParentSizing(current, parent);
        break;
    case UI_PASS_SIZES:
        calculateElementSizing_closer(current, parent);
        break;
    case UI_PASS_FULL_SIZES:
        break;
    case UI_PASS_POSITIONS:
        break;
    }
}

void correctElementParentSizing(UI_Element *elem, UI_Element *parent)
{
    if (parent)
    {
        if (ELEM_STATE(elem)->sizeing == UI_SIZE_FULL && ELEM_STATE(parent)->sizeing == UI_SIZE_FIT)
        {
            ELEM_STATE(parent)->sizeing = UI_SIZE_FULL;
        }
    }
    else
    {
        // canvas
        if (ELEM_STATE(elem)->sizeing == UI_SIZE_FULL && canvas.sizeing == UI_SIZE_FIT)
        {
            canvas.sizeing = UI_SIZE_FULL;
            canvas.remainingSize = canvas.size;
        }
    }
}

void calculateElementSizing_oppening(UI_Element *elem, UI_Element *parent)
{
    switch (ELEM_STATE(elem)->sizeing)
    {
        case UI_SIZE_FIT:{
            //nothing
            if(parent)ELEM_STATE(parent)->remaining_children-=1;
            else canvas.remainingChildren-=1;
        }
        break;
        case UI_SIZE_WIDTH_HEIGHT:
        {
            ELEM_STATE(elem)->final.width = elem->style.width;
            ELEM_STATE(elem)->final.height = elem->style.height;
            ELEM_STATE(elem)->remaining_size.w = MAX(0, ELEM_STATE(elem)->final.width - elem->style.padding.left - elem->style.padding.right);
            ELEM_STATE(elem)->remaining_size.h = MAX(0, ELEM_STATE(elem)->final.height - elem->style.padding.top - elem->style.padding.bottom);
            if(parent)ELEM_STATE(parent)->remaining_children-=1;
            else canvas.remainingChildren-=1;
        }
        break;
        case UI_SIZE_FULL:
        {
            ELEM_STATE(elem)->remaining_size.w = 0;
            ELEM_STATE(elem)->remaining_size.h = 0;
        }
        break;
    }
}

void calculateElementSizing_closer(UI_Element *elem, UI_Element *parent){
    switch (ELEM_STATE(elem)->sizeing)
    {
        case UI_SIZE_FIT:
        {
            // TODO: add text size
            u32 gap_count = (ELEM_STATE(elem)->children_count > 0 ? ELEM_STATE(elem)->children_count - 1 : 0);
            switch (elem->style.layout)
            {
            case UI_LAYOUT_HORIZONTAL:
                ELEM_STATE(elem)->final.width += (gap_count * elem->style.gap.x);
                break;
            case UI_LAYOUT_VERTICAL:
                ELEM_STATE(elem)->final.height += (gap_count * elem->style.gap.y);
                break;
            }
        }
        break;
        case UI_SIZE_WIDTH_HEIGHT:
        break;
        case UI_SIZE_FULL:
        break;
    }

    //expand parent to fit child if fit
    if (parent)
    {
        switch(ELEM_STATE(parent)->sizeing)
        {
            case UI_SIZE_FIT :{
                switch (parent->style.layout)
                {
                case UI_LAYOUT_HORIZONTAL:
                {
                    ELEM_STATE(parent)->final.width += (ELEM_STATE(elem)->final.width + elem->style.margin.left + elem->style.margin.right);
                    ELEM_STATE(parent)->final.height =
                        MAX(
                            ELEM_STATE(parent)->final.height,
                            ELEM_STATE(elem)->final.height + parent->style.padding.top + parent->style.padding.bottom + elem->style.margin.top + elem->style.margin.bottom);
                }
                break;
                case UI_LAYOUT_VERTICAL:
                {
                    ELEM_STATE(parent)->final.height += (ELEM_STATE(elem)->final.height + elem->style.margin.top + elem->style.margin.bottom);
                    ELEM_STATE(parent)->final.width =
                        MAX(
                            ELEM_STATE(parent)->final.width,
                            ELEM_STATE(elem)->final.width + parent->style.padding.left + parent->style.padding.right + elem->style.margin.left + elem->style.margin.right);
                }
                break;
                }
            }
            break;
            case UI_SIZE_FULL:
            case UI_SIZE_WIDTH_HEIGHT:{
                //nothing
            }
            break;
        }
    }

    //subtract from remainingSize
    UISize parentSize = parent? ELEM_STATE(parent)->sizeing : canvas.sizeing;
    if((parentSize == UI_SIZE_FULL || parentSize == UI_SIZE_WIDTH_HEIGHT) && ELEM_STATE(elem)->sizeing == UI_SIZE_FIT){
        if(parent){
            switch(parent->style.layout){
                case UI_LAYOUT_HORIZONTAL :
                    ELEM_STATE(parent)->remaining_size.w -= (ELEM_STATE(elem)->final.width);
                break;
                case UI_LAYOUT_VERTICAL :
                    ELEM_STATE(parent)->remaining_size.h -= (ELEM_STATE(elem)->final.height);
                break;
            }
        }else{
            switch(canvas.layout){
                case UI_LAYOUT_HORIZONTAL :
                    canvas.remainingSize.w -= ELEM_STATE(elem)->final.width;
                break;
                case UI_LAYOUT_VERTICAL :
                    canvas.remainingSize.h -= ELEM_STATE(elem)->final.height;
                break;
            }
        }
    }
}

void calculateElementFullSizing(UI_Element* elem, UI_Element* parent){
    if(current_node_idx == 0){
        canvas.remainingSize = vec2_new(
            canvas.size.w - canvas.padding.left - canvas.padding.right,
            canvas.size.h - canvas.padding.top - canvas.padding.bottom
        );
        return;
    }

    if(ELEM_STATE(elem)->sizeing == UI_SIZE_FULL){
        if(parent){
            u32 gap_count = (ELEM_STATE(parent)->children_count > 0 ? ELEM_STATE(parent)->children_count - 1 : 0);
            u32 parentRemainingChildren = ELEM_STATE(parent)->remaining_children;
            switch(parent->style.layout){
                case UI_LAYOUT_HORIZONTAL :
                    ELEM_STATE(elem)->final.width = ((ELEM_STATE(parent)->remaining_size.w - (parent->style.gap.x * gap_count)) / parentRemainingChildren) - elem->style.margin.right - elem->style.margin.left;
                    ELEM_STATE(elem)->final.height = (ELEM_STATE(parent)->remaining_size.h) - elem->style.margin.top - elem->style.margin.bottom;
                    break;
                case UI_LAYOUT_VERTICAL :
                    ELEM_STATE(elem)->final.width = (ELEM_STATE(parent)->remaining_size.w) - elem->style.margin.right - elem->style.margin.left;
                    ELEM_STATE(elem)->final.height = ((ELEM_STATE(parent)->remaining_size.h - (parent->style.gap.y * gap_count)) / parentRemainingChildren) - elem->style.margin.top - elem->style.margin.bottom;
                break;
            }
        }else{
            u32 gap_count = (canvas.childrenCount > 0 ? canvas.childrenCount - 1 : 0);
            u32 parentRemainingChildren = canvas.remainingChildren;
            switch(canvas.layout){
                case UI_LAYOUT_HORIZONTAL :
                    ELEM_STATE(elem)->final.width = ((canvas.remainingSize.w - (canvas.gap.x * gap_count)) / parentRemainingChildren) - elem->style.margin.right - elem->style.margin.left;
                    ELEM_STATE(elem)->final.height = (canvas.remainingSize.h) - elem->style.margin.top - elem->style.margin.bottom;
                    break;
                case UI_LAYOUT_VERTICAL :
                    ELEM_STATE(elem)->final.width = (canvas.remainingSize.w) - elem->style.margin.right - elem->style.margin.left;
                    ELEM_STATE(elem)->final.height = ((canvas.remainingSize.h - (parent->style.gap.y * gap_count)) / parentRemainingChildren) - elem->style.margin.top - elem->style.margin.bottom;
                break;
            }
        }

        ELEM_STATE(elem)->remaining_size.w += (ELEM_STATE(elem)->final.width - elem->style.padding.right - elem->style.padding.left);
        ELEM_STATE(elem)->remaining_size.h += (ELEM_STATE(elem)->final.height - elem->style.padding.top - elem->style.padding.bottom);

    }
}

void calculateElementPosition(UI_Element *elem, Transform2D *t, UI_Element *parent)
{
    UILayout layout = parent ? parent->style.layout : canvas.layout;
    Vec2 *cursor = parent ? &ELEM_STATE(parent)->cursor : &canvas.cursor;
    Vec2 gap = parent ? parent->style.gap : canvas.gap;

    t->position.x = cursor->x;
    t->position.y = cursor->y;

    switch (layout)
    {
    case UI_LAYOUT_HORIZONTAL:
        cursor->x += ELEM_STATE(elem)->final.width + gap.x + elem->style.margin.right;
        cursor->y -= elem->style.margin.top;
        break;
    case UI_LAYOUT_VERTICAL:
        cursor->y += ELEM_STATE(elem)->final.height + gap.y + elem->style.margin.bottom;
        cursor->x -= elem->style.margin.left;
        break;
    }
}