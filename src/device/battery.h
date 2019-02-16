#include "Arduino.h"
#include "pinout.h"

#define BATT_FULL_LEVEL 4200 
#define BATT_RECHARGE_LEVEL 3900


#define WDT_CYCLES_CHECK_BAT 10 //about 10 sec
#define CNT_SEND_BATT_STATUS WDT_CYCLES_CHECK_BAT*20 // 20*10 sec between batt status send

uint16_t checkBateryState() ;
void enable_mains_power(bool state);
