#include "buttons.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

const button_t BTN_TITLE = { 45, 15, 54, 5,
        " _____                   ____  _        _       _     "
        "|_   _|__ _ __ _ __ ___ / ___|| | _____| |_ ___| |__  "
        "  | |/ _ \\ '__| '_ ` _ \\\\___ \\| |/ / _ \\ __/ __| '_ \\ "
        "  | |  __/ |  | | | | | |___) |   <  __/ || (__| | | |"
        "  |_|\\___|_|  |_| |_| |_|____/|_|\\_\\___|\\__\\___|_| |_|"
};
const button_t BTN_TITLE_NEW  = { 60, 25, 5, 3,
        "-----"
        " NEW "
        "-----"
};

const button_t BTN_TITLE_OPEN = { 78, 25, 6, 3,
        "------"
        " OPEN "
        "------"
};
const button_t BTN_SIZING_SMALL = { 40, 17, 11, 5,
        " _________ "
        "|         |"
        "|  SMALL  |"
        "|  32x8   |"
        "|_________|"
};
const button_t BTN_SIZING_MEDIUM    = { 63, 16, 14, 7,
        " ____________ "
        "|            |"
        "|            |"
        "|   MEDIUM   |"
        "|   64x16    |"
        "|            |"
        "|____________|"
};
const button_t BTN_SIZING_LARGE     = { 89, 15, 17, 9,
        " _______________ "
        "|               |"
        "|               |"
        "|               |"
        "|     LARGE     |"
        "|     128x32    |"
        "|               |"
        "|               |"
        "|_______________|"
};
const button_t BTN_SIZING_BACK      = { 67, 30, 6, 3,
        "------"
        " BACK "
        "------"
};
const button_t BTN_CANVAS_RED       = { 50, 37, 8, 3, 0 };
const button_t BTN_CANVAS_GREEN     = { 58, 37, 8, 3, 0 };
const button_t BTN_CANVAS_BLUE      = { 66, 37, 8, 3, 0 };
const button_t BTN_CANVAS_WHITE     = { 74, 37, 8, 3, 0 };
const button_t BTN_CANVAS_BLACK     = { 82, 37, 8, 3, 0 };
const button_t BTN_CANVAS_SAVE      = { 110, 37, 6, 3,
        "------"
        " SAVE "
        "------"
};
const button_t BTN_CANVAS_DELETE    = { 20, 37, 8, 3,
        "--------"
        " DELETE "
        "--------"
};
const button_t BTN_BROWSER_ENTRY = { 0, 0, 50, 3,
        "--------------------------------------------------"
        "                                                  "
        "--------------------------------------------------"
};
const button_t BTN_BROWSER_BACK     = { 67, 37, 6, 3,
        "------"
        " BACK "
        "------"
};

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
