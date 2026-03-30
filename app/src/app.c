/********************** inclusions *******************************************/

#include "ao_led.h"
#include "ao_ui.h"
#include "main.h"
#include "cmsis_os.h"
#include "logger.h"
#include "dwt.h"

#include "task_button.h"

/********************** macros and definitions *******************************/


/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data declaration *****************************/

ao_led_handler_t led_red, led_green, led_blue;

/********************** external functions definition ************************/
void app_init(void)
{
  // Init led handlers
  ao_led_init(&led_red, AO_LED_COLOR_RED);
  ao_led_init(&led_green, AO_LED_COLOR_GREEN);
  ao_led_init(&led_blue, AO_LED_COLOR_BLUE);

  ao_led_handler_t colours[3] = { led_red, led_green, led_blue };
  ao_ui_init(colours);

  BaseType_t status = xTaskCreate(task_button, "task_button", 128, NULL, tskIDLE_PRIORITY, NULL);
  while (pdPASS != status)
  {
    // error
  }

  LOGGER_INFO("app init");

  cycle_counter_init();
}

/********************** end of file ******************************************/
