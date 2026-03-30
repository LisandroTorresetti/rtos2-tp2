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
#define BLINK_PERIOD_MS         1000

/********************** internal data declaration ****************************/

typedef struct {
  QueueHandle_t hqueue;
  ao_led_handler_t colours[3];
} ao_ui_handle_t;

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

static ao_ui_handle_t hao;

/********************** internal functions definition ************************/

static void task(void *argument) {
  while (true) {
    ao_led_message_t msg;
    msg.blink_time = BLINK_PERIOD_MS;
    msg_event_t event_msg;

    if (pdPASS == xQueueReceive(hao.hqueue, &event_msg, portMAX_DELAY)) {
      ao_led_handler_t haoLed = hao.colours[event_msg];
      ao_led_send(&haoLed, &msg);
    }
  }
}

/********************** external functions PULSE definition ************************/

bool ao_ui_send_event(msg_event_t msg) {
  return (pdPASS == xQueueSend(hao.hqueue, &msg, 0));
}

void ao_ui_init(ao_led_handler_t colours[3]) {
  hao.hqueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
  while(NULL == hao.hqueue) {
    // error
  }

  BaseType_t status = xTaskCreate(task, "task_ao_ui", 128, NULL, tskIDLE_PRIORITY, NULL);
  while (pdPASS != status) {
    // error
  }

  for (uint8_t i = 0; i < 3; i++) {
    hao.colours[i] = colours[i];
  }
}

/********************** end of file ******************************************/
