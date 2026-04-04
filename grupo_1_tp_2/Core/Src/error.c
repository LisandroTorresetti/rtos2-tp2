#include "error.h"
#include "main.h"

void error_signal() {
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	while (1) {};
}

