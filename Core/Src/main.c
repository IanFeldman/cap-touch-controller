#include "main.h"
#include "terminaluart.h"
#include <stdio.h>

#define TIMING_FACTOR 0xD15
#define DEVICE_ADDRESS 0x38

#define CANVAS_HEIGHT 50
#define CANVAS_WIDTH  200

#define POS_V_MAX 240
#define POS_H_MAX 320

typedef struct {
    uint8_t  update;
    uint8_t  pos_v;
    uint16_t pos_h;
} status_t;

void SystemClock_Config(void);
void Touchscreen_Init();
void Touchscreen_Read(status_t *status);
void Move_Cursor(status_t *status);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    Touchscreen_Init();
    UART_Init();
    UART_Reset_Screen();

    status_t status = { 0, 0, 0 };
    while (1)
    {
        Touchscreen_Read(&status);
        Move_Cursor(&status);
    }
}

void Touchscreen_Init() {
    // PA6 : SCL
    // PA7 : SDA

    // enable GPIO clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    // alternate function mode
    GPIOB->MODER &= ~(GPIO_MODER_MODE6_Msk | GPIO_MODER_MODE7_Msk);
    GPIOB->MODER |= (GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);
    // open-drain
    GPIOB->OTYPER |= (GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7);
    // high speed
    GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED6_Msk | GPIO_OSPEEDR_OSPEED7_Msk);
    // pull up resistors
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD6_Msk | GPIO_PUPDR_PUPD7_Msk);
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPD6_0 | GPIO_PUPDR_PUPD7_0);
    // set alternate function
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6_Msk | GPIO_AFRL_AFSEL7_Msk);
    GPIOB->AFR[0] |= (4 << GPIO_AFRL_AFSEL6_Pos) | (4 << GPIO_AFRL_AFSEL7_Pos);
    // enable I2C clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
    // disable before start
    I2C1->CR1 &= ~I2C_CR1_PE;
    // configure I2C1 timing
    I2C1->TIMINGR = TIMING_FACTOR;
    // set up filters
    I2C1->CR1 &= ~I2C_CR1_ANFOFF;
    I2C1->CR1 &= ~I2C_CR1_DNF_Msk;
    // no stretch
    I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;
    // re-enable
    I2C1->CR1 |= I2C_CR1_PE;
}

void Touchscreen_Read(status_t *status) {
    // autoend
    I2C1->CR1 |= (I2C_CR2_AUTOEND);
    // 7 bit address
    I2C1->CR2 &= ~(I2C_CR2_ADD10);
    // set peripheral address
    I2C1->CR2 &= ~(I2C_CR2_SADD);
    I2C1->CR2 |= (DEVICE_ADDRESS << 1);
    // read
    I2C1->CR2 |= (I2C_CR2_RD_WRN);
    // read 9 bytes (register address)
    I2C1->CR2 &= ~(I2C_CR2_NBYTES);
    I2C1->CR2 |= (9 << I2C_CR2_NBYTES_Pos);
    // start
    I2C1->CR2 |= (1 << I2C_CR2_START_Pos);

    // wait for flag
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    uint8_t data0 = I2C1->RXDR;
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    uint8_t data1 = I2C1->RXDR;
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    uint8_t data2 = I2C1->RXDR;
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    uint8_t data3 = I2C1->RXDR;
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    uint8_t data4 = I2C1->RXDR;
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    uint8_t data5 = I2C1->RXDR;
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    uint8_t data6 = I2C1->RXDR;
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    uint8_t data7 = I2C1->RXDR;
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    uint8_t data8 = I2C1->RXDR;

    // update status
    status->update = data2;
    status->pos_v  = data4;
    status->pos_h  = (data5 << 8) + data6;
}

void Move_Cursor(status_t *status) {
    if (status->update != 1) return;

    uint16_t adj_y = CANVAS_HEIGHT - CANVAS_HEIGHT * status->pos_v / POS_V_MAX;
    uint16_t adj_x = CANVAS_WIDTH * status->pos_h / POS_H_MAX;

    uint8_t buff[20];
    sprintf(buff, "[%u;%uH", adj_y, adj_x);
    UART_Print_Esc(buff);
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
