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
    UI_PASS_POSITIONS,
    UI_MAX_PASS
} UIPass;

typedef struct UI_Element_State_t
{
    struct
    {
        f32 width, height;
        UISize sizeing;
    } final;
    Vec2 remaining_size;

    u32 children_count;
    u32 remaining_children;
    Vec2 cursor;
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

typedef struct ElementNode
{
    u32 parent_idx;
    u32 this_idx;
    u32 node_level;
    EntityID this;
    u32 *children;
} ElementNode;

static UIPass current_pass = UI_PASS_SIZE_CORRECTION;
static UI_Canvas canvas = {0};
static ElementNode *elements_tree = 0;
static u32 current_node_idx = 0;

#define ELEM_STATE(elem) ((UI_Element_State *)((elem)->internal))

/////////////////////////////////////
/////   Forward Declarations    /////
/////////////////////////////////////

void correctElementParentSizing(UI_Element *elem, UI_Element *parent);
void calculateElementSizing_oppening(UI_Element *elem, UI_Element *parent);
void calculateElementSizing_closer(UI_Element *elem, UI_Element *parent);
void calculateElementFullSizing(UI_Element *elem, UI_Element *parent);
void calculateElementPosition(UI_Element *elem, Transform2D *t, UI_Element *parent);
void open_element(EntityID elem);
void close_element();
void ui_run_pass(UIPass pass);

///////////////////////////////////////////
////////    CANVAS getter/setter    ///////
///////////////////////////////////////////

Vec4 ui_builder_canvas_padding_get(void) { return canvas.padding; }
void ui_builder_canvas_padding_set(Vec4 pad)
{
    canvas.padding = pad;
}
Vec2 ui_builder_canvas_positon_get(void) { return canvas.position; }
void ui_builder_canvas_positon_set(Vec2 pos)
{
    canvas.position = pos;
}
Vec2 ui_builder_canvas_size_get(void) { return canvas.size; }
void ui_builder_canvas_size_set(Vec2 size)
{
    canvas.size = size;
}
u32 ui_builder_canvas_resolution_get(void) { return canvas.pixelsPerPoint; }
void ui_builder_canvas_resolution_set(u32 pixels_per_point)
{
    canvas.pixelsPerPoint = pixels_per_point;
}
void ui_builder_canvas_flush(void)
{
    canvas.childrenCount = 0;
    canvas.cursor = vec2_add(canvas.position, vec2_new(canvas.padding.left, canvas.padding.top));
}

Vec2 ui_element_get_final_size(UI_Element *elem)
{
    return vec2_new(ELEM_STATE(elem)->final.width, ELEM_STATE(elem)->final.height);
}

////////////////////////////////////////
///////     TREE MANAGEMENT     ////////
////////////////////////////////////////

ElementNode *get_node(u32 idx)
{
    return &elements_tree[idx];
}

void ui_builder_reset_tree(void)
{
    // If the tree already exists, free its memory.
    if (elements_tree)
    {
        // Free children arrays of each node.
        u32 count = DynamicArray_Length(elements_tree);
        for (u32 i = 0; i < count; i++)
        {
            ElementNode *node = get_node(i);
            if (node->children)
            {
                DynamicArray_Destroy(node->children);
                node->children = 0;
            }
        }
        // Clear the tree dynamic array.
        DynamicArray_Clear(elements_tree);
    }
    else
    {
        // Create a new tree if it doesn't exist.
        elements_tree = DynamicArray_Create(ElementNode);
    }

    // Create the root node (canvas).
    ElementNode canvas = {
        .node_level = 0,
        .parent_idx = 0, // For the root, you might set this to 0 or an invalid value.
        .this_idx = 0,
        .this = INVALID_ENTITY,
        .children = 0};
    DynamicArray_Push(elements_tree, canvas);
    current_node_idx = 0;
}

void ui_builder_open_element(EntityID elem)
{
    ElementNode *current_node = get_node(current_node_idx);

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

void ui_builder_close_element(void)
{
    // move to current ptr to parent node
    if (current_node_idx)
    {
        current_node_idx = get_node(current_node_idx)->parent_idx;
    }
}

///////////////////////////////////////
//////      Builder Logic       ///////
///////////////////////////////////////

void ui_builder_build()
{
    for (UIPass pass = UI_PASS_SIZE_CORRECTION; pass < UI_MAX_PASS; pass++)
    {
        ui_run_pass(pass);
    }
}

static void traverse_tree_branch(ElementNode *node)
{
    open_element(node->this);
    if (node->children)
    {
        const u32 child_count = DynamicArray_Length(node->children);
        for (u8 i = 0; i < child_count; i++)
        {
            const u32 child_idx = node->children[i];
            if (DynamicArray_Length(elements_tree) > child_idx)
            {
                current_node_idx = child_idx;
                traverse_tree_branch(get_node(child_idx));
            }
        }
    }
    close_element();
    current_node_idx = node->parent_idx;
}

static void ui_run_pass(UIPass pass)
{
    current_pass = CLAMP(pass, UI_PASS_SIZE_CORRECTION, UI_PASS_POSITIONS);
    current_node_idx = 0;
    traverse_tree_branch(&elements_tree[current_node_idx]);
}

//////////////////////////////////////////
//////      Element Operations      //////
//////////////////////////////////////////

static void open_element(EntityID elem)
{
    if (current_node_idx == 0)
    {
        // root / canvas
        canvas.childrenCount = DynamicArray_Length(get_node(0)->children);
        canvas.remainingChildren = canvas.childrenCount;
        // temp
        canvas.sizeing = UI_SIZE_FULL;
        canvas.remainingSize = vec2_sub(canvas.size, vec2_new(canvas.padding.right + canvas.padding.left, canvas.padding.top + canvas.padding.bottom));
        return;
    }

    UI_Element *current = GET_COMPONENT(0, elem, UI_Element);
    if (!current->internal)
    {
        current->internal = malloc(sizeof(UI_Element_State));
        if (!current->internal)
        {
            LOG_ERROR("UI Builder -> open element : Failed to initialize element internal data");
        }
    }

    ElementNode *currentN = get_node(current_node_idx);
    EntityID parentE = get_node(currentN->parent_idx)->this;
    UI_Element *parent = (parentE != INVALID_ENTITY) ? GET_COMPONENT(0, parentE, UI_Element) : 0;

    ELEM_STATE(current)->children_count = currentN->children ? DynamicArray_Length(currentN->children) : 0;

    switch (current_pass)
    {
        case UI_PASS_SIZE_CORRECTION:{
            ELEM_STATE(current)->final.sizeing = current->style.size;
            break;
        }
        case UI_PASS_SIZES:{
            ELEM_STATE(current)->remaining_children = ELEM_STATE(current)->children_count;
            ELEM_STATE(current)->final.width = current->style.padding.left + current->style.padding.right;
            ELEM_STATE(current)->final.height = current->style.padding.top + current->style.padding.bottom;
            calculateElementSizing_oppening(current, parent);
            break;
        }
        case UI_PASS_FULL_SIZES:
        {
            calculateElementFullSizing(current, parent);
            break;
        }
        case UI_PASS_POSITIONS:
        {
            // initial advancement for the current element
            ELEM_STATE(current)->cursor = vec2_new(current->style.padding.left, current->style.padding.top);
            // advance the parent/canvas cursor
            Vec2* cursor = parent ? &ELEM_STATE(parent)->cursor : &canvas.cursor; 
            *cursor = vec2_add(*cursor, vec2_new(current->style.margin.left, current->style.margin.top));
            
            Transform2D *transform = GET_COMPONENT(0, current->id, Transform2D);
            calculateElementPosition(current, transform, parent);
            break;
        }
    }
}

static void close_element()
{
    if (current_node_idx == 0)
    {
        // root / canvas
        // skip
        return;
    }

    ElementNode *currentNode = get_node(current_node_idx);
    const EntityID parent_id = currentNode->this != INVALID_ENTITY && currentNode->node_level > 0 ? get_node(currentNode->parent_idx)->this : INVALID_ENTITY;

    UI_Element *current = GET_COMPONENT(0, currentNode->this, UI_Element);
    UI_Element *parent = (parent_id != INVALID_ENTITY) ? GET_COMPONENT(0, parent_id, UI_Element) : 0;
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
        case UI_PASS_POSITIONS:
            //No actions needed
            break;
    }
}

void correctElementParentSizing(UI_Element *elem, UI_Element *parent)
{
    UISize* parent_sizing = parent ? &ELEM_STATE(parent)->final.sizeing : &canvas.sizeing;

        if (ELEM_STATE(elem)->final.sizeing == UI_SIZE_FULL && *parent_sizing == UI_SIZE_FIT)
        {
            *parent_sizing = UI_SIZE_FULL;
            if(!parent) canvas.remainingSize = canvas.size;
        }
}

void calculateElementSizing_oppening(UI_Element *elem, UI_Element *parent)
{
    switch (ELEM_STATE(elem)->final.sizeing)
    {
    case UI_SIZE_FIT:
    {
        // nothing
        if (parent)
            ELEM_STATE(parent)->remaining_children -= 1;
        else
            canvas.remainingChildren -= 1;
    }
    break;
    case UI_SIZE_WIDTH_HEIGHT:
    {
        ELEM_STATE(elem)->final.width = elem->style.width;
        ELEM_STATE(elem)->final.height = elem->style.height;
        ELEM_STATE(elem)->remaining_size.w = MAX(0, ELEM_STATE(elem)->final.width - elem->style.padding.left - elem->style.padding.right);
        ELEM_STATE(elem)->remaining_size.h = MAX(0, ELEM_STATE(elem)->final.height - elem->style.padding.top - elem->style.padding.bottom);
        if (parent)
            ELEM_STATE(parent)->remaining_children -= 1;
        else
            canvas.remainingChildren -= 1;
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

void calculateElementSizing_closer(UI_Element *elem, UI_Element *parent)
{
    switch (ELEM_STATE(elem)->final.sizeing)
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

    // expand parent to fit child if fit
    if (parent)
    {
        switch (ELEM_STATE(parent)->final.sizeing)
        {
        case UI_SIZE_FIT:
        {
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
        case UI_SIZE_WIDTH_HEIGHT:
        {
            // nothing
        }
        break;
        }
    }

    // subtract from remainingSize
    UISize parentSize = parent ? ELEM_STATE(parent)->final.sizeing : canvas.sizeing;
    if ((parentSize == UI_SIZE_FULL || parentSize == UI_SIZE_WIDTH_HEIGHT) && ELEM_STATE(elem)->final.sizeing == UI_SIZE_FIT)
    {
        if (parent)
        {
            switch (parent->style.layout)
            {
            case UI_LAYOUT_HORIZONTAL:
                ELEM_STATE(parent)->remaining_size.w -= (ELEM_STATE(elem)->final.width);
                break;
            case UI_LAYOUT_VERTICAL:
                ELEM_STATE(parent)->remaining_size.h -= (ELEM_STATE(elem)->final.height);
                break;
            }
        }
        else
        {
            switch (canvas.layout)
            {
            case UI_LAYOUT_HORIZONTAL:
                canvas.remainingSize.w -= ELEM_STATE(elem)->final.width;
                break;
            case UI_LAYOUT_VERTICAL:
                canvas.remainingSize.h -= ELEM_STATE(elem)->final.height;
                break;
            }
        }
    }
}

void calculateElementFullSizing(UI_Element *elem, UI_Element *parent)
{
    if (current_node_idx == 0)
    {
        canvas.remainingSize = vec2_new(
            canvas.size.w - canvas.padding.left - canvas.padding.right,
            canvas.size.h - canvas.padding.top - canvas.padding.bottom);
        return;
    }

    if (ELEM_STATE(elem)->final.sizeing == UI_SIZE_FULL)
    {
        if (parent)
        {
            u32 gap_count = (ELEM_STATE(parent)->children_count > 0 ? ELEM_STATE(parent)->children_count - 1 : 0);
            u32 parentRemainingChildren = ELEM_STATE(parent)->remaining_children;
            switch (parent->style.layout)
            {
            case UI_LAYOUT_HORIZONTAL:
                ELEM_STATE(elem)->final.width = ((ELEM_STATE(parent)->remaining_size.w - (parent->style.gap.x * gap_count)) / parentRemainingChildren) - elem->style.margin.right - elem->style.margin.left;
                ELEM_STATE(elem)->final.height = (ELEM_STATE(parent)->remaining_size.h) - elem->style.margin.top - elem->style.margin.bottom;
                break;
            case UI_LAYOUT_VERTICAL:
                ELEM_STATE(elem)->final.width = (ELEM_STATE(parent)->remaining_size.w) - elem->style.margin.right - elem->style.margin.left;
                ELEM_STATE(elem)->final.height = ((ELEM_STATE(parent)->remaining_size.h - (parent->style.gap.y * gap_count)) / parentRemainingChildren) - elem->style.margin.top - elem->style.margin.bottom;
                break;
            }
        }
        else
        {
            u32 gap_count = (canvas.childrenCount > 0 ? canvas.childrenCount - 1 : 0);
            u32 parentRemainingChildren = canvas.remainingChildren;
            switch (canvas.layout)
            {
            case UI_LAYOUT_HORIZONTAL:
                ELEM_STATE(elem)->final.width = ((canvas.remainingSize.w - (canvas.gap.x * gap_count)) / parentRemainingChildren) - elem->style.margin.right - elem->style.margin.left;
                ELEM_STATE(elem)->final.height = (canvas.remainingSize.h) - elem->style.margin.top - elem->style.margin.bottom;
                break;
            case UI_LAYOUT_VERTICAL:
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