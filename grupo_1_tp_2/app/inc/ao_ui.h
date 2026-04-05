#ifndef INC_AO_UI_H_
#define INC_AO_UI_H_

#include <stdbool.h>

#include "ao_led.h"
#include "messages.h"

typedef enum {
    MSG_EVENT_BUTTON_PULSE,
    MSG_EVENT_BUTTON_SHORT,
    MSG_EVENT_BUTTON_LONG,
} msg_event_t;

app_err_t ao_ui_init(ao_led_handler_t colours[3]);

void ao_ui_send_event(event_t button_event);

#endif /* INC_AO_UI_H_ */
