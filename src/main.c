#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "serial_printf.h"
#include "timer0_lib.h"
#include "servo_lib.h"
#include "color_sensor.h"
#include "adc_lib.h"

#define VREF        5
#define SERVO_DDR   DDRB

#define SIG08 0b01010101

#define SIG_ADRS    0 
#define RED_ADRS    1 
#define GREEN_ADRS  2 
#define BLUE_ADRS   3 
#define UNK_ADRS    4

//Servos
void servoStart(void);
void servoSet(uint8_t servo, uint16_t time);

//Timer
void timer_init(void);
void start_T(uint8_t Timer, uint16_t T_ms);
uint8_t end_T(uint8_t Timer);

//Color Sensor
void colorsensor_init();
char get_color ();

// ADC
void init_adc(void);
unsigned int read_adc(unsigned char chan);

//EEPROM
uint8_t EEPROM_read_low(uint8_t adrs);
void EEPROM_write_low(uint8_t adrs, uint8_t input);

//MAIN FUNCTIONS
void io_init(void);
uint8_t B1_is_set (void);
uint8_t B2_is_set (void);
uint8_t RE_B1(void);
uint8_t RE_B2(void);

void set_GRABER (uint8_t on);
uint8_t setServo3();
uint8_t go_neutral();
uint8_t go_grab (uint16_t s1_pos, uint16_t s2_pos, uint16_t s3_pos);
uint8_t piece(uint16_t S1, uint16_t S2, uint16_t S3);
uint8_t go_box(char color);
char go_sensor ();
void print_report();

uint8_t s2 = 0;
uint8_t s2_prev = 0;
uint8_t s1 = 0;
uint8_t s1_prev = 0;
uint8_t RED_store=0, BLUE_store=0, GREEN_store=0, UNK_store=0;
char color_read;

void io_init (void){
  
  // SERVO Outputs
  SERVO_DDR |= (1<<1) | (1<<2) | (1<<3);

  DDRB &= ~(1<<PB5); // B1 input
  DDRB &= ~(1<<PB4); // B2 input
  DDRB |= (1<<PB0);  // IMAN OUTPUT

  //SENSOR
  DDRD |= (1<<PD4) | (1<<PD5);  // S0 e S1
  DDRD |= (1<<PD6) | (1<<PD7);  // S2 e S3
  DDRD  &= ~(1<<PD3);           // Output

  PORTD |=  (1<<PD4);           // Output freq scale 20%
  PORTD &= ~(1<<PD5); 
}
uint8_t B1_is_set (void){
  if (!(PINB&(1<<PB5)))
      return 1;
  else 
      return 0;
}
uint8_t B2_is_set (void){
  if (!(PINB&(1<<PB4)))
      return 1;
  else 
      return 0;
}
uint8_t RE_B1 (void){

  s1_prev = s1;
  s1 = B1_is_set();

  if (s1 && !s1_prev)
    return 1;
    
  else 
    return 0;
}
uint8_t RE_B2 (void){

  s2_prev = s2;
  s2 = B2_is_set();

  if (s2 && !s2_prev) 
    return 1;

  else 
    return 0;
}

void set_GRABER (uint8_t on){

  if (on) PORTB |= (1<<PB0);
  else PORTB &= ~(1<<PB0);
}

uint8_t setServo3 (uint16_t pos){   // retorna 1 enquanto servo 3 estiver ativo

  uint16_t actual_pos = read_adc(2);
  
  if ((abs(pos - actual_pos)) <= 2) {
    servoSet(3, 1500); //parar
    return 0;
  }

  else if( (abs(pos - actual_pos)) < 20 ){
    if(pos>actual_pos) servoSet(3, 1730);
    else if(pos<actual_pos) servoSet(3, 1270);
  }

  else 
    servoSet(3, 1500 + (pos - actual_pos) * 7 );

  return 1;
} 

uint8_t go_neutral (){   //retorna 1 enquanto movimentaçoes ocorrem

  static uint8_t state_f_neutral = 0;
  
  if(state_f_neutral == 0){
    servoSet(1, 1800);
    state_f_neutral = 1;
    start_T(1, 100);
  } 
  
  if(state_f_neutral == 1 && end_T(1)){
    servoSet(2, 1800);
    state_f_neutral = 2;
    start_T(1, 500);
  }
  else if(state_f_neutral == 2 && end_T(1)){
    while (setServo3(500));   //espera servo 3 na posição certa
    state_f_neutral = 0;
    start_T(1, 500);
    while(!end_T(1));
    return 0;
  }
 
  return 1;
}

