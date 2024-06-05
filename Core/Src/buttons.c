#include "buttons.h"

const button_t BTN_TITLE_NEW        = { 90,  29, 5, 3 };
const button_t BTN_TITLE_OPEN       = { 110, 29, 6, 3 };

const button_t BTN_SIZING_SMALL     = {  70, 20, 11, 5 };
const button_t BTN_SIZING_MEDIUM    = {  93, 19, 14, 7 };
const button_t BTN_SIZING_LARGE     = { 119, 18, 17, 9 };
const button_t BTN_SIZING_BACK      = {  97, 34,  6, 3 };

const button_t BTN_CANVAS_RED       = { 170, 14, 8, 3 };
const button_t BTN_CANVAS_GREEN     = { 170, 18, 8, 3 };
const button_t BTN_CANVAS_BLUE      = { 170, 22, 8, 3 };
const button_t BTN_CANVAS_WHITE     = { 170, 26, 8, 3 };
const button_t BTN_CANVAS_BLACK     = { 170, 30, 8, 3 };
const button_t BTN_CANVAS_DONE      = { 97,  44, 6, 3 };

const button_t BTN_SAVE_DONE        = { 80, 20, 40, 20 };
const button_t BTN_BROWSER_BACK     = { 80, 20, 40, 20 };
const button_t BTN_BROWSER_SELECT   = { 80, 20, 40, 20 };
const button_t BTN_OPTIONS_EDIT     = { 80, 20, 40, 20 };
const button_t BTN_OPTIONS_BACK     = { 80, 20, 40, 20 };
const button_t BTN_OPTIONS_PREVIEW  = { 80, 20, 40, 20 };
const button_t BTN_PREVIEW_DONE     = { 80, 20, 40, 20 };

uint8_t On_Btn(uint16_t x, uint16_t y, button_t btn) {
   return (x >= btn.x) && (x < btn.x + btn.w) &&
          (y >= btn.y) && (y < btn.y + btn.h);
}
