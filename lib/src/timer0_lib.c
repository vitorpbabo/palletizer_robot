#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t tis_T1=0, tis_T2=0;
volatile uint8_t T1_q=0, T2_q=0;

#define TIME0BASE 4 // em ms
#define T0TOP 250

// TIMER 0 para controlo de estados

void timer_init(void) {
  TCCR0B = 0;             // Stop TC2
  TIFR0 |= (7<<TOV0);     // Clear pending intr
  TCCR0A = 2;             // Mode CTC
  TCNT0 = 0;              // Load BOTTOM value
  OCR0A = T0TOP;          // Load TOP value
  TIMSK0 = (1<<OCIE0A);   // Enable COMPA intr
  TCCR0B = 4;             // Start TC2 (TP=256)
}

ISR(TIMER0_COMPA_vect) {
  if (tis_T1){
    tis_T1--;
    T1_q = 0;
  } 
  else T1_q = 1;
  
  if (tis_T2){
    tis_T2--;
    T2_q = 0;
  } 
  else T2_q = 1;
}

uint16_t safe_read_tis(uint8_t Timer){      //Lê o valor de tis em segurança parando a rotina de interrupção

  uint16_t tmp;

  switch (Timer)
  {
    case 1:{
      cli();
      tmp = tis_T1;
      sei();
    }break;

     case 2:{
      cli();
      tmp = tis_T2;
      sei();
    }break;

  default:
    return 0;
  break;
  }
  return tmp;
}

void start_T(uint8_t Timer, uint16_t T_ms){         //  4 timers predefinidos (1,2,3,4) que assumem valor T_ms)

  switch (Timer)
  {
    case 1:{
      T1_q = 0;
      tis_T1 = T_ms/TIME0BASE;                       // define o numero de ticks para a timebase 
    }break;

     case 2:{
      T2_q = 0;
      tis_T2 = T_ms/TIME0BASE;  
    }break;

  default:
  break;
  }
}

uint8_t end_T(uint8_t Timer){                       //  Sinaliza fim do timer

  switch (Timer)
  {
    case 1:{
      if (T1_q) return 1;  
    }break;

    case 2:{
      if (T2_q) return 1;       
    }break;

  default:
  break;
  }
  return 0;
}

uint16_t get_T(uint8_t Timer, uint16_t Tini_ms){    //  Deveolve valor atual do timer
  
   switch (Timer)
  {
    case 1:{
      return Tini_ms - safe_read_tis(Timer) * TIME0BASE;     
    }break;

    case 2:{      
      return Tini_ms - safe_read_tis(Timer) * TIME0BASE;       
    }break;
    
  default:
  break;
  }
  return 0;
}