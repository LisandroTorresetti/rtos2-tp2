#ifndef INC_AO_LED_H_
#define INC_AO_LED_H_

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "main.h"
#include "error.h"

typedef struct {
    bool turn_on;
} ao_led_message_t;

typedef struct {
	QueueHandle_t hqueue;
	GPIO_TypeDef* led_port;
	uint16_t led_pin;
} ao_led_handler_t;

app_err_t ao_led_init(ao_led_handler_t* handler_ao, GPIO_TypeDef* led_port, uint16_t led_pin);

bool ao_led_send(ao_led_handler_t* handler_ao, ao_led_message_t* msg);

#endif /* INC_AO_LED_H_ */
