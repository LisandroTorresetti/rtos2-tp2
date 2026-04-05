#ifndef INC_ERROR_H_
#define INC_ERROR_H_

#include <stdint.h>

typedef int32_t app_err_t;

#define APP_OK 0
#define ERR_INTERNAL -1
#define ERR_UNKNOWN -2
#define ERR_NO_MEMORY -3


void error_signal();

void mayday_signal();

#endif /* INC_ERROR_H_ */
