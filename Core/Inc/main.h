
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"
#include "touchscreen.h"

#define TERMINAL_HEIGHT 50
#define TERMINAL_WIDTH  200

// FSM
typedef enum {
    TITLE,
    PROPERTIES,
    CANVAS,
    SAVE,
    BROWSER,
    OPTIONS,
    PREVIEW
} state_t;

void Error_Handler(void);
void SystemClock_Config(void);
void Move_Cursor(status_t status);
void On_Click(state_t *state, status_t status);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
