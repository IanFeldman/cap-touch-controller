#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#include <stdint.h>

#define EEPROM_TIMING_FACTOR 0x7
#define EEPROM_ADDRESS 0x50

#define NAME_LEN_MAX 32

void EEPROM_Init();
void EEPROM_Write(uint8_t write_data, uint16_t address);
uint8_t EEPROM_Read(uint16_t address);

uint8_t EEPROM_Write_Image(uint8_t name[NAME_LEN_MAX], uint8_t *image, uint8_t size_y);

#endif