char go_sensor (){    // vai ao sensor e retorna a cor recebida;

  static uint8_t state_s = 0;
  
  if (state_s == 0 ){
    color_read = 0;
    servoSet(2, 1800);
    state_s = 1;
    start_T(1,20);
  }
  else if (state_s == 1 && end_T(1)){
    servoSet(1, 1750);
    start_T(1,500);
    while(!end_T(1));
    while(setServo3(322));
    state_s = 2;
    start_T(1,200); 
  }
  else if (state_s == 2 && end_T(1) ){ 
    servoSet(2, 1876);
    state_s = 3;
    start_T(1,500);
  }
  else if (state_s == 3 && end_T(1) ){ 
    servoSet(1, 1484);
    state_s = 4;
    start_T(1,500);
  }
  else if (state_s == 4 && end_T(1) ){
    color_read = get_color();
    if (color_read){
      state_s = 0;
      start_T(1, 500);
      while(!end_T(1));
      return color_read;
    }
  }
  return 0;
}

void print_report(){
  
  RED_store   = EEPROM_read_low(RED_ADRS);
  GREEN_store = EEPROM_read_low(GREEN_ADRS);
  BLUE_store  = EEPROM_read_low(BLUE_ADRS);
  UNK_store   = EEPROM_read_low(UNK_ADRS);

  printf("Peças Operadas\n RED: %d, GREEN: %d, BLUE: %d, UNK: %d \n", RED_store, GREEN_store, BLUE_store, UNK_store);
}

uint8_t go_grab (uint16_t s1_pos, uint16_t s2_pos, uint16_t s3_pos){ // agarra a peça na posição escolhida

  static uint8_t state_g = 0;

  if (state_g == 0 ){
    servoSet(1, 1800);
    start_T(1,200); 
    state_g = 1;
  }
  else if (state_g == 1 && end_T(1)){ 
    servoSet(2, 1800);
    start_T(1,500); 
    state_g = 2;
  }
  else if (state_g == 2 && end_T(1)){ 
    while(setServo3(s3_pos)); 
    start_T(1,200); 
    state_g = 3;
  }
  else if (state_g == 3 && end_T(1)){ 
    servoSet(2, s2_pos);
    start_T(1,500); 
    state_g = 4;
  }
  else if (state_g == 4 && end_T(1) ){
    servoSet(1, s1_pos);
    start_T(1,500); 
    state_g = 5;
  }
  else if (state_g == 5 && end_T(1) ){
    set_GRABER(1);
    start_T(1, 1000);
    while(!end_T(1));
    state_g = 0;
    return 0;
  }

  return 1;
}

uint8_t go_box(char color){   // vai á BOX da cor designada e larga a peça

  static uint8_t state_b = 0;

  switch (color)
  {
  case 'R':
    if (state_b == 0 && end_T(1)){
      EEPROM_write_low(RED_ADRS, ++RED_store);
      servoSet(1, 1717);
      state_b = 1;
      start_T(1,200);
    }
    else if (state_b == 1 && end_T(1)){
      servoSet(2, 1760);   
      state_b = 2;
      start_T(1,200);
    }
    else if (state_b == 2 && end_T(1) ){
      while(setServo3(570));  
      state_b = 3;
      start_T(1,500); 
    }
    else if (state_b == 3 && end_T(1) ){
      set_GRABER(0);
      state_b = 4;
      start_T(1,500); 
    }
    else if (state_b == 4 && end_T(1) ){ 
      state_b = 0;
      return 0;     
    }
    break;

    case 'G':
      if (state_b == 0 ){
        servoSet(1, 1807);
        state_b = 1;
        EEPROM_write_low(GREEN_ADRS, ++GREEN_store);
        start_T(1,200); 
      }
      else if (state_b == 1 && end_T(1)){
        servoSet(2, 2027);   
        state_b = 2;
        start_T(1,200);
      }
      else if (state_b == 2 && end_T(1) ){
        while(setServo3(410));  
        state_b = 3;
        start_T(1,500); 
      }
      else if (state_b == 3 && end_T(1) ){
        set_GRABER(0);
        state_b = 4;
        start_T(1,500); 
      }
      else if (state_b == 4 && end_T(1) ){ 
        state_b = 0;
        return 0;     
      }
      break;

    case 'B':
      if (state_b == 0 ){
        servoSet(1, 1700);
        state_b = 1;
        EEPROM_write_low(BLUE_ADRS, ++BLUE_store);
        start_T(1,200); 
      }
      else if (state_b == 1 && end_T(1)){
        servoSet(2, 1811);   
        state_b = 2;
        start_T(1,200);
      }
      else if (state_b == 2 && end_T(1) ){
        while(setServo3(392));  
        state_b = 3;
        start_T(1,500); 
      }
      else if (state_b == 3 && end_T(1) ){
        set_GRABER(0);
        state_b = 4;
        start_T(1,500); 
      }
      else if (state_b == 4 && end_T(1) ){ 
        state_b = 0;
        return 0;     
      }
      break;

    case 'D':
      if (state_b == 0 ){
        servoSet(1, 1760);
        state_b = 1;
        EEPROM_write_low(UNK_ADRS, ++UNK_store);
        start_T(1,200); 
      }
      else if (state_b == 1 && end_T(1)){
        servoSet(2, 1988);   
        state_b = 2;
        start_T(1,200);
      }
      else if (state_b == 2 && end_T(1) ){
        while(setServo3(550));  
        state_b = 3;
        start_T(1,500); 
      }
      else if (state_b == 3 && end_T(1) ){
        set_GRABER(0);
        state_b = 4;
        start_T(1,500); 
      }
      else if (state_b == 4 && end_T(1) ){ 
        state_b = 0;
        return 0;     
      }
      break;
  
    default:
      printf("ERRO LEITURA COR");
      while(go_neutral());
      set_GRABER(0);
      return 0;
    break;
  }

  return 1;
}

