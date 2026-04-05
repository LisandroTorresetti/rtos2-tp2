/*
 * Copyright (c) 2024 Sebastian Bedin <sebabedin@gmail.com>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @file   : ao_led.c
 * @date   : Feb 17, 2023
 * @author : Sebastian Bedin <sebabedin@gmail.com>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "cmsis_os.h"

#include "ao_ui.h"
#include "ao_led.h"
#include "messages.h"

/********************** macros and definitions *******************************/

#define QUEUE_LENGTH            (1)
#define QUEUE_ITEM_SIZE         (sizeof(event_message_t*))

static const char* TASK_NAME = "task_ao_ui";

/********************** internal data declaration ****************************/

// States of the Finite State Machine
typedef enum {
    STANDBY_STATE,
    RED_STATE,
	GREEN_STATE,
	BLUE_STATE
} state_type_t;

// state_t each state has its type and a handler of the corresponding LED
typedef struct {
	state_type_t state_type;
	ao_led_handler_t state_handler;
} state_t;


typedef struct {
  QueueHandle_t hqueue;
  state_t states[4];
} ao_ui_handle_t;


static state_type_t current_state;


/********************** internal functions declaration ***********************/

static void entry_state(ao_led_handler_t* ao_led_handler);
static void exit_state(ao_led_handler_t* ao_led_handler);

/********************** internal data definition *****************************/

static ao_ui_handle_t ao_ui_handler;

/********************** internal functions definition ************************/

static void task(void *argument) {
  while (true) {
	  event_message_t* event_msg;

    if (pdPASS == xQueueReceive(ao_ui_handler.hqueue, &event_msg, portMAX_DELAY)) {
    	state_type_t new_state = event_msg->button_event + 1;

    	if (new_state == current_state) {
    		// NoOp
    		event_msg->callback(event_msg);
    		continue;
    	}

    	if (current_state != STANDBY_STATE) {
        	exit_state(&ao_ui_handler.states[current_state].state_handler);
    	}

    	entry_state(&ao_ui_handler.states[new_state].state_handler);
    	current_state = new_state;

    	event_msg->callback(event_msg);
    }
  }
}

void entry_state(ao_led_handler_t* ao_led_handler) {
	ao_led_send(ao_led_handler, TURN_ON);
}

void exit_state(ao_led_handler_t* ao_led_handler) {
	ao_led_send(ao_led_handler, TURN_OFF);
}

static void message_processed_callback(void* msg) {
	vPortFree(msg);
}

/********************** external functions PULSE definition ************************/

app_err_t ao_ui_init(ao_led_handler_t colours[3]) {
  ao_ui_handler.hqueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
  if (NULL == ao_ui_handler.hqueue) {
    return ERR_NO_MEMORY;
  }

  BaseType_t status = xTaskCreate(task, TASK_NAME, 512, NULL, tskIDLE_PRIORITY, NULL);
  if (pdPASS != status) {
	  return ERR_NO_MEMORY;
  }

  current_state = STANDBY_STATE;
  ao_ui_handler.states[0].state_type = STANDBY_STATE;

  for (uint8_t i = 0; i < 3; i++) {
	  ao_ui_handler.states[i + 1].state_type = i + 1;
	  ao_ui_handler.states[i + 1].state_handler = colours[i];
  }

  return APP_OK;
}

void ao_ui_send_event(event_t button_event) {
	event_message_t* msg = (event_message_t*)pvPortMalloc(sizeof(event_message_t));
	if (msg == NULL) {
		mayday_signal();
		return;
	}

	msg->button_event = button_event;
	msg->callback = message_processed_callback;

	if (pdPASS != xQueueSend(ao_ui_handler.hqueue, (void*)&msg, 0)) {
		mayday_signal();
		vPortFree(msg);
	}
}

/********************** end of file ******************************************/
