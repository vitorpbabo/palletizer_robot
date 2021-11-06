//EEPROM Read/Write 16bit/8bit

#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t EEPROM_read_low(uint8_t adrs){
    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE));

    /* Set up address register */
    EEAR = adrs;

    /* Start eeprom read by writing EERE */
    EECR |= (1<<EERE);

    /* Return lowbyte from Data Register */
    return EEDR;
}

uint8_t EEPROM_read_High(uint8_t adrs){
    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE));

    /* Set up address register */
    EEAR = adrs + 1;

    /* Start eeprom read by writing EERE */
    EECR |= (1<<EERE);

    /* Return highbyte from Data Register */
    return EEDR;
}

uint16_t EEPROM_read_16bit(uint8_t adrs){
    
    uint8_t l = EEPROM_read_low(adrs);
    uint8_t h = EEPROM_read_High(adrs);

    uint16_t H = (h<<8);

    return H | l;
}


void EEPROM_write_low(uint8_t adrs, uint8_t input){
   
    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE));

    /* Set up address and Data Registers */
    EEAR = adrs;
    EEDR = input;

    /* Write logical one to EEMPE */
    EECR |= (1<<EEMPE);

    /* Start eeprom write by setting EEPE */
    EECR |= (1<<EEPE);

}

void EEPROM_write_high(uint8_t adrs, uint8_t input){

    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE));

    /* Set up address and Data Registers */
    EEAR = adrs + 1;
    EEDR = input;

    /* Write logical one to EEMPE */
    EECR |= (1<<EEMPE);

    /* Start eeprom write by setting EEPE */
    EECR |= (1<<EEPE);
}

void EEPROM_write_16bit(uint8_t adrs, uint16_t input){
    
    uint8_t lowByte = (input & 0xFF); 
    EEPROM_write_low(adrs, lowByte);
    uint8_t highByte = (input >> 8);
    EEPROM_write_high(adrs, highByte);
}