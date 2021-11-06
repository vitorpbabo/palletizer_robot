#include <avr/io.h>
#include <avr/interrupt.h>

void timer_init(void);
ISR(TIMER0_COMPA_vect);

uint16_t safe_read_tis(uint8_t Timer);
void start_T(uint8_t Timer /* 1 ou 2*/, uint16_t T_ms);
uint8_t end_T(uint8_t Timer /* 1 ou 2*/);
uint16_t get_T(uint8_t Timer /* 1 ou 2*/, uint16_t Tini_ms);
