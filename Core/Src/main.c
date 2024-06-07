#include "eeprom.h"
#include "main.h"
#include "terminaluart.h"
#include <stdio.h>
#include <stdlib.h>

// set by EXTI ISRs
volatile uint8_t click_prelim;
volatile uint8_t click;
// set by UART ISR
volatile uint8_t char_input;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    Touchscreen_Init();
    UART_Init();
    Button_Init();
    EEPROM_Init();
    __enable_irq();

    state_t  state  = TITLE;
    status_t status = { 0, 0, 0 };
    uint8_t  cursor_allowed = 1;
    button_t canvas = { 0, 0, 0, 0, 0 };
    UART_Update_Screen(state, canvas);

    while (1)
    {
        // poll for click
        if (click) {
            On_Click(&state, &canvas, status, &cursor_allowed);
        }
        // poll for key
        if (!cursor_allowed && char_input) {
            On_Press(&state, &canvas, &cursor_allowed);
        }
        // move cursor
        if (cursor_allowed) {
            Touchscreen_Read(&status);
            Move_Cursor(status);
        }
    }
}

void Move_Cursor(status_t status) {
    if (status.update != 1) return;
    char buff[BUFF_LEN];
    sprintf(buff, "[%u;%uH", status.term_y, status.term_x);
    UART_Print_Esc(buff);
}

void On_Click(state_t *state, button_t *canvas, status_t status, uint8_t *cursor_allowed) {
    static uint8_t color = 1;
    uint8_t state_update = 0;
    uint8_t allocate_mem = 0;

    // get location
    uint16_t x = status.term_x;
    uint16_t y = status.term_y;

    switch (*state) {
        case TITLE:
            if (On_Btn(x, y, BTN_TITLE_NEW)) {
                *state = SIZING;
                state_update = 1;
                break;
            }
            if (On_Btn(x, y, BTN_TITLE_OPEN)) {
                *state = BROWSER;
                state_update = 1;
                break;
            }
            break;

        case SIZING:
            if (On_Btn(x, y, BTN_SIZING_SMALL)) {
                canvas->w = SIZE_SMALL_X;
                canvas->h = SIZE_SMALL_Y;
                *state = CANVAS;
                state_update = 1;
            }
            else if (On_Btn(x, y, BTN_SIZING_MEDIUM)) {
                canvas->w = SIZE_MEDIUM_X;
                canvas->h = SIZE_MEDIUM_Y;
                *state = CANVAS;
                state_update = 1;
            }
            else if (On_Btn(x, y, BTN_SIZING_LARGE)) {
                canvas->w = SIZE_LARGE_X;
                canvas->h = SIZE_LARGE_Y;
                *state = CANVAS;
                state_update = 1;
            }
            else if (On_Btn(x, y, BTN_SIZING_BACK)) {
                *state = TITLE;
                state_update = 1;
                break;
            }
            if (state_update) {
                Get_Canvas_XY(canvas);
                allocate_mem = 1; // set flag to malloc AFTER update screen
            }
            break;

        case CANVAS:
            // update color
            char buff[BUFF_LEN];
            sprintf(buff, "[4%um", color);
            UART_Print_Esc(buff);

            if (On_Btn(x, y, *canvas)) {
                // draw to canvas
                UART_Print_Char(PIXEL_CHAR);
                UART_Print_Esc("[1D");
                // get image location
                uint16_t image_x = x - canvas->x;
                uint16_t image_y = y - canvas->y;
                // write to ram
                canvas->data[image_y * canvas->w + image_x] = color;
                break;
            }
            if (On_Btn(x, y, BTN_CANVAS_RED)) {
                color = COLOR_RED;
                break;
            }
            if (On_Btn(x, y, BTN_CANVAS_GREEN)) {
                color = COLOR_GREEN;
                break;
            }
            if (On_Btn(x, y, BTN_CANVAS_BLUE)) {
                color = COLOR_BLUE;
                break;
            }
            if (On_Btn(x, y, BTN_CANVAS_WHITE)) {
                color = COLOR_WHITE;
                break;
            }
            if (On_Btn(x, y, BTN_CANVAS_BLACK)) {
                color = COLOR_BLACK;
                break;
            }
            if (On_Btn(x, y, BTN_CANVAS_DONE)) {
                *state = SAVE;
                *cursor_allowed = 0;
                state_update = 1;
                break;
            }

        case SAVE:
            // no click operations
            break;

        case BROWSER:
            // file buttons
            button_t entry = { 0, 0, 0, 0};
            entry.w = BTN_BROWSER_ENTRY.w;
            entry.h = BTN_BROWSER_ENTRY.h;

            // determine if file selected
            uint8_t file_selected = 0;
            uint8_t i;
            for (i = 0; i < MEM_BLOCK_CNT; i++) {
                // determine x
                if (i >= (MEM_BLOCK_CNT / 2) - 1) {
                    entry.x = BTN_BROWSER_ENTRY_COL2_X;
                }
                // determine y
                entry.y = BTN_BROWSER_ENTRY_INIT_Y + i * BTN_BROWSER_ENTRY_SPACING_Y;

                // check for selection
                if (On_Btn(x, y, entry)) {
                    file_selected = 1;
                    break;
                }
            }
            if (file_selected) {
                // read header
                header_t header;
                EEPROM_Read_Header(&header, i);
                // save to canvas
                canvas->w = header.size_x;
                canvas->h = header.size_y;
                Get_Canvas_XY(canvas);

                // read file into image
                canvas->data = EEPROM_Read_Image(&header, i);
                // update
                *state = CANVAS;
                state_update = 1;
                break;
            }
            if (On_Btn(x, y, BTN_BROWSER_BACK)) {
                *state = TITLE;
                state_update = 1;
                break;
            }
            break;
        default:
            break;
    }

    // update screen if there was a press
    if (state_update) UART_Update_Screen(*state, *canvas);
    if (allocate_mem) {
        // allocate memory for canvas if we have not yet
        // clear it
        uint16_t size = canvas->w * canvas->h;
        canvas->data = (uint8_t*)malloc(sizeof(uint8_t) * size);
        for (uint16_t i = 0; i < size; i++) {
            canvas->data[i] = COLOR_WHITE;
        }
    }
}

