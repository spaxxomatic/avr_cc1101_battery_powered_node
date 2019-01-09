#include "pinout.h"
#include "Arduino.h"

#define BATT_FULL_LEVEL 4100 //we do not want to fully load the batt because the cc1101 is directly connected to the battery (with a 0,7V drop diode)
#define BATT_RECHARGE_LEVEL 3500

extern volatile uint8_t flashLedCnt ;

#define FLASH_LED(no_of_flashes) flashLedCnt = 2*no_of_flashes;  bitSet(TIMSK1, OCIE1A);

void setupActivityTimer();
