//EEPROM Read/Write 16bit/8bit

#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t EEPROM_read_low(uint8_t adrs);
uint8_t EEPROM_read_High(uint8_t adrs);
uint16_t EEPROM_read_16bit(uint8_t adrs);

void EEPROM_write_low(uint8_t adrs, uint8_t input);
void EEPROM_write_high(uint8_t adrs, uint8_t input);
void EEPROM_write_16bit(uint8_t adrs, uint16_t input);