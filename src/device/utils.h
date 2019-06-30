#include "pinout.h"
#include "Arduino.h"

extern volatile uint8_t flashLedCnt ;
extern volatile uint8_t impulseSwitchContdownCnt;
extern volatile uint16_t trackDoorStateCnt;
extern volatile uint16_t motorShutdownContdownCnt;

#define TIMER_FREQ 8
//make sure sleep is disabled as long as the counter runs
#define FLASH_LED(no_of_flashes) digitalWrite(LEDOUTPUT, HIGH); flashLedCnt = 2*no_of_flashes; \
  bitSet(TIMSK1, OCIE1A); \
  commstack.bEnterSleep = false

//5 seconds delay before sending the pulse, and make sure sleep is disabled as long as the counter runs
#define SEND_DELAYED_MOTORUNIT_PULSE impulseSwitchContdownCnt = 5*TIMER_FREQ;  \
bitSet(TIMSK1, OCIE1A);   \
commstack.bEnterSleep = false 

//0,5 delay before sending the pulse
#define SEND_MOTORUNIT_PULSE impulseSwitchContdownCnt = TIMER_FREQ/2;  \
bitSet(TIMSK1, OCIE1A);   \
commstack.bEnterSleep = false 

void setupActivityTimer();
