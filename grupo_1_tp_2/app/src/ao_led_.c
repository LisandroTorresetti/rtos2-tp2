#include "ao_led.h"
#include "board.h"

#define QUEUE_LENGTH            (1)
#define QUEUE_ITEM_SIZE         (sizeof(ao_led_message_t))

static const char* TASK_NAME = "task_ao_led";


static void task(void *argument) {
    ao_led_handler_t* hao = argument;
    while (true) {
        ao_led_message_t msg;

        if (pdPASS == xQueueReceive(hao->hqueue, &msg, portMAX_DELAY)) {
        	GPIO_PinState action = msg.turn_on ? GPIO_PIN_SET : GPIO_PIN_RESET;
        	HAL_GPIO_WritePin(hao->led_port, hao->led_pin, action);
        	// aca deberia morir la task si no hay mas nada encolado
        	// el tema es si mato y despues encolo, ahi nada procesa
        	// de ultima se mete un lock a ese handler
        	// para ver que onda
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

bool ao_led_send(ao_led_handler_t* hao, ao_led_message_t* msg) {
	// aca tiene que haber una logica para crear si no hay handlers

    return (pdPASS == xQueueSend(hao->hqueue, (void*)msg, 0));
}

/********************** end of file ******************************************/
