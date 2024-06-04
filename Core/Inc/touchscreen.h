#ifndef INC_TOUCHSCREEN_H_
#define INC_TOUCHSCREEN_H_

#define TIMING_FACTOR 0xD15
#define DEVICE_ADDRESS 0x38

#include "stm32l4xx_hal.h"

typedef struct {
    uint8_t  update;
    uint8_t  pos_v;
    uint16_t pos_h;
} status_t;

void TOUCH_Init();
void TOUCH_Read(status_t *status);

#endif
