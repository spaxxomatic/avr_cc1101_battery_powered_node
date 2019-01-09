#include "utils.h"
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

volatile uint8_t flashLedCnt ;

ISR(TIMER1_COMPA_vect) 
{ //1000 Hz here
  static uint8_t cnt ;
  cnt+=1;
  if (flashLedCnt > 0){
    digitalWrite(LEDOUTPUT, cnt%2);
    flashLedCnt--;
  }else{
    digitalWrite(LEDOUTPUT, LOW);
    bitClear(TIMSK1, OCIE1A);
  }
}

void setupActivityTimer(){
  cli();
  flashLedCnt = 0;
  MCUSR = 0;  // clear out any flags of prior resets.
// -- init timers
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B

  //OCR1A = 15624; // set compare match register to desired timer count. 16 MHz with 1024 prescaler = 15624 counts/s
  OCR1A = 1953/2; // set compare match register to desired timer count. 16 MHz with 1024 prescaler 15624/8=1953 -> 8 Hz interrupt frequency
  //OCR1A = 249; // set compare match register to desired timer count. 16 MHz with 64 prescaler 250000/1000=250 -> 1000 Hz interrupt frequency
  TCCR1B |= (1 << WGM12); // turn on CTC mode. clear timer on compare match
  TCCR1B |= (1 << CS10); // Set CS10 and CS11 bits for 64 prescaler
  //TCCR1B |= (1 << CS11); 
  TCCR1B |= (1 << CS10); // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12);
  //TIMSK1 |= (1 << OCIE1A); // enable timer 1 compare interrupt
  bitSet(TIMSK1, OCIE1A);
  sei();
};

