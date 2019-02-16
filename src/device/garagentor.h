#ifndef _REGTABLE_H
#define _REGTABLE_H

#include "product.h"
#include "utils.h"
#include "register.h"
#include "commonregs.h"
#include "pinout.h"

//Check the door state periodically
#define WDT_CYCLES_CHECK_DOOR_STATE 4 

DEFINE_REGINDEX_START()
  REGI_CLOSEDOOR,
  REGI_OPENDOOR,
  REGI_SENDPULSE,
  REGI_GETDOORSTAT,
  REGI_GETCAPS
DEFINE_REGINDEX_END()

extern bool keep_charger_on;

enum TOR_COMMAND
{
    CMD_CLOSE = 0, 
    CMD_PULSE,
    CMD_OPEN, 
    CMD_POWER_MOTORUNIT
};

enum TOR_STATUS 
{
    STAT_CLOSED=0,    
    STAT_UNKNOWN, 
    STAT_OPENED,
    ERR_TIMEOUT_WAITING_FOR_STATE
};

const void torstate_init();
const void closeDoor(byte rId);
const void openDoor(byte rId);
const void sendImpulse(byte rId);
const uint8_t getDoorStatus(byte rId);
const bool pollRealDoorState();
const void sendDoorStat();
const void sendBattState();
const void triggerAlarm(byte reason);

#endif
