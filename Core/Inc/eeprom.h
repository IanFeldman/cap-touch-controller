#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#include <stdint.h>

#define EEPROM_TIMING_FACTOR 0x7
#define EEPROM_ADDRESS 0x50

#define NAME_LEN_MAX 32

// break memory into 16 blocks of 4096 (128x32)
#define MEM_BLOCK_CNT    16
#define MEM_BLOCK_SIZE 4096
#define MEM_HEADER_SIZE 256

#define BLOCK_NOT_FOUND 0xFF

/* HEADER
 * 1   byte: block_used
 * 1   byte: x size
 * 1   byte: y size
 * 32  byte: name
 * 221 byte: reserved
 */


void EEPROM_Init();
void EEPROM_Write(uint8_t write_data, uint16_t address);

uint8_t EEPROM_Read_Byte(uint16_t address);
uint8_t EEPROM_Write_Image(uint8_t name[NAME_LEN_MAX], uint8_t *image, uint8_t size_x, uint8_t size_y);
uint8_t EEPROM_Find_Free_Block();

#endif
