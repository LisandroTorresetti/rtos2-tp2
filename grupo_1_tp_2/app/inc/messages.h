#ifndef INC_MESSAGES_H_
#define INC_MESSAGES_H_

#include <stdbool.h>


typedef void (*callback_t)(void* msg);


typedef enum {
    EVENT_BUTTON_PULSE,
    EVENT_BUTTON_SHORT,
    EVENT_BUTTON_LONG,
} event_t;


typedef struct {
	event_t button_event;
	callback_t callback;
} event_message_t;


typedef enum {
    TURN_ON,
    TURN_OFF,
} led_action_t;

typedef struct {
	led_action_t led_action;
	callback_t callback;
} action_message_t;

#endif /* INC_MESSAGES_H_ */
