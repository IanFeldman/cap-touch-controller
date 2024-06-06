#ifndef INC_TERMINALUART_H_
#define INC_TERMINALUART_H_

#define ESC_CHAR 0x1b
#define CANVAS_CHAR ' '
#define PIXEL_CHAR  ' '

void UART_Init();
void UART_Reset_Screen();
void UART_Print_Char(char data);
void UART_Print_Esc(char *string);
void UART_Print(char *string);

void UART_Update_Screen(state_t state, button_t *canvas);
void UART_Print_Btn(button_t btn, uint8_t is_solid);

#endif /* INC_TERMINALUART_H_ */
