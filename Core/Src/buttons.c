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

/* Determines whether input position overlaps input virtual button */
uint8_t On_Btn(uint16_t x, uint16_t y, button_t btn) {
   return (x >= btn.x) && (x < btn.x + btn.w) &&
          (y >= btn.y) && (y < btn.y + btn.h);
}

/* Initializes GPIO, EXTI, timer for physical button (PA0, PA1) */
void Button_Init() {
    // configure GPIO
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    // input mode
    GPIOA->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1);
    // pull down
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1);
    GPIOA->PUPDR |= (2 << GPIO_PUPDR_PUPD0_Pos) | (2 << GPIO_PUPDR_PUPD1_Pos);

    // configure EXTI0 for rising (PA0), EXTI1 for falling (PA1)
    // unmask lines 0, 1
    EXTI->IMR1 |= EXTI_IMR1_IM0 | EXTI_IMR1_IM1;
    // enable rising trigger
    EXTI->RTSR1 |= EXTI_RTSR1_RT0;
    // enable falling trigger
    EXTI->FTSR1 |= EXTI_FTSR1_FT1;
    // connect pins to EXTI
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR1_EXTI0;
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR1_EXTI1;

    // configure debounce timer
    // enable clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    // set reload value
    TIM2->ARR = PRESS_MIN_CYCLES - 1;
    // enabling interrupt for overflow
    TIM2->DIER = TIM_DIER_UIE;
    // don't enable timer

    // set up interrupts
    // timer
    uint8_t irq_idx = TIM2_IRQn >> 5;
    NVIC->ISER[irq_idx] = 1 << (TIM2_IRQn & 0x1F);
    // exti 0
    irq_idx = EXTI0_IRQn >> 5;
    NVIC->ISER[irq_idx] = 1 << (EXTI0_IRQn & 0x1F);
    // exti 1
    irq_idx = EXTI1_IRQn >> 5;
    NVIC->ISER[irq_idx] = 1 << (EXTI1_IRQn & 0x1F);
}
