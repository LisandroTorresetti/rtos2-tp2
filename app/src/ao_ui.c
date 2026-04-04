/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "cmsis_os.h"

#include "ao_ui.h"
#include "ao_led.h"
#include "logger.h"

/********************** macros and definitions *******************************/

#define QUEUE_LENGTH            (10)
#define QUEUE_ITEM_SIZE         (sizeof(msg_event_t))

/********************** internal data declaration ****************************/

typedef struct {
  QueueHandle_t hqueue;
  ao_led_handler_t colours[3];
} ao_ui_handle_t;

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

static ao_ui_handle_t hao;
static callback_t nextCallback;
static void freeCallback(ao_led_message_t* msg);
static void resetCallback(ao_led_message_t* msg);
static ao_led_color_t lastColour;
static bool wasSet;
static bool taskCreated = false;
/********************** internal functions definition ************************/

static void freeCallback(ao_led_message_t* msg) {
  nextCallback = resetCallback;
  lastColour = msg->actualColour;
  vPortFree((void*) msg);
}

static void resetCallback(ao_led_message_t* msg) {
  if (msg->action == MSG_AO_LED_EVENT_RESET_COLOR) {
    ao_led_message_t* newMsg = (ao_led_message_t*) pvPortMalloc(sizeof (ao_led_message_t));
    newMsg->callback = freeCallback;
    newMsg->action = MSG_AO_LED_EVENT_SET_COLOR;
    newMsg->actualColour = msg->nextColour;
    ao_led_send(&hao.colours[msg->nextColour], newMsg);
  }
  vPortFree((void*) msg);
}

static void task(void *argument) {
  while (true) {
    msg_event_t event_msg;

    taskENTER_CRITICAL();
    if (pdPASS == xQueueReceive(hao.hqueue, &event_msg, 0)) {
      ao_led_message_t* msg = (ao_led_message_t*) pvPortMalloc(sizeof (ao_led_message_t));
      ao_led_color_t thisColour = (ao_led_color_t) event_msg;
      msg->callback = nextCallback;
      ao_led_handler_t haoLed = hao.colours[wasSet ? lastColour : event_msg];
      msg->action = thisColour != lastColour && wasSet;
      msg->actualColour = wasSet ? lastColour : haoLed.color;
      msg->nextColour = thisColour;
      wasSet = true;
      ao_led_send(&haoLed, msg);
      lastColour = haoLed.color;
      taskEXIT_CRITICAL();
    } else {
    	LOGGER_INFO("AO UI being closed as no msg was received");
    	taskCreated = false;
    	taskEXIT_CRITICAL();
    	vTaskDelete(NULL);
    }
  }
}

/********************** external functions PULSE definition ************************/

bool ao_ui_send_event(msg_event_t msg) {
  bool status = (pdPASS == xQueueSend(hao.hqueue, &msg, 0));
  taskENTER_CRITICAL();
  if (status && !taskCreated) {
	  xTaskCreate(task, "task_ao_ui", 128, NULL, tskIDLE_PRIORITY, NULL);
	  taskCreated = true;
  }
  taskEXIT_CRITICAL();
  return status;
}

void ao_ui_init(ao_led_handler_t colours[3]) {
  hao.hqueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
  while(NULL == hao.hqueue) {
    // error
  }

  nextCallback = freeCallback;

  for (uint8_t i = 0; i < 3; i++) {
    hao.colours[i] = colours[i];
  }
}

/********************** end of file ******************************************/
