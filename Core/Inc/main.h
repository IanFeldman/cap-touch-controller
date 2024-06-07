
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "buttons.h"
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
#define CHAR_ESCAPE 27

#define COLOR_RED   1
#define COLOR_GREEN 2
#define COLOR_BLUE  4
#define COLOR_WHITE 7
#define COLOR_BLACK 0

#define BLOCK_IDX_UNUSED 0xFF

// FSM
typedef enum {
    TITLE,
    SIZING,
    CANVAS,
    SAVE,
    BROWSER,
} state_t;

typedef struct {
    state_t  state;
    button_t canvas;
    uint8_t  block_idx;
    uint8_t  cursor_allowed;
} info_t;

void Error_Handler(void);
void SystemClock_Config(void);
void Move_Cursor(status_t status);
void On_Click(info_t *info, status_t status);
void On_Press(info_t *info);
void Get_Canvas_XY(button_t *canvas);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
