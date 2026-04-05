#include "ao_led.h"
#include "board.h"

#define QUEUE_LENGTH            (1)
#define QUEUE_ITEM_SIZE         (sizeof(action_message_t*))

static const char* TASK_NAME = "task_ao_led";


static void task(void *argument) {
    ao_led_handler_t* hao = argument;
    while (true) {
    	action_message_t* msg;

        if (pdPASS == xQueueReceive(hao->hqueue, &msg, portMAX_DELAY)) {
        	GPIO_PinState action = (msg->led_action == TURN_ON) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        	HAL_GPIO_WritePin(hao->led_port, hao->led_pin, action);
        	msg->callback(msg);
        }
    }
}

static app_err_t create_resources(ao_led_handler_t* handler_ao) {
    BaseType_t status = xTaskCreate(task, TASK_NAME, 512, (void * const) handler_ao, tskIDLE_PRIORITY, NULL);
    if (pdPASS != status) {
        return ERR_NO_MEMORY;
    }

    return APP_OK;
}

static void message_processed_callback(void* msg) {
	vPortFree(msg);
}

/********************** external functions definition ************************/

app_err_t ao_led_init(ao_led_handler_t* handler_ao, GPIO_TypeDef* led_port, uint16_t led_pin) {
	handler_ao->led_port = led_port;
	handler_ao->led_pin = led_pin;

	handler_ao->hqueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    if (!handler_ao->hqueue) {
        return ERR_NO_MEMORY;
    }

	return create_resources(handler_ao);
}

void ao_led_send(ao_led_handler_t* handler_ao, led_action_t action) {
	action_message_t* msg = (action_message_t*)pvPortMalloc(sizeof(action_message_t));
	if (msg == NULL) {
		mayday_signal();
		return;
	}

	msg->led_action = action;
	msg->callback = message_processed_callback;

    if (pdPASS != xQueueSend(handler_ao->hqueue, (void*)&msg, 0)) {
    	mayday_signal();
    	vPortFree(msg);
    }
}

/********************** end of file ******************************************/
