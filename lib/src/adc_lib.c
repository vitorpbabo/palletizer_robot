#include <avr/io.h>
#include <avr/interrupt.h>

void init_adc (void){
  // Definir Vref=AVcc
  ADMUX = ADMUX | (1<<REFS0);
  // Desativar buffer digital em PC0
  DIDR0 = DIDR0 | (1<<PC0) | (1<<PC1) | (1<<PC2);
  // Pré-divisor em 128 e ativar ADC
  ADCSRA = ADCSRA | (7<<ADPS0)|(1<<ADEN);
}

unsigned int read_adc (unsigned char chan){
  // escolher o canal...
  ADMUX = (ADMUX & 0xF0) | (chan & 0x0F);
  // iniciar a conversão
  // em modo manual (ADATE=0)
  ADCSRA |= (1<<ADSC);
  // esperar pelo fim da conversão
  while(ADCSRA & (1<<ADSC));

  return ADC;
}