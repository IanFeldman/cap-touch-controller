#ifndef INC_TERMINALUART_H_
#define INC_TERMINALUART_H_

#include <stdint.h>

#define ESC_CHAR 0x1b

void UART_Init();
void UART_Reset_Screen();
void UART_Print_Char(char data);
void UART_Print_Esc(char *string);
void UART_Print(char *string);

#endif /* INC_TERMINALUART_H_ */
