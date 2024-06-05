#ifndef INC_TOUCHSCREEN_H_
#define INC_TOUCHSCREEN_H_

#define TIMING_FACTOR 0x7
#define DEVICE_ADDRESS 0x38

#define TOUCH_Y_MAX 240
#define TOUCH_X_MAX 320

#include "stm32l4xx_hal.h"

typedef struct {
    uint8_t  update;
    uint8_t touch_y;
    uint16_t  touch_x;
    uint16_t screen_y;
    uint16_t screen_x;
} status_t;

void Touchscreen_Init();
void Touchscreen_Read(status_t *status);

#endif
