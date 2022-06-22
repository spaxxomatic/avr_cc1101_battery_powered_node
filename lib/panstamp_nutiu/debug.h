#ifndef debug_h
#define debug_h
#include "ccpacket.h"


#define SERIAL_DEBUG_ON
#ifdef SERIAL_DEBUG_ON
#define SERIAL_DEBUG Serial.println
#define SERIAL_DEBUGC Serial.print
#else
#define SERIAL_DEBUG(...)
#define SERIAL_DEBUGC(...)
#endif

#ifdef PRINT_RADIO_PACKET
#define PRINTLN_DATA Serial.println
#define PRINT_DATA Serial.print
#else
#define PRINT_DATA(...)
#define PRINTLN_DATA(...)
#endif

void dbgprintPacket(char direction, CCPACKET* packet);

#endif //debug_h