uint8_t piece (uint16_t S1, uint16_t S2, uint16_t S3){    //Rotina completa para uma dada peça
  
  static uint8_t state=1;

  if(state == 1){
    printf("going grab\n ");
    while(go_grab(S1, S2, S3));
    printf("going sensor\n ");
    state = 2;
  }

  else if (state == 2){
    go_sensor();
    if(color_read!=0){
      printf("cor: %c\n ", color_read);
      state = 3;
    }   
  }

  else if(state == 3){
    while(go_box(color_read));
    state = 1;
    return 1;
  }

  return 0;
}

int main(void){

uint8_t modo = 1;       // MODO: 1 auto # 2 manual
uint16_t pos1, pos2, pos3, pulse_with_1, pulse_with_2;

  io_init();              // Inicializa inputs/outputs                            
  timer_init();           // Timer 0 para controlo de estados
  colorsensor_init();     // Medir frequencia da cor
  servoStart();           // Inicia servos
  init_adc();             // Inicia conversão ADC
  printf_init();          // Init the serial port to have the ability to printf
  sei();                  // Enable interrupts after every initialization
  
  servoSet(3, 1500);
  
  if(EEPROM_read_low(SIG_ADRS) != SIG08){                      
    EEPROM_write_low(RED_ADRS, 0);               // Grava se for a primeira execução  
    EEPROM_write_low(GREEN_ADRS, 0);
    EEPROM_write_low(BLUE_ADRS, 0); 
    EEPROM_write_low(UNK_ADRS, 0);
    EEPROM_write_low(SIG_ADRS, SIG08);           // Assina Endreço
  }

  print_report();

  while (1) {

    //SELECT MODE

    if (modo == 1 && RE_B1() ) {
      printf(" MODO MANUAL \n");
      modo = 2; 
    }
    else if (modo == 2 && RE_B1() ) {
      printf(" MODO AUTOMATICO \n ");
      modo = 1; 
    }
    
    // MODO AUTO

    if (modo == 1 && RE_B2()){  

      printf("going neutro\n ");
      while(go_neutral()); 
      
      start_T(1,200);
      while(!end_T(1));
      while(!piece(1550, 2080, 475));   // 0,0
      start_T(1,200);
      while(!end_T(1));
      while(!piece(1552, 2080, 505));   // 0,1
      start_T(1,200);
      while(!end_T(1));
      while(!piece(1550, 2080, 530));   // 0,2
    
      start_T(1,200);
      while(!end_T(1));
      while(!piece(1480, 1990, 470));   // 1,0
      start_T(1,200);
      while(!end_T(1));
      while(!piece(1480, 1990, 505));   // 1,1
      start_T(1,200);
      while(!end_T(1));
      while(!piece(1480, 1990, 530));   // 1,2
    
      start_T(1,500);
      while(!end_T(1));
      while(!piece(1360, 1885, 470));   // 2,0
      start_T(1,500);
      while(!end_T(1));
      while(!piece(1360, 1885, 495));   // 2,1
      start_T(1,500);
      while(!end_T(1));
      while(!piece(1360, 1885, 540));   // 2,2
    
      printf("going neutro\n ");
      while(go_neutral()); 
    }

    // MODO MANUAL
    if (modo == 2 ) {
    
      pos1 = read_adc(0) - 53;      //potenciometro servo 1 e 2 com ajustes
      pos2 = read_adc(1) - 423;
      pos3 = read_adc(2);           //potenciometro servo 3

      pulse_with_1 = pos1 + 1220;   //pulso max e min ajustado
      pulse_with_2 = pos2 + 1600;

      servoSet(1, pulse_with_1); 
      servoSet(2, pulse_with_2);

      if (RE_B2()) { 
        printf("MODO MANUAL: Pulse With » S1: %d - S2: %d - S3 Pos: %d \n", pulse_with_1, pulse_with_2, pos3);
        printf("%c \n", get_color());  
      }
      if ( B2_is_set()) set_GRABER(1);
      else set_GRABER(0);
    
    }
  }
  return 0;
}