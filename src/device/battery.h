#include "pinout.h"

#define BATT_FULL_LEVEL 4100 //we do not want to fully load the batt because the cc1101 is directly connected to the battery (with a 0,7V drop diode)
#define BATT_RECHARGE_LEVEL 3500

long checkBateryState() ;
void enable_mains_power(bool state);
