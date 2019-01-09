#include "Arduino.h"
#include "debug.h"

void dbgprintPacket(char direction, CCPACKET* packet){
Serial.print(direction);    
Serial.print(':');
#ifdef PRINT_RADIO_PACKET
  for(int j=0; j<packet->length; j++){
    if (j>0) PRINT_DATA(":");
    PRINT_DATA(packet->data[j],HEX);
  }
#endif
Serial.println("");
}