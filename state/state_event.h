/** @copydoc */
#ifndef STATE_STATE_EVENT_H
#define STATE_STATE_EVENT_H
/**  @abstract */
/**  @version    @author    @date */

enum lm_event_type {
    LM_SYSTEM_MANAGE_EVENT,
    LM_IN_MESSAGE_EVENT,
    LM_OUT_MESSAGE_EVENT,
    LM_WALL_TIMER_EVENT,
    LM_LOGIC_TIMER_EVENT,

};

/**
1. Event's json structure sample
{
"type":"system manage"
"name":"system start event",
"id": 12
"properties":{
    "info_name":"system start",
    "instance_name": "current scenario"
}

}

2. Event's runtime structure sample
struct event_state {
intptr_t event_struct;
intptr_t event_value;
};

array_event_value {
int32_t capacity;
int32_t size;
int32_t value[size];
}

array_event_struct {
int32_t capacity;
int32_t size;
struct ev_struct {
int32_t id;

}[size];
}

*/

#endif /** STATE_STATE_EVENT_H */
