#ifndef INC_BUTTONS_H_
#define INC_BUTTONS_H_

#include <stdint.h>

// must press for 1 ms
#define PRESS_MIN_CYCLES 4000

typedef struct {
    uint16_t x, y, w, h;
} button_t;

extern const button_t BTN_TITLE_NEW;
extern const button_t BTN_TITLE_OPEN;

extern const button_t BTN_SIZING_SMALL;
extern const button_t BTN_SIZING_MEDIUM;
extern const button_t BTN_SIZING_LARGE;
extern const button_t BTN_SIZING_BACK;

extern const button_t BTN_CANVAS_RED;
extern const button_t BTN_CANVAS_GREEN;
extern const button_t BTN_CANVAS_BLUE;
extern const button_t BTN_CANVAS_WHITE;
extern const button_t BTN_CANVAS_BLACK;
extern const button_t BTN_CANVAS_DONE;

extern const button_t BTN_SAVE_DONE;
extern const button_t BTN_BROWSER_BACK;
extern const button_t BTN_BROWSER_SELECT;
extern const button_t BTN_OPTIONS_EDIT;
extern const button_t BTN_OPTIONS_BACK;
extern const button_t BTN_OPTIONS_PREVIEW;
extern const button_t BTN_PREVIEW_DONE;

uint8_t On_Btn(uint16_t x, uint16_t y, button_t btn);

void Button_Init();

#endif
