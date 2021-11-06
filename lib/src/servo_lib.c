#include <avr/io.h>
#include <avr/interrupt.h>

#define SERVO_PORT  PORTB

// Num Servos
#define N_SERVOS    4

// Time between servo pulses.  
#define SERVO_FRAME 20000 // microseconds (50Hz)

// Time slot available for each servo.
#define SERVO_TIME_DIV (SERVO_FRAME / N_SERVOS)

// Timer ticks >>> microseconds.
#define US2TIMER1(us) ((us) * (uint16_t)(F_CPU /( (1E6) )))

// Servo times - (using US2TIMER1).
// Atualizar com interrupções desativadas.
volatile uint16_t servoTime[N_SERVOS];


const static uint8_t servoOutMask[N_SERVOS] = {  
    0b00000000, // PX0 - Não utilizado
    0b00000010, // PX1
    0b00000100, // PX2
    0b00001000, // PX3
};

//CONTROLO SERVOS

void servoStart(void)
{ 
  // Primeira comparação
  OCR1A = US2TIMER1(1800);
  // start timer 1 - no prescaler
  TCCR1B = (1 << CS10);
  // Enable interrupt
  TIMSK1 |= (1 << OCIE1A);
  
}

ISR(TIMER1_COMPA_vect) 
{ 
  static uint16_t nextStart;
  static uint8_t servo;
  static uint8_t outputHigh = 1;        
  uint16_t currentTime = OCR1A;
  uint8_t mask = servoOutMask[servo];
  
  if (outputHigh) {
    SERVO_PORT |= mask;
    // Define o fim do pulso do servo
    OCR1A = currentTime + servoTime[servo]; 
    nextStart = currentTime + US2TIMER1(SERVO_TIME_DIV);
  } 
  else {
    SERVO_PORT &= ~mask;
    if (++servo == N_SERVOS) {
        servo = 0;
    }
    OCR1A = nextStart; 
  }
  outputHigh = !outputHigh;
}

void servoSet(uint8_t servo, uint16_t time /* microsegundos */)
{
    uint16_t ticks = US2TIMER1(time); 
    cli();    // Para interrupções para leitura do valor em segurança
    servoTime[servo] = ticks;
    sei();    //  Ativa novamente
}