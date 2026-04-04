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

/********************** macros and definitions *******************************/

#define QUEUE_LENGTH            (1)
#define QUEUE_ITEM_SIZE         (sizeof(msg_event_t))

static const char* TASK_NAME = "task_ao_ui";

/********************** internal data declaration ****************************/

typedef struct {
  QueueHandle_t hqueue;
  ao_led_handler_t colours[3];
} ao_ui_handle_t;

typedef enum {
    STANDBY_STATE,
    RED_STATE,
	GREEN_STATE,
	BLUE_STATE
} state_type_t;

typedef struct {
	state_type_t state;
	ao_led_handler_t* state_handler;
} state_t;

static state_t current_state;

// cada estado puede saber como transicionar a otro o que hacer dependiendo del pulso
// si soy rojo y me llega algo rojo me quedo en el lugar, mientras que si llega uno de los otros casos voy para alla



/********************** internal functions declaration ***********************/

static void init_state(ao_ui_handle_t* ui_handler, state_t* state, msg_event_t event_msg);
static void entry_state(ao_led_handler_t* ao_led_handler); // tiene que pasarle el AO del led handler y sabe que mandar
static void exit_state(ao_led_handler_t* ao_led_handler);

/********************** internal data definition *****************************/

static ao_ui_handle_t hao;

/********************** internal functions definition ************************/

static void task(void *argument) {
  while (true) {
    msg_event_t event_msg;

    if (pdPASS == xQueueReceive(hao.hqueue, &event_msg, portMAX_DELAY)) {
    	state_t new_state;
    	init_state(&hao, &new_state, event_msg);

    	if (new_state.state == current_state.state) {
    		continue;
    	}

    	if (current_state.state_handler != NULL) {
        	exit_state(current_state.state_handler);
    	}

    	entry_state(new_state.state_handler);

    	current_state.state = new_state.state;
    	current_state.state_handler = new_state.state_handler;
    }
  }
}

void entry_state(ao_led_handler_t* ao_led_handler) {
	ao_led_message_t msg;
	msg.turn_on = true;
	ao_led_send(ao_led_handler, &msg);
}

void exit_state(ao_led_handler_t* ao_led_handler) {
	ao_led_message_t msg;
	msg.turn_on = false;
	ao_led_send(ao_led_handler, &msg);
}

void init_state(ao_ui_handle_t* ui_handler, state_t* state, msg_event_t event_msg) {
	state->state_handler = &ui_handler->colours[event_msg];
	if (event_msg == MSG_EVENT_BUTTON_PULSE) {
		state->state = RED_STATE;
		return;

	}

	if (event_msg == MSG_EVENT_BUTTON_SHORT) {
		state->state = GREEN_STATE;
		return;
	}

	state->state = BLUE_STATE;
}

/********************** external functions PULSE definition ************************/

app_err_t ao_ui_init(ao_led_handler_t colours[3]) {
  current_state.state = STANDBY_STATE;
  current_state.state_handler = NULL;

  hao.hqueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
  if (NULL == hao.hqueue) {
    return ERR_NO_MEMORY;
  }

  BaseType_t status = xTaskCreate(task, TASK_NAME, 512, NULL, tskIDLE_PRIORITY, NULL);
  if (pdPASS != status) {
	  return ERR_NO_MEMORY;
  }

  for (uint8_t i = 0; i < 3; i++) {
    hao.colours[i] = colours[i];
  }

  return APP_OK;
}

bool ao_ui_send_event(msg_event_t msg) {
	// misma logica para ver si creas la task o no

  return (pdPASS == xQueueSend(hao.hqueue, &msg, 0));
}

/********************** end of file ******************************************/
