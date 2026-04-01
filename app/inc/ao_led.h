#ifndef TP1_AO_LED_H
#define TP1_AO_LED_H

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os.h"

typedef struct msg_t ao_led_message_t;

typedef void (*callback_t)(ao_led_message_t* pmsg);

typedef enum {
    MSG_AO_LED_EVENT_SET_COLOR,
    MSG_AO_LED_EVENT_RESET_COLOR,
} actionColour;

typedef enum {
    AO_LED_COLOR_RED,
    AO_LED_COLOR_GREEN,
    AO_LED_COLOR_BLUE,
} ao_led_color_t;

struct msg_t {
    callback_t callback;
    actionColour action;
    ao_led_color_t actualColour;
    ao_led_color_t nextColour;
};

typedef struct {
    ao_led_color_t color;
    QueueHandle_t hqueue;
} ao_led_handler_t;

bool ao_led_send(ao_led_handler_t* hao, void* msg);

void ao_led_init(ao_led_handler_t* hao, ao_led_color_t color);

#endif //TP1_AO_LED_H
