#include "ao_led.h"
#include "board.h"
#include "main.h"
#include "logger.h"

#define QUEUE_LENGTH            (10)
#define QUEUE_ITEM_SIZE         (sizeof(ao_led_message_t*))

static GPIO_TypeDef* led_port[] = {LED_RED_PORT, LED_GREEN_PORT,  LED_BLUE_PORT};
static uint16_t led_pin[] = {LED_RED_PIN,  LED_GREEN_PIN, LED_BLUE_PIN };

static char colours[3][10] = { "red", "green", "blue"};
static char actions[2][10] = { "set", "reset" };

static void task(void *argument)
{
    ao_led_handler_t* hao = argument;
    while (true) {
        ao_led_message_t* msg;
        if (pdPASS == xQueueReceive(hao->hqueue, (void*) &msg, 0)) {
            HAL_GPIO_WritePin(led_port[hao->color], led_pin[hao->color], (GPIO_PinState) msg->action);
        	char log_message[256];        	snprintf(log_message, sizeof(log_message), "writing colour with action: %s, action: %s", colours[hao->color], actions[msg->action]);
        	LOGGER_INFO(log_message);
        	msg->callback(msg);
        } else {
        	char log_message[256];
			snprintf(log_message, sizeof(log_message), "I as led: %d, should be dead", hao->color);
			LOGGER_INFO(log_message);
			vTaskDelay((TickType_t)(1000 / portTICK_PERIOD_MS));
        }
    }
}

/********************** external functions definition ************************/

bool ao_led_send(ao_led_handler_t* hao, ao_led_message_t* msg) {
    return (pdPASS == xQueueSend(hao->hqueue, &msg, 0));
}

void ao_led_init(ao_led_handler_t* hao, ao_led_color_t color) {
    hao->color = color;

    hao->hqueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    while(NULL == hao->hqueue) {
        // error
    }


    BaseType_t status = xTaskCreate(task, "task_ao_led", 512, (void * const) hao, tskIDLE_PRIORITY, NULL);
    while (pdPASS != status) {
        // error
    }
}

/********************** end of file ******************************************/
