#ifndef EVENT_H
#define EVENT_H

#include "data_types.h"
#include "engine_defines.h"

#define CAST_EVENT_CONTEXT(context, event_type) *(event_type*)&context
#define EVENT_CALLBACK(name) void name(EventType eType, void* sender,  void* listener, EventContext eContext)


typedef enum EventType_e{
    //window events
    EVENT_TYPE_WINDOW_CREATED,
    EVENT_TYPE_WINDOW_MINIMIZED,
    EVENT_TYPE_WINDOW_MAXIMIZED,
    EVENT_TYPE_WINDOW_RESIZE_SET,
    EVENT_TYPE_WINDOW_RESIZING,
    EVENT_TYPE_WINDOW_RESIZED,
    EVENT_TYPE_WINDOW_FOCUSED,
    EVENT_TYPE_WINDOW_DEFOCUSED,
    EVENT_TYPE_WINDOW_CLOSED,

    //INPUTS
    EVENT_TYPE_KEY_UP,
    EVENT_TYPE_KEY_DOWN,
    EVENT_TYPE_MOUSE_MOVED,
    EVENT_TYPE_MOUSE_BUTTON_UP,
    EVENT_TYPE_MOUSE_BUTTON_DOWN,
    EVENT_TYPE_MOUSE_SCROLL,


    MAX_EVENT_TYPE
} EventType;

typedef struct EventContext_t{
    union {
        f64 f64[4];
        u64 u64[4];
        i64 i64[4];

        f32 f32[8];
        u32 u32[8];
        i32 i32[4];

        u64 u16[16];
        i64 i16[16];

        u8 u8[32];
        i8 i8[32];
        char c[32];
    };
} EventContext;


typedef void (*EventCallback)(EventType eType, void* sender,  void* listener, EventContext eContext);

typedef struct EventListener_t{
    u16 _id;
    EventCallback callback;
    void* listener;
} EventListener;



//functions
void init_event_sys();
API void subscribe_to_event(EventType t, EventListener* listener);
void emit_event(EventType e, EventContext context, void* sender);
API void unsubsribe_from_event(EventType t, EventListener* listener);
void shutdown_event_sys();


#endif //EVENT_H