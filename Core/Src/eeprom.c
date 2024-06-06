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

void EEPROM_Write_Byte(uint8_t write_data, uint16_t address) {
    // write mode
    I2C2->CR2 &= ~I2C_CR2_RD_WRN;
    // 3 byte transfer
    I2C2->CR2 &= ~I2C_CR2_NBYTES;
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

    // delay to allow write cycle
    for (uint16_t k = 0; k < WRITE_CYCLE_DELAY; k++);
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
uint8_t EEPROM_Write_Image(header_t header, uint8_t *image) {
    // find block
    uint8_t block = EEPROM_Find_Free_Block();
    if (block == BLOCK_NOT_FOUND) {
        return 1;
    }

    // convert header to array
    uint8_t header_size_used = 3 + NAME_LEN_MAX;
    uint8_t *header_arr = (uint8_t*)malloc(sizeof(uint8_t) * header_size_used);
    header_arr[0] = header.block_used;
    header_arr[1] = header.size_x;
    header_arr[2] = header.size_y;
    for (uint8_t i = 0; i < NAME_LEN_MAX; i++) {
        header_arr[3 + i] = header.name[i];
    }

    // write header
    uint16_t address = block * MEM_BLOCK_SIZE;
    uint8_t num_bytes = header_size_used + 2; // add two for address
    I2C2->CR2 &= ~(I2C_CR2_RD_WRN);     // write
    I2C2->CR2 &= ~(I2C_CR2_NBYTES);     // set number of bytes to write
    I2C2->CR2 |= (num_bytes << I2C_CR2_NBYTES_Pos);
    // start
    I2C2->CR2 |= (1 << I2C_CR2_START_Pos);
    // write address
    while(!(I2C2->ISR & I2C_ISR_TXIS));
    I2C2->TXDR = address >> 8;
    while(!(I2C2->ISR & I2C_ISR_TXIS));
    I2C2->TXDR = address & 0xFF;
    // write header
    for (uint16_t i = 0; i < header_size_used; i++) {
        // await flag, then write
        while(!(I2C2->ISR & I2C_ISR_TXIS));
        I2C2->TXDR = header_arr[i];
    }
    free(header_arr);

    // delay to allow write cycle
    for (uint16_t k = 0; k < WRITE_CYCLE_DELAY; k++);

    // write pages (write address at start of each page)
    // go to data address
    address += MEM_HEADER_SIZE;
    num_bytes = MEM_PAGE_SIZE + 2; // add two for address
    uint8_t page_cnt = (header.size_x * header.size_y) / MEM_PAGE_SIZE;
    // set number of bytes
    I2C2->CR2 &= ~(I2C_CR2_NBYTES);
    I2C2->CR2 |= (num_bytes << I2C_CR2_NBYTES_Pos);

    // iterate over number of pages we will write
    for (uint8_t i = 0; i < page_cnt; i++) {
        // write address
        I2C2->CR2 |= (1 << I2C_CR2_START_Pos);
        // upper
        while(!(I2C2->ISR & I2C_ISR_TXIS));
        I2C2->TXDR = address >> 8;
        // lower
        while(!(I2C2->ISR & I2C_ISR_TXIS));
        I2C2->TXDR = address & 0xFF;
        // write data
        for (uint8_t j = 0; j < MEM_PAGE_SIZE; j++) {
            // image
            while(!(I2C2->ISR & I2C_ISR_TXIS));
            I2C2->TXDR = image[i * MEM_PAGE_SIZE + j];
        }
        // increment address
        address += MEM_PAGE_SIZE;
        // delay to allow write cycle
        for (uint16_t k = 0; k < WRITE_CYCLE_DELAY; k++);
    }
    free(image);
    return 0;
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

/* clears block_used flag in every block. Only use to clear ROM*/
void EEPROM_Clear() {
    for (uint8_t block = 0; block < MEM_BLOCK_CNT; block++) {
        uint16_t address = block * MEM_BLOCK_SIZE;
        EEPROM_Write_Byte(0x0, address);
    }
}

void EEPROM_Read_Header(header_t *header, uint8_t block_index) {
    header->block_used = EEPROM_Read_Byte(block_index * MEM_BLOCK_SIZE);
    if (!header->block_used) return;

    // sequential read continues at next address
    // still in read mode
    uint8_t num_bytes = 2 + NAME_LEN_MAX;
    I2C2->CR2 &= ~(I2C_CR2_NBYTES);
    I2C2->CR2 |= (num_bytes << I2C_CR2_NBYTES_Pos);
    // start
    I2C2->CR2 |= (1 << I2C_CR2_START_Pos);

    // read sizes
    while(!(I2C2->ISR & I2C_ISR_RXNE));
    header->size_x = I2C2->RXDR;
    while(!(I2C2->ISR & I2C_ISR_RXNE));
    header->size_y = I2C2->RXDR;

    // read name
    for (uint8_t i = 0; i < NAME_LEN_MAX; i++) {
        while(!(I2C2->ISR & I2C_ISR_RXNE));
        header->name[i] = I2C2->RXDR;
    }
}
