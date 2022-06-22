

#ifndef _TIMERMACROS_H
#define _TIMERMACROS_H

#include "Arduino.h"

 // The ACTIVITY_LED is wired to the Arduino Output 4
#define LEDOUTPUT 4


extern volatile uint8_t flashLedCnt ;

#define TIMER_FREQ 8
//make sure sleep is disabled as long as the counter runs
#define FLASH_LED(no_of_flashes) digitalWrite(LEDOUTPUT, HIGH); flashLedCnt = 2*no_of_flashes; \
  bitSet(TIMSK1, OCIE1A); \
  commstack.bEnterSleepAllowed = false

void setupActivityTimer();

#endif