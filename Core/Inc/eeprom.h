#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#include <stdint.h>

#define EEPROM_TIMING_FACTOR 0x7
#define EEPROM_ADDRESS 0x50
#define WRITE_CYCLE_DELAY 20000

#define NAME_LEN_MAX 32

// break memory into 16 blocks of 4096 (128x32)
#define MEM_BLOCK_CNT    16
#define MEM_BLOCK_SIZE 4096
#define MEM_HEADER_SIZE 256
#define MEM_PAGE_SIZE   128

#define BLOCK_NOT_FOUND 0xFF

typedef struct {
    uint8_t block_used;
    uint8_t size_x;
    uint8_t size_y;
    char name[NAME_LEN_MAX];
} header_t;

void EEPROM_Init();
void EEPROM_Write_Byte(uint8_t write_data, uint16_t address);
uint8_t EEPROM_Read_Byte(uint16_t address);
uint8_t EEPROM_Write_Image(header_t *header, uint8_t *image);
uint8_t EEPROM_Find_Free_Block();
void EEPROM_Clear();
void EEPROM_Read_Header(header_t *header, uint8_t block_index);
uint8_t *EEPROM_Read_Image(header_t *header, uint8_t block_index);

#endif
