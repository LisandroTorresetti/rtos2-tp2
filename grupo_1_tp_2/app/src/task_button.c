/*
 * Copyright (c) 2023 Sebastian Bedin <sebabedin@gmail.com>.
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
 * @author : Sebastian Bedin <sebabedin@gmail.com>
 */

/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "ao_ui.h"
#include "main.h"
#include "cmsis_os.h"
#include "board.h"

/********************** macros and definitions *******************************/

#define BUTTON_PERIOD_MS         (50)
#define BUTTON_PULSE_TIMEOUT     (200)
#define BUTTON_SHORT_TIMEOUT     (1000)
#define BUTTON_LONG_TIMEOUT      (2000)

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

typedef enum {
  BUTTON_TYPE_NONE,
  BUTTON_TYPE_PULSE,
  BUTTON_TYPE_SHORT,
  BUTTON_TYPE_LONG,
} button_type_t;

/**
 * @brief Categorizes a button press duration into a button_type_t.
 *
 */
static button_type_t categorize(uint32_t value) {
  return (value > BUTTON_PULSE_TIMEOUT) + (value >= BUTTON_SHORT_TIMEOUT) + (value >= BUTTON_LONG_TIMEOUT);
}

static struct {
    uint32_t counter;
} button;

static void button_init_(void) {
  button.counter = 0;
}

static button_type_t button_process_state(bool value) {
  button_type_t ret = BUTTON_TYPE_NONE;
  if(!value) {
    button.counter += BUTTON_PERIOD_MS;
  }

  else {
    ret = categorize(button.counter);
    button.counter = 0;
  }

  return ret;
}

/********************** external functions definition ************************/

void task_button(void* argument) {
  button_init_();

  while(true) {
    GPIO_PinState button_state = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);

    button_type_t button_type = button_process_state(button_state);

    if (button_type != BUTTON_TYPE_NONE) {
      ao_ui_send_event(button_type - 1);
    }

    vTaskDelay((TickType_t)(BUTTON_PERIOD_MS / portTICK_PERIOD_MS));
  }
}

/********************** end of file ******************************************/
