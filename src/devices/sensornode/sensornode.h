#ifndef _REGTABLE_H
#define _REGTABLE_H

#include "utils.h"
#include "register.h"
#include "commonregs.h"
#include "pinout.h"

//Check the door state periodically
#define WDT_CYCLES_CHECK_DOOR_STATE 4 

DEFINE_REGINDEX_START()
  REGI_READINPUT1,
  REGI_READINPUT2,
  REGI_GETCAPS
DEFINE_REGINDEX_END()

enum TOR_COMMAND
{
    CMD_CLOSE = 0, 
    CMD_PULSE,
    CMD_OPEN, 
    CMD_POWER_MOTORUNIT
};

const void state_init();
const void sendBattState();
const void triggerAlarm(byte reason);

#endif
