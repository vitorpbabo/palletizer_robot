//MEDIÇÃO COR C/ TIMER2

#include <avr/io.h>
#include <avr/interrupt.h>

void colorsensor_init ();
ISR(TIMER2_OVF_vect);

uint32_t FreqMeas();
char get_color ();

