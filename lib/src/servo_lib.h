
#include <avr/io.h>
#include <avr/interrupt.h>

void servoStart(void);
ISR(TIMER1_COMPA_vect);

void servoSet(uint8_t servo, uint16_t time /* microseconds */);
