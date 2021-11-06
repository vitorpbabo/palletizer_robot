//MEDIÇÃO COR C/ TIMER2

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "serial_printf.h"


uint16_t volatile incrementador=0;

void colorsensor_init (){
  TCCR2A=0;
  TIMSK2=(1<<TOIE2);
}

ISR(TIMER2_OVF_vect) {
 TCNT2 = 0;                         // reload TC2
 incrementador++;
}

uint16_t FreqMeas(){

  if (PIND&(1 << 3)){         
      while(PIND&(1 << 3));       // Espera para o falling edge
  }

  TIFR2 = (7<<TOV2);

  while(!(PIND&(1 << 3)));        // Espera para o rising edge e inicio da contagem

      TCNT2=0;                    // Reset Counter
      incrementador=0;
      TCCR2B=(1<<CS20);           // Prescaller = F_CPU/1 (Start Counting)

  while(PIND&(1 << 3));           // Fim do pulso

      TCCR2B=0;                   // Stop Timer

  return (F_CPU/(2*(incrementador*255 + TCNT2)));
}


char get_color (){
  uint16_t red, blue, green;
  PORTD &= ~(1<<PD7); PORTD &= ~(1<<PD6); //(S3,S2) freq red
  red = abs(FreqMeas());

  PORTD |= (1<<PD7); PORTD &= ~(1<<PD6); //(S3,S2) freq blue
  blue = abs(FreqMeas());

  PORTD |= (1<<PD7); PORTD |= (1<<PD6); //(S3,S2) freq green
  green = abs(FreqMeas());

  printf("RED: %d, GREEN: %d, BLUE: %d \n", red, green, blue);

  if(red>blue && red>green && green<blue && red > 10000 ) return 'R';
  else if(blue>red && blue>green && red<green && blue > 10000 ) return 'B';
  else if(green>red && green>blue && green > 10000) return 'G';
  else return 'D';
}