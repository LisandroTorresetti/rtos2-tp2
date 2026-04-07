#include "ao_led.h"
#include "board.h"
#include "main.h"
#include "logger.h"

#define QUEUE_LENGTH            (10)
#define QUEUE_ITEM_SIZE         (sizeof(ao_led_message_t*))
#define HAS_PINS_CONNECTED 1
#define USE_SAME_QUEUE 0

#if USE_SAME_QUEUE
static QueueHandle_t queue = NULL;
#endif

#if HAS_PINS_CONNECTED
static GPIO_TypeDef* led_port[] = {LED_RED_PORT, LED_GREEN_PORT,  LED_BLUE_PORT};
static uint16_t led_pin[] = {LED_RED_PIN,  LED_GREEN_PIN, LED_BLUE_PIN };
#endif

static char colours[3][10] = { "red", "green", "blue"};
static char actions[2][10] = { "reset", "set" };

static void task(void *argument)
{
    ao_led_handler_t* hao = argument;
    while (true) {
    	taskENTER_CRITICAL();
        ao_led_message_t* msg;
        if (pdPASS == xQueueReceive(hao->hqueue, (void*) &msg, 0)) {
        	taskEXIT_CRITICAL();
        	GPIO_PinState realAction = !msg->action;
#if USE_SAME_QUEUE
        	ao_led_color_t color = msg->actualColour; // If we have the same queue, the msg carries the choice of what colour to use
#else
        	ao_led_color_t color = hao->color;
#endif
#if HAS_PINS_CONNECTED
            HAL_GPIO_WritePin(led_port[color], led_pin[color], realAction);
#endif
        	char log_message[256];        	snprintf(log_message, sizeof(log_message), "writing colour with action: %s, action: %s", colours[color], actions[realAction]);
        	LOGGER_INFO(log_message);
        	msg->callback(msg);
        } else {
        	hao->taskFunctioning = false;
			taskEXIT_CRITICAL();
        	char log_message[256];
			snprintf(log_message, sizeof(log_message), "I as led: %s, will be killed", colours[hao->color]);
			LOGGER_INFO(log_message);
			vTaskDelete(NULL);
        };
    }
}

/********************** external functions definition ************************/

bool ao_led_send(ao_led_handler_t* hao, ao_led_message_t* msg) {
    bool status = (pdPASS == xQueueSend(hao->hqueue, &msg, 0));
    taskENTER_CRITICAL();
    if (status && !hao->taskFunctioning) {
    	hao->taskFunctioning = pdPASS == xTaskCreate(task, "task_ao_led", 512, (void * const) hao, tskIDLE_PRIORITY, NULL);
    }
    taskEXIT_CRITICAL();
    return status;
}

void ao_led_init(ao_led_handler_t* hao, ao_led_color_t color) {
    hao->color = color;
#if USE_SAME_QUEUE // Done like this because some interpretations on the 2.f. were that we needed to use the same queue for the thread of execution
    if (queue == NULL) {
    	queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    }
    hao->hqueue = queue;
#else
    hao->hqueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
#endif
    while(NULL == hao->hqueue) {
        // error
    }
    hao->taskFunctioning = false;
}

/********************** end of file ******************************************/