void On_Press(state_t *state, button_t *canvas, uint8_t *cursor_allowed) {
    static uint8_t filename[NAME_LEN_MAX];
    static uint8_t filename_idx = 0;

    // clear attributes
    UART_Print_Esc("[0m");

    // exit
    if (char_input == CHAR_ESCAPE) {
        free(canvas->data);
        canvas->data = 0;
        // change state
        *state = TITLE;
        *cursor_allowed = 1;
        UART_Update_Screen(*state, *canvas);
    }
    // delete
    else if (char_input == CHAR_DELETE && filename_idx > 0) {
        // move cursor left
        UART_Print_Esc("[1D");
        // clear
        UART_Print_Char(' ');
        // move left
        UART_Print_Esc("[1D");
        filename_idx--;
    }
    // enter
    else if (char_input == CHAR_RETURN) {
        // add null term
        filename[filename_idx] = '\0';
        // write file
        header_t header;
        header.block_used = 1;
        header.size_x = canvas->w;
        header.size_y = canvas->h;
        for (uint8_t i = 0; i < NAME_LEN_MAX; i++) {
            header.name[i] = filename[i];
        }
        uint8_t err = EEPROM_Write_Image(&header, canvas->data);
        canvas->data = 0;
        if (err) {
            UART_Print("ERROR OCCURED");
            char_input = 0;
            return;
        }
        // change state
        *state = TITLE;
        *cursor_allowed = 1;
        UART_Update_Screen(*state, *canvas);
    }
    else if (char_input != CHAR_DELETE && filename_idx < NAME_LEN_MAX - 1) {
        filename[filename_idx++] = char_input;
        UART_Print_Char(char_input);
    }

    char_input = 0;
}

void Get_Canvas_XY(button_t *canvas) {
    // find x, y pos
    canvas->x = (TERMINAL_WIDTH  >> 1) - (canvas->w >> 1) + 1;
    canvas->y = (TERMINAL_HEIGHT >> 1) - (canvas->h >> 1);
}

void USART2_IRQHandler() {
    // check update interrupt flag
    if (USART2->ISR & USART_ISR_RXNE) {
        char_input = USART2->RDR;
        // flag cleared by reading register
    }
}

// rising
void EXTI0_IRQHandler() {
    // check flag
    if (EXTI->PR1 & EXTI_PR1_PIF0) {
        if (!click) {
            click_prelim = 1;
            // start debounce timer
            TIM2->CR1 = TIM_CR1_CEN;
        }
        // reset flag by setting
        EXTI->PR1 |= EXTI_PR1_PIF0;
    }
}

// falling
void EXTI1_IRQHandler() {
    // check flag
    if (EXTI->PR1 & EXTI_PR1_PIF1) {
        if (click) {
            click_prelim = 0;
            // start debounce timer
            TIM2->CR1 = TIM_CR1_CEN;
        }
        // reset flag by setting
        EXTI->PR1 |= EXTI_PR1_PIF0;
    }
}

void TIM2_IRQHandler() {
    // check update interrupt flag
    if (TIM2->SR & TIM_SR_UIF) {
        uint8_t curr_click = GPIOA->IDR & GPIO_IDR_ID0;
        // if it is still where it was
        if (curr_click == click_prelim) {
            click = click_prelim;
        }
        // clear flags
        TIM2->SR &= ~TIM_SR_UIF;
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
