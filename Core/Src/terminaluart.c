#include "buttons.h"
#include "eeprom.h"
#include "main.h"
#include "terminaluart.h"
#include <stdint.h>
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
void UART_Update_Screen(state_t state, button_t canvas) {
    // save cursor position
    UART_Print_Esc("7");
    UART_Reset_Screen();

    switch(state) {
        case TITLE:
            UART_Print_Btn(BTN_TITLE, 0);
            UART_Print_Btn(BTN_TITLE_NEW, 0);
            UART_Print_Btn(BTN_TITLE_OPEN, 0);
            break;

        case SIZING:
            // check if there is space to make new block
            if (EEPROM_Find_Free_Block() == NULL_BLOCK_INDEX) {
                UART_Print_Esc("[19;45H");
                UART_Print("Memory full. Please delete sketches to make space.");
                UART_Print_Btn(BTN_SIZING_BACK, 0);
                break;
            }

            UART_Print_Esc("[7;64H");
            UART_Print("== SIZING ==");

            UART_Print_Btn(BTN_SIZING_SMALL, 0);
            UART_Print_Btn(BTN_SIZING_MEDIUM, 0);
            UART_Print_Btn(BTN_SIZING_LARGE, 0);

            UART_Print_Btn(BTN_SIZING_BACK, 0);
            break;

        case CANVAS:
            UART_Print_Esc("[2;64H");
            UART_Print("== CANVAS ==");
            // background to white
            UART_Print_Esc("[47m");
            UART_Print_Canvas(canvas);
            // red
            UART_Print_Esc("[41m");
            UART_Print_Btn(BTN_CANVAS_RED, 1);
            // green
            UART_Print_Esc("[42m");
            UART_Print_Btn(BTN_CANVAS_GREEN, 1);
            // blue
            UART_Print_Esc("[44m");
            UART_Print_Btn(BTN_CANVAS_BLUE, 1);
            // white
            UART_Print_Esc("[47m");
            UART_Print_Btn(BTN_CANVAS_WHITE, 1);
            // black
            UART_Print_Esc("[40m");
            UART_Print_Btn(BTN_CANVAS_BLACK, 1);
            // reset background
            UART_Print_Esc("[0m");
            UART_Print_Btn(BTN_CANVAS_SAVE, 0);
            UART_Print_Btn(BTN_CANVAS_DELETE, 0);
            break;

        case SAVE:
            UART_Print_Esc("[10;66H");
            UART_Print("== SAVE ==");

            UART_Print_Esc("[15;45H");
            UART_Print("Save as:                                  (32 character max)");

            UART_Print_Esc("[20;60H");
            UART_Print("Hit [Enter] when done");

            // move cursor back to type spot
            UART_Print_Esc("[15;54H");
            // return so dont restore cursor position
            return;

        case BROWSER:
            UART_Print_Esc("[2;64H");
            UART_Print("== BROWSER ==");

            // iterate over all files
            header_t files[MEM_BLOCK_CNT];
            uint16_t x = BTN_BROWSER_ENTRY_COL1_X;
            uint16_t y = BTN_BROWSER_ENTRY_INIT_Y;
            for (uint8_t i = 0; i < MEM_BLOCK_CNT; i++) {
                // get header
                EEPROM_Read_Header(&files[i], i);
                // create button
                button_t entry = BTN_BROWSER_ENTRY;
                entry.x = x;
                entry.y = y;
                UART_Print_Btn(entry, 0);

                // move cursor
                char buff[BUFF_LEN];
                sprintf(buff, "[%u;%uH", y + 1, x);
                UART_Print_Esc(buff);

                if (files[i].block_used) {
                    // print name
                    UART_Print(files[i].name);
                    // print size
                    sprintf(buff, " (%ux%u)", files[i].size_x, files[i].size_y);
                    UART_Print(buff);
                }
                else {
                    UART_Print("NO FILE FOUND");
                }

                y += BTN_BROWSER_ENTRY_SPACING_Y;

                if (i == (MEM_BLOCK_CNT / 2) - 1) {
                    x = BTN_BROWSER_ENTRY_COL2_X;
                    y = BTN_BROWSER_ENTRY_INIT_Y;
                }
            }

            //  done
            UART_Print_Btn(BTN_BROWSER_BACK, 0);
            break;
        default:
            break;
    }

    // restore cursor position
    UART_Print_Esc("8");
}

/* prints a button at its position
 * if is_solid==0, will print the button text
 * otherwise prints blank*/
void UART_Print_Btn(button_t btn, uint8_t is_solid) {
    char buff[BUFF_LEN];
    for (uint16_t j = 0; j < btn.h; j++) {
        // move cursor
        sprintf(buff, "[%u;%uH", btn.y + j, btn.x);
        UART_Print_Esc(buff);
        for (uint16_t i = 0; i < btn.w; i++) {
            uint8_t c = btn.data[j * btn.w + i];
            if (is_solid) {
                c = CANVAS_CHAR;
            }
            UART_Print_Char(c);
        }
    }
}

/* Prints the canvas. Loads canvas image if canvas.data != 0*/
void UART_Print_Canvas(button_t canvas) {
    // set color to white by default
    UART_Print_Esc("[47m");
    char buff[BUFF_LEN];
    for (uint16_t j = 0; j < canvas.h; j++) {
        // move cursor
        sprintf(buff, "[%u;%uH", canvas.y + j, canvas.x);
        UART_Print_Esc(buff);
        // row
        for (uint16_t i = 0; i < canvas.w; i++) {
            // if there is an image to draw
            if (canvas.data) {
                uint8_t color = canvas.data[j * canvas.w + i];
                sprintf(buff, "[4%um", color);
                UART_Print_Esc(buff);
            }
            UART_Print_Char(CANVAS_CHAR);
        }
    }
}
