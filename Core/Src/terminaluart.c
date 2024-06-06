#include "main.h"
#include "terminaluart.h"
#include <stdio.h>
#include <stdlib.h>

/* initialize UART for 115200 baud rate communication */
void UART_Init() {
    // USART GPIO pins
    // PA2, PA3

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
    NVIC->ISER[irq_idx] = 1 << (USART2_IRQn & 0x1F); // get remainder
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
void UART_Update_Screen(state_t state, uint8_t canvas_size_x, uint8_t canvas_size_y) {
    // save cursor position
    UART_Print_Esc("7");
    UART_Reset_Screen();

    switch(state) {
        case TITLE:
            UART_Print_Esc("[15;45H"); UART_Print(" _____                   ____  _        _       _ "          );
            UART_Print_Esc("[16;45H"); UART_Print("|_   _|__ _ __ _ __ ___ / ___|| | _____| |_ ___| |__"        );
            UART_Print_Esc("[17;45H"); UART_Print("  | |/ _ \\ '__| '_ ` _ \\\\___ \\| |/ / _ \\ __/ __| '_ \\" );
            UART_Print_Esc("[18;45H"); UART_Print("  | |  __/ |  | | | | | |___) |   <  __/ || (__| | | |"      );
            UART_Print_Esc("[19;45H"); UART_Print("  |_|\\___|_|  |_| |_| |_|____/|_|\\_\\___|\\__\\___|_| |_|" );

            UART_Print_Esc("[25;60H"); UART_Print("-----");
            UART_Print_Esc("[26;60H"); UART_Print(" NEW ");
            UART_Print_Esc("[27;60H"); UART_Print("-----");

            UART_Print_Esc("[25;78H"); UART_Print("------");
            UART_Print_Esc("[26;78H"); UART_Print(" OPEN ");
            UART_Print_Esc("[27;78H"); UART_Print("------");
            break;
        case SIZING:
            UART_Print_Esc("[7;64H"); UART_Print("== SIZING ==");

            UART_Print_Esc("[17;40H"); UART_Print(" _________ ");
            UART_Print_Esc("[18;40H"); UART_Print("|         |");
            UART_Print_Esc("[19;40H"); UART_Print("|  SMALL  |");
            UART_Print_Esc("[20;40H"); UART_Print("|  32x8   |");
            UART_Print_Esc("[21;40H"); UART_Print("|_________|");

            UART_Print_Esc("[16;63H"); UART_Print(" ____________ ");
            UART_Print_Esc("[17;63H"); UART_Print("|            |");
            UART_Print_Esc("[18;63H"); UART_Print("|            |");
            UART_Print_Esc("[19;63H"); UART_Print("|   MEDIUM   |");
            UART_Print_Esc("[20;63H"); UART_Print("|   64x16    |");
            UART_Print_Esc("[21;63H"); UART_Print("|            |");
            UART_Print_Esc("[22;63H"); UART_Print("|____________|");

            UART_Print_Esc("[15;89H"); UART_Print(" _______________");
            UART_Print_Esc("[16;89H"); UART_Print("|               |");
            UART_Print_Esc("[17;89H"); UART_Print("|               |");
            UART_Print_Esc("[18;89H"); UART_Print("|               |");
            UART_Print_Esc("[19;89H"); UART_Print("|     LARGE     |");
            UART_Print_Esc("[20;89H"); UART_Print("|     128x32    |");
            UART_Print_Esc("[21;89H"); UART_Print("|               |");
            UART_Print_Esc("[22;89H"); UART_Print("|               |");
            UART_Print_Esc("[23;89H"); UART_Print("|_______________|");

            UART_Print_Esc("[30;67H"); UART_Print("------");
            UART_Print_Esc("[31;67H"); UART_Print(" BACK ");
            UART_Print_Esc("[32;67H"); UART_Print("------");
            break;
        case CANVAS:
            UART_Print_Esc("[2;64H"); UART_Print("== CANVAS ==");
            // draw canvas
            // background to white
            UART_Print_Esc("[47m");
            // move cursor
            uint8_t width  = canvas_size_x;
            uint8_t height = canvas_size_y;
            uint16_t x_min = (TERMINAL_WIDTH  >> 1) - (width  >> 1) + 1;
            uint16_t y_min = (TERMINAL_HEIGHT >> 1) - (height >> 1);
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
            uint16_t color_btn_x = 8;
            uint16_t color_btn_y = 37;

            // red
            UART_Print_Esc("[41m");
            UART_Draw_Box(BLANK_CHAR, color_btn_x + 0, color_btn_y, color_btn_width, color_btn_height);
            // blue
            UART_Print_Esc("[42m");
            UART_Draw_Box(BLANK_CHAR, color_btn_x + 8, color_btn_y, color_btn_width, color_btn_height);
            // green
            UART_Print_Esc("[44m");
            UART_Draw_Box(BLANK_CHAR, color_btn_x + 16, color_btn_y, color_btn_width, color_btn_height);
            // white
            UART_Print_Esc("[47m");
            UART_Draw_Box(BLANK_CHAR, color_btn_x + 24, color_btn_y, color_btn_width, color_btn_height);
            // black
            UART_Print_Esc("[40m");
            UART_Draw_Box(BLANK_CHAR, color_btn_x + 32, color_btn_y, color_btn_width, color_btn_height);

            // reset background
            UART_Print_Esc("[0m");
            //  done
            UART_Print_Esc("[37;67H"); UART_Print("------");
            UART_Print_Esc("[38;67H"); UART_Print(" DONE ");
            UART_Print_Esc("[39;67H"); UART_Print("------");
            break;
        case SAVE:
            UART_Print_Esc("[10;64H"); UART_Print("== SAVE ==");
            UART_Print_Esc("[15;40H"); UART_Print("Save as:                                  (32 character max)");
            UART_Print_Esc("[20;60H"); UART_Print("Hit [Enter] when done");
            // move cursor back to type spot
            UART_Print_Esc("[15;49H");
            // return so dont restore cursor position
            return;
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
