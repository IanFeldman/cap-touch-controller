#include "main.h"
#include "terminaluart.h"
#include <stdio.h>
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
void UART_Update_Screen(state_t state, uint8_t canvas_size) {
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
        case SIZING:
            UART_Print_Esc("[10;97H"); UART_Print("SIZING");

            UART_Print_Esc("[20;70H"); UART_Print(" _________ ");
            UART_Print_Esc("[21;70H"); UART_Print("|         |");
            UART_Print_Esc("[22;70H"); UART_Print("|  SMALL  |");
            UART_Print_Esc("[23;70H"); UART_Print("|  32x8   |");
            UART_Print_Esc("[24;70H"); UART_Print("|_________|");

            UART_Print_Esc("[19;93H"); UART_Print(" ____________ ");
            UART_Print_Esc("[20;93H"); UART_Print("|            |");
            UART_Print_Esc("[21;93H"); UART_Print("|            |");
            UART_Print_Esc("[22;93H"); UART_Print("|   MEDIUM   |");
            UART_Print_Esc("[23;93H"); UART_Print("|   64x16    |");
            UART_Print_Esc("[24;93H"); UART_Print("|            |");
            UART_Print_Esc("[25;93H"); UART_Print("|____________|");

            UART_Print_Esc("[18;119H"); UART_Print(" _______________");
            UART_Print_Esc("[19;119H"); UART_Print("|               |");
            UART_Print_Esc("[20;119H"); UART_Print("|               |");
            UART_Print_Esc("[21;119H"); UART_Print("|               |");
            UART_Print_Esc("[22;119H"); UART_Print("|     LARGE     |");
            UART_Print_Esc("[23;119H"); UART_Print("|     128x32    |");
            UART_Print_Esc("[24;119H"); UART_Print("|               |");
            UART_Print_Esc("[25;119H"); UART_Print("|               |");
            UART_Print_Esc("[26;119H"); UART_Print("|_______________|");

            UART_Print_Esc("[34;97H"); UART_Print("------");
            UART_Print_Esc("[35;97H"); UART_Print(" BACK ");
            UART_Print_Esc("[36;97H"); UART_Print("------");
            break;
        case CANVAS:
            UART_Print_Esc("[5;97H"); UART_Print("CANVAS");
            // draw canvas
            // background to white
            UART_Print_Esc("[47m");
            // move cursor
            uint8_t height = canvas_size;
            uint8_t width  = canvas_size << 2;
            uint16_t y_min = (TERMINAL_HEIGHT >> 1) - (height >> 1);
            uint16_t x_min = (TERMINAL_WIDTH  >> 1) - (width  >> 1);
            char buff[BUFF_LEN];
            sprintf(buff, "[%u;%uH", y_min, x_min);
            UART_Print_Esc(buff);
            for (uint8_t i = 1; i <= height; i++) {
                for (uint8_t j = 0; j < width; j++) {
                    UART_Print_Char(BLANK_CHAR);
                }
                // move cursor down
                sprintf(buff, "[%u;%uH", y_min + i, x_min);
                UART_Print_Esc(buff);
            }

            // color buttons
            uint8_t color_btn_width  = 8;
            uint8_t color_btn_height = 3;
            uint16_t color_btn_x = 170;
            uint16_t color_btn_y = 14;
            // red
            UART_Print_Esc("[41m");
            UART_Draw_Box(BLANK_CHAR, color_btn_x, color_btn_y + 0, color_btn_width, color_btn_height);
            // blue
            UART_Print_Esc("[42m");
            UART_Draw_Box(BLANK_CHAR, color_btn_x, color_btn_y + 4, color_btn_width, color_btn_height);
            // green
            UART_Print_Esc("[44m");
            UART_Draw_Box(BLANK_CHAR, color_btn_x, color_btn_y + 8, color_btn_width, color_btn_height);
            // white
            UART_Print_Esc("[47m");
            UART_Draw_Box(BLANK_CHAR, color_btn_x, color_btn_y + 12, color_btn_width, color_btn_height);
            // black
            UART_Print_Esc("[40m");
            UART_Draw_Box(BLANK_CHAR, color_btn_x, color_btn_y + 16, color_btn_width, color_btn_height);

            // reset background
            UART_Print_Esc("[0m");
            //  done
            UART_Print_Esc("[44;97H"); UART_Print("------");
            UART_Print_Esc("[45;97H"); UART_Print(" DONE ");
            UART_Print_Esc("[46;97H"); UART_Print("------");
            break;
        case SAVE:
            UART_Print_Esc("[20;85H"); UART_Print("SAVE");
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

void UART_Draw_Box(char c, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    // move cursor
    char buff[BUFF_LEN];
    sprintf(buff, "[%u;%uH", y, x);
    UART_Print_Esc(buff);

    for (uint16_t i = 1; i <= h; i++) {
        for (uint16_t j = 0; j < w; j++) {
            UART_Print_Char(c);
        }
        // move cursor down
        sprintf(buff, "[%u;%uH", y + i, x);
        UART_Print_Esc(buff);
    }
}
