#include "ao_led.h"
#include "board.h"
#include "main.h"

#define QUEUE_LENGTH            (1)
#define QUEUE_ITEM_SIZE         (sizeof(ao_led_message_t))

static GPIO_TypeDef* led_port[] = {LED_RED_PORT, LED_GREEN_PORT,  LED_BLUE_PORT};
static uint16_t led_pin[] = {LED_RED_PIN,  LED_GREEN_PIN, LED_BLUE_PIN };


static void task(void *argument)
{
    ao_led_handler_t* hao = argument;
    while (true) {
        ao_led_message_t msg;

        if (pdPASS == xQueueReceive(hao->hqueue, &msg, portMAX_DELAY)) {
            HAL_GPIO_WritePin(led_port[hao->color], led_pin[hao->color], GPIO_PIN_SET);
            vTaskDelay(msg.blink_time / portTICK_PERIOD_MS);
            HAL_GPIO_WritePin(led_port[hao->color], led_pin[hao->color], GPIO_PIN_RESET);
        }
    }
}

/********************** external functions definition ************************/

bool ao_led_send(ao_led_handler_t* hao, ao_led_message_t* msg) {
    return (pdPASS == xQueueSend(hao->hqueue, (void*)msg, 0));
}

void ao_led_init(ao_led_handler_t* hao, ao_led_color_t color) {
    hao->color = color;

    hao->hqueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    while(NULL == hao->hqueue) {
        // error
    }

    BaseType_t status = xTaskCreate(task, "task_ao_led", 128, (void * const) hao, tskIDLE_PRIORITY, NULL);
    while (pdPASS != status) {
        // error
    }
}

/********************** end of file ******************************************/
