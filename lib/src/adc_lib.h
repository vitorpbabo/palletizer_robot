#include <avr/io.h>
#include <avr/interrupt.h>

void init_adc (void);
unsigned int read_adc (unsigned char chan);