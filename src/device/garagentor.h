#ifndef _REGTABLE_H
#define _REGTABLE_H

#include "product.h"
#include "utils.h"
#include "register.h"
#include "commonregs.h"
#include "pinout.h"
/**
 * Register indexes
 */
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
    CMD_STOP,
    CMD_OPEN, 
    CMD_POWER_DEVICE
};

enum TOR_STATUS 
{
    STAT_UNKNOWN = 0, 
    STAT_OPENING, 
    STAT_OPENED,
    STAT_CLOSING,
    STAT_CLOSED
};

#endif
