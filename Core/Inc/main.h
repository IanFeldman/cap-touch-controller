
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"
#include "touchscreen.h"

#define TERMINAL_WIDTH  138
#define TERMINAL_HEIGHT 40

// sizings
#define SIZE_SMALL_X  32
#define SIZE_SMALL_Y   8
#define SIZE_MEDIUM_X 64
#define SIZE_MEDIUM_Y 16
#define SIZE_LARGE_X 128
#define SIZE_LARGE_Y  30 // 256 bytes extra for header

#define BUFF_LEN 20

#define CHAR_DELETE 127
#define CHAR_RETURN 13

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

// properties that must be passed between On_Click() and On_Press()
typedef struct {
    uint8_t canvas_width;
    uint8_t canvas_height;
    uint8_t cursor_allowed;
    uint8_t *image;
} properties_t;

void Error_Handler(void);
void SystemClock_Config(void);
void Move_Cursor(status_t status);
void On_Click(state_t *state, status_t status, properties_t *properties);
void On_Press(state_t *state, properties_t *properties);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
