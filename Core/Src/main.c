#include "buttons.h"
#include "main.h"
#include "terminaluart.h"
#include <stdio.h>

volatile uint8_t click_prelim;
volatile uint8_t click;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    Touchscreen_Init();
    UART_Init();
    Button_Init();
    __enable_irq();

    state_t  state  = TITLE;
    status_t status = { 0, 0, 0 };
    uint8_t  move_cursor_allowed = 1;
    UART_Update_Screen(state, 0);

    while (1)
    {
        if (click) On_Click(&state, status);
        Touchscreen_Read(&status);
        if (move_cursor_allowed) Move_Cursor(status);
    }
}

void Move_Cursor(status_t status) {
    if (status.update != 1) return;
    char buff[BUFF_LEN];
    sprintf(buff, "[%u;%uH", status.screen_y, status.screen_x);
    UART_Print_Esc(buff);
}

void On_Click(state_t *state, status_t status) {
    static uint8_t size = 0;
    static uint8_t color = 1;
    uint8_t state_update = 0;

    // get location
    uint16_t x = status.screen_x;
    uint16_t y = status.screen_y;

    // update canvas size
    button_t canvas = {
        (TERMINAL_WIDTH  >> 1) - (size << 1),
        (TERMINAL_HEIGHT >> 1) - (size >> 1),
        size << 2,
        size
    };

    // update color
    char buff[BUFF_LEN];
    sprintf(buff, "[4%um", color);
    UART_Print_Esc(buff);

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
                size = SIZE_SMALL;
                *state = CANVAS;
                state_update = 1;
                break;
            }
            if (On_Btn(x, y, BTN_SIZING_MEDIUM)) {
                size = SIZE_MEDIUM;
                *state = CANVAS;
                state_update = 1;
                break;
            }
            if (On_Btn(x, y, BTN_SIZING_LARGE)) {
                size = SIZE_LARGE;
                *state = CANVAS;
                state_update = 1;
                break;
            }
            if (On_Btn(x, y, BTN_SIZING_BACK)) {
                *state = TITLE;
                state_update = 1;
                break;
            }
            break;
        case CANVAS:
            if (On_Btn(x, y, canvas)) {
                // draw to canvas
                UART_Print_Char(BLANK_CHAR);
                UART_Print_Esc("[1D");
                break;
            }
            if (On_Btn(x, y, BTN_CANVAS_RED)) {
                color = 1;
                break;
            }
            if (On_Btn(x, y, BTN_CANVAS_GREEN)) {
                color = 2;
                break;
            }
            if (On_Btn(x, y, BTN_CANVAS_BLUE)) {
                color = 4;
                break;
            }
            if (On_Btn(x, y, BTN_CANVAS_WHITE)) {
                color = 7;
                break;
            }
            if (On_Btn(x, y, BTN_CANVAS_BLACK)) {
                color = 0;
                break;
            }
            if (On_Btn(x, y, BTN_CANVAS_DONE)) {
                *state = SAVE;
                state_update = 1;
                break;
            }
        default:
            break;
    }

    // update screen if there was a press
    if (state_update) UART_Update_Screen(*state, size);
}

void USART2_IRQHandler() {
    // check update interrupt flag
    if (USART2->ISR & USART_ISR_RXNE) {
        switch (USART2->RDR) {
            default:
                break;
        }
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
