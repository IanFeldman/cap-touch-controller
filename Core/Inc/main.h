
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"
#include "touchscreen.h"

/*
#define TERMINAL_HEIGHT 50
#define TERMINAL_WIDTH  200
*/
#define TERMINAL_HEIGHT 40
#define TERMINAL_WIDTH  138

// y dimension
#define SIZE_SMALL  8
#define SIZE_MEDIUM 16
#define SIZE_LARGE  32

#define BUFF_LEN 14

// FSM
typedef enum {
    TITLE,
    SIZING,
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
