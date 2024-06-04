#include "main.h"
#include "terminaluart.h"
#include <stdlib.h>

/* initialize UART for 115200 baud rate communication */
void UART_Init() {
    // USART GPIO pins
    // clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    // alternate func
    GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
    GPIOA->MODER |= (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);
    // alt func: 7
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3);
    GPIOA->AFR[0] |= ((7 << GPIO_AFRL_AFSEL2_Pos) | (7 << GPIO_AFRL_AFSEL3_Pos));

    // UART power
    PWR->CR2 |= PWR_CR2_IOSV;
    // UART clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    // receive interrupts
    USART2->CR1 |= USART_CR1_RXNEIE;

    // Divider = 34.72
    USART2->BRR = 35;
    // 1 stop bit
    USART2->CR2 &= 0;
    // Enable USART
    USART2->CR1 |= USART_CR1_UE;
    // Set TE, bits
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;

    // set up interrupts
    uint8_t irq_idx = USART2_IRQn >> 5; // divide by 32
    NVIC->ISER[irq_idx] = 1 << (USART2_IRQn & NVIC_MSK); // get remainder
    // global interrupts
    __enable_irq();
}

/* print single character */
void UART_Print_Char(char data) {
    // wait for TXE to be true
    while (!(USART2->ISR & USART_ISR_TXE)) {}
    // write data
    USART2->TDR = data;
    // wait until frame complete
    while (!(USART2->ISR & USART_ISR_TC)) {}
}

/* print single string */
void UART_Print(char *string) {
    for (; *string != '\0'; string++) {
        UART_Print_Char(*string);
    }
}

/* print escape code */
void UART_Print_Esc(char *string) {
    UART_Print_Char(ESC_CHAR);
    UART_Print(string);
}

/* clear screen */
void UART_Reset_Screen() {
    UART_Print_Esc("[2J"); // clear screen
    UART_Print_Esc("[0m"); // disable attributes
    UART_Print_Esc("[H");  // move cursor back to top
}

/* update the screen based on what state it just switched to */
void UART_Update_Screen(state_t state) {
    // save cursor position
    UART_Print_Esc("7");
    UART_Reset_Screen();

    switch(state) {
        case TITLE:
            UART_Print_Esc("[20;85H"); UART_Print(" ____   _    ___ _   _ _______   __");
            UART_Print_Esc("[21;85H"); UART_Print("|  _ \\ / \\  |_ _| \\ | |_   _\\ \\ / /");
            UART_Print_Esc("[22;85H"); UART_Print("| |_) / _ \\  | ||  \\| | | |  \\ V / ");
            UART_Print_Esc("[23;85H"); UART_Print("|  __/ ___ \\ | || |\\  | | |   | |  ");
            UART_Print_Esc("[24;85H"); UART_Print("|_| /_/   \\_\\___|_| \\_| |_|   |_|  ");

            UART_Print_Esc("[29;90H"); UART_Print("-----");
            UART_Print_Esc("[30;90H"); UART_Print(" NEW ");
            UART_Print_Esc("[31;90H"); UART_Print("-----");

            UART_Print_Esc("[29;110H"); UART_Print("------");
            UART_Print_Esc("[30;110H"); UART_Print(" OPEN ");
            UART_Print_Esc("[31;110H"); UART_Print("------");
            break;
        case PROPERTIES:
            UART_Print_Esc("[20;85H"); UART_Print("PROPERTIES");
            break;
        case BROWSER:
            UART_Print_Esc("[20;85H"); UART_Print("BROWSER");
            break;
        default:
            break;
    }

    // restore cursor position
    UART_Print_Esc("8");
}
