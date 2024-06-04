#include "buttons.h"

const button_t BTN_TITLE_NEW        = { 80, 20, 40, 20 };
const button_t BTN_TITLE_OPEN       = { 80, 20, 40, 20 };
const button_t BTN_PROPERTIES_BACK  = { 80, 20, 40, 20 };
const button_t BTN_PROPERTIES_START = { 80, 20, 40, 20 };
const button_t BTN_CANVAS_DONE      = { 80, 20, 40, 20 };
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
