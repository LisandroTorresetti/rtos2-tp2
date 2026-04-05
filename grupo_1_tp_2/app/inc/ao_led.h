#ifndef INC_AO_LED_H_
#define INC_AO_LED_H_

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "main.h"
#include "error.h"
#include "messages.h"

typedef struct {
	TaskHandle_t task_handler;
	QueueHandle_t hqueue;
	GPIO_TypeDef* led_port;
	uint16_t led_pin;
} ao_led_handler_t;

app_err_t ao_led_init(ao_led_handler_t* handler_ao, GPIO_TypeDef* led_port, uint16_t led_pin);

void ao_led_send(ao_led_handler_t* handler_ao, led_action_t action);

#endif /* INC_AO_LED_H_ */
