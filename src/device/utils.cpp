#include "utils.h"
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "spaxstack.h"
#include "garagentor.h"

volatile uint8_t flashLedCnt ;
volatile uint8_t impulseSwitchContdownCnt ;
volatile uint16_t trackDoorStateCnt;
volatile uint16_t motorShutdownContdownCnt;

ISR(TIMER1_COMPA_vect) 
{ //8 Hz here
  static uint8_t cnt ;
  cnt+=1;
  if (flashLedCnt > 0){
    digitalWrite(LEDOUTPUT, cnt%2);
    flashLedCnt--;
  }
  if (flashLedCnt == 0) digitalWrite(LEDOUTPUT, LOW);

  if (impulseSwitchContdownCnt > 0){
    if (impulseSwitchContdownCnt == 3) digitalWrite(PULSE_SWITCH, HIGH);
    impulseSwitchContdownCnt--;
  }
  if (impulseSwitchContdownCnt == 0) digitalWrite(PULSE_SWITCH, LOW);
  
  if (motorShutdownContdownCnt == 1) digitalWrite(GATE_MOTOR_POWER_ON_PIN,  HIGH); //power off the motor unit;
  if (motorShutdownContdownCnt > 1) impulseSwitchContdownCnt--;

  if (trackDoorStateCnt > 0){
    if (pollRealDoorState()) { 
      //door reached expected state
      trackDoorStateCnt = 0; //stop this countdown
    }else{
      trackDoorStateCnt--;
    }
    if (trackDoorStateCnt == 1){ 
    //the timer is expired - it should have been resetted by a door state change
    //but this did not happend so send an alarm
      triggerAlarm(ERR_TIMEOUT_WAITING_FOR_STATE);
      trackDoorStateCnt = 0; //stop this countdown
    }
  }
  
  //if all counter are at 0, stop timer and allow sleep mode
  if (trackDoorStateCnt==0 && flashLedCnt==0 && impulseSwitchContdownCnt==0 && motorShutdownContdownCnt==0){
    bitClear(TIMSK1, OCIE1A);
    commstack.bEnterSleep = true;
  }  
}

void setupActivityTimer(){
  cli();
  flashLedCnt = 0;
  impulseSwitchContdownCnt = 0;
  trackDoorStateCnt = 0;
  MCUSR = 0;  // clear out any flags of prior resets.
// -- init timers
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B

  //OCR1A = 15624; // set compare match register to desired timer count. 16 MHz with 1024 prescaler = 15624 counts/s
  OCR1A = 1953; // set compare match register to desired timer count. 16 MHz with 1024 prescaler 15624/8=1953 -> 8 Hz interrupt frequency
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

