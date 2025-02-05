#include "core/events.h"

#include <collections/DynamicArray.h>
#include "core/debugger.h"

EventListener* event_listeners[MAX_EVENT_TYPE];

void init_event_sys(){
    for(u32 i=0; i<MAX_EVENT_TYPE; i++){
        event_listeners[i] = 0;
    }
    // LOG_TRACE("Event system initialized");
}
void subscribe_to_event(EventType t, EventListener* listener){
    if(event_listeners[t] == 0){
        event_listeners[t] = DynamicArray_Create(EventListener);
    }
    
    listener->_id = DynamicArray_Length(event_listeners[t]);
    DynamicArray_Push(event_listeners[t], *listener);
    // LOG_TRACE("Subscribed to event %d", t);
}
void emit_event(EventType type, EventContext context, void* sender){
    if(event_listeners[type] == 0){
        return;
    }
    u16 listnersCount = DynamicArray_Length(event_listeners[type]);
    for(u32 i=0; i<listnersCount; i++){
        EventListener listener = event_listeners[type][i];
        listener.callback(type, sender, listener.listener, context);
    }
    // LOG_TRACE("Emitted event %d [listners : %d]", type, listnersCount);
}

void unsubsribe_from_event(EventType t, EventListener* listener){
    if(event_listeners[t] == 0){
        return;
    }
    DynamicArray_PopAt(event_listeners[t], listener->_id, 0);
    // LOG_TRACE("Unsubscribed from event %d", t);
}
void shutdown_event_sys(){
    for(u32 i=0; i<MAX_EVENT_TYPE; i++){
        if(event_listeners[i] != 0){
            DynamicArray_Destroy(event_listeners[i]);
        }
    }
    // LOG_TRACE("Event system shutdown");
}