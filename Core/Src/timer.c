#include "stm32l4xx_hal.h"
#include "timer.h"

void TIMER_Init(void) {
    // PA5

    // configure GPIO
    // clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    // alt func mode
    GPIOA->MODER &= ~GPIO_MODER_MODE5;
    GPIOA->MODER |=  GPIO_MODER_MODE5_1;
    // func 1
    GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL5_Pos;
    GPIOA->AFR[0] |= 1 << GPIO_AFRL_AFSEL5_Pos;
    // output type push pull
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT5;

    // enable clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    // PWM on channel 1
    // pwm mode 1, preload enabled (required)
    TIM2->CCMR1 = (6 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
    // auto reload preload (required)
    TIM2->CR1 = TIM_CR1_ARPE;
    // update generation
    TIM2->EGR = TIM_EGR_CC1G | TIM_EGR_UG;
    // enable capture/comp 1
    TIM2->CCER = TIM_CCER_CC1E;

    // set frequency
    TIM2->ARR = ARR_FREQ;
    // set duty
    TIM2->CCR1 = PWM_DUTY;

    // enable timer
    TIM2->CR1 = TIM_CR1_CEN;
}
