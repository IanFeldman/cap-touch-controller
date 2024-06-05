#include "eeprom.h"
#include "stm32l4xx_hal.h"
#include <stdlib.h>

/* Initialize eeprom device (PB10, PB11) */
void EEPROM_Init() {
    // enable GPIO clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    // alternate function mode
    GPIOB->MODER &= ~(GPIO_MODER_MODE10 | GPIO_MODER_MODE11);
    GPIOB->MODER |= (GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1);
    // open-drain
    GPIOB->OTYPER |= (GPIO_OTYPER_OT10 | GPIO_OTYPER_OT11);
    // high speed
    GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED10 | GPIO_OSPEEDR_OSPEED11);
    // pull up resistors
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11);
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPD10_0 | GPIO_PUPDR_PUPD11_0);
    // set alternate function
    GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL10 | GPIO_AFRH_AFSEL11);
    GPIOB->AFR[1] |= (4 << GPIO_AFRH_AFSEL10_Pos) | (4 << GPIO_AFRH_AFSEL11_Pos);
    // enable I2C clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C2EN;
    // disable before start
    I2C2->CR1 &= ~I2C_CR1_PE;
    // configure I2C2 timing
    I2C2->TIMINGR = EEPROM_TIMING_FACTOR;
    // set up filters
    I2C2->CR1 &= ~I2C_CR1_ANFOFF;
    I2C2->CR1 &= ~I2C_CR1_DNF_Msk;
    // no stretch
    I2C2->CR1 &= ~I2C_CR1_NOSTRETCH;
    // re-enable
    I2C2->CR1 |= I2C_CR1_PE;

    // auto end
    I2C2->CR2 |= I2C_CR2_AUTOEND;
    // 7 bit address mode
    I2C2->CR2 &= ~(I2C_CR2_ADD10);
    //set the address
    I2C2->CR2 &= ~(I2C_CR2_SADD);
    I2C2->CR2 |= (EEPROM_ADDRESS << 1);
}

void EEPROM_Write(uint8_t write_data, uint16_t address) {
    // auto end
    I2C2->CR2 |= I2C_CR2_AUTOEND;
    // 7 bit address mode
    I2C2->CR2 &= ~(I2C_CR2_ADD10);
    //set the address
    I2C2->CR2 &= ~(I2C_CR2_SADD);
    I2C2->CR2 |= (EEPROM_ADDRESS << 1);
    // set to write
    I2C2->CR2 &= ~(I2C_CR2_RD_WRN);
    // 3 byte transfer
    I2C2->CR2 &= ~(I2C_CR2_NBYTES);
    I2C2->CR2 |= (3 << I2C_CR2_NBYTES_Pos);
    // start
    I2C2->CR2 |= (1 << I2C_CR2_START_Pos);

    // wait for flag
    while(!(I2C2->ISR & I2C_ISR_TXIS));
    I2C2->TXDR = address >> 8;
    while(!(I2C2->ISR & I2C_ISR_TXIS));
    I2C2->TXDR = address & 0xff;
    while(!(I2C2->ISR & I2C_ISR_TXIS));
    I2C2->TXDR = write_data;
}

uint8_t EEPROM_Read_Byte(uint16_t address) {
    // write mode
    I2C2->CR2 &= ~(I2C_CR2_RD_WRN);
    // write 2 bytes (register address)
    I2C2->CR2 &= ~(I2C_CR2_NBYTES);
    I2C2->CR2 |= (2 << I2C_CR2_NBYTES_Pos);
    // start
    I2C2->CR2 |= (1 << I2C_CR2_START_Pos);

    while(!(I2C2->ISR & I2C_ISR_TXIS));
    I2C2->TXDR = address >> 8;
    while(!(I2C2->ISR & I2C_ISR_TXIS));
    I2C2->TXDR = address & 0xff;

    // read mode
    I2C2->CR2 |= (I2C_CR2_RD_WRN);
    // read 1 byte (data)
    I2C2->CR2 &= ~(I2C_CR2_NBYTES);
    I2C2->CR2 |= (1 << I2C_CR2_NBYTES_Pos);
    // start
    I2C2->CR2 |= (1 << I2C_CR2_START_Pos);

    // wait for flag
    while(!(I2C2->ISR & I2C_ISR_RXNE));
    return I2C2->RXDR;
}

/* Writes image to block in EEPROM
 * return 0 on success, 1 on failure
 */
uint8_t EEPROM_Write_Image(uint8_t name[NAME_LEN_MAX], uint8_t *image, uint8_t size_x, uint8_t size_y) {
    // find block
    uint8_t block = EEPROM_Find_Free_Block();
    if (block == BLOCK_NOT_FOUND) {
        return 0;
    }

    /* HEADER
     * block_used
     * x size
     * y size
     * name[]
     * reserved[]
     */
    uint8_t header_size_used = 3 + NAME_LEN_MAX;
    uint8_t *header = (uint8_t*)malloc(sizeof(uint8_t) * header_size_used);
    header[0] = 1;
    header[1] = size_x;
    header[2] = size_y;
    for (uint8_t i = 0; i < NAME_LEN_MAX; i++) {
        header[3 + i] = name[i];
    }

    // write header
    uint16_t address = block * MEM_BLOCK_SIZE;
    I2C2->CR2 &= ~(I2C_CR2_RD_WRN);     // write
    I2C2->CR2 &= ~(I2C_CR2_NBYTES);     // set number of bytes to write
    I2C2->CR2 |= (header_size_used << I2C_CR2_NBYTES_Pos);
    // start
    I2C2->CR2 |= (1 << I2C_CR2_START_Pos);
    // TODO

    free(header);
    free(image);
}

/* return block idx on success, BLOCK_NOT_FOUND on failure */
uint8_t EEPROM_Find_Free_Block() {
    uint8_t block;
    uint8_t block_found = 0;
    for (block = 0; block < MEM_BLOCK_CNT; block++) {
        uint8_t block_used = EEPROM_Read_Byte(block * MEM_BLOCK_SIZE);
        if (!block_used) {
            block_found = 1;
            break;
        }
    }
    // failure
    if (!block_found) {
        return BLOCK_NOT_FOUND;
    }
    return block;
}
