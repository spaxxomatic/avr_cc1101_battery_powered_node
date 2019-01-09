/**
 * regtable
 *
 * This file is part of the spaxstack project.
 * 
 * spaxstack  is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 * 
 * panStamp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with panStamp; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 
 * USA
 * 
 * Author: Francisco Barrena
 * Creation date: 04/10/2015
 */


#include "garagentor.h"
#include "debug.h"
#include "nvolat.h"
#include "utils.h"

#define EEPROM_STATE EEPROM_FIRST_CUSTOM + 1
/**
 * Declaration of custom functions
 */
const void closeDoor(byte rId);
const void openDoor(byte rId);
const void sendImpulse(byte rId);
const void getDoorStatus(byte rId);
const void getCaps(byte rId);


/**
 * Definition of common getter/setter callback functions
 */

/**
 * Definition of common registers
 */
DEFINE_COMMON_REGISTERS()
DEFINE_COMMON_CALLBACKS()
/*
 * Definition of custom registers
 */
// Door status 
static byte tor_status[1];
const void setTorState(byte id, byte *state);

REGISTER regDoorState(tor_status, sizeof(tor_status), &getDoorStatus, &setTorState);
//REGISTER regOpenDoor(tor_status, sizeof(tor_status), &openDoor, &setTorState);
//REGISTER regSendImpulse(tor_status, sizeof(tor_status), &sendImpulse, NULL);
REGISTER regGetDoorStatus(tor_status, sizeof(tor_status), &getDoorStatus, NULL);
//REGISTER regGetCapabilities(status, sizeof(status), &getCaps, NULL);

/**
 * Initialize table of registers
 */
DECLARE_REGISTERS_START()
  &regDoorState,  
  &regGetDoorStatus
DECLARE_REGISTERS_END()

/*PROGMEM const char capabilities[] = {
    "Set door state\r\n"
		"Get door status\r\n"
		"Get capabilities\r\n"
		"\r\n"
};*/
 
/**
 * closeDoor
 *
 * Closes the garage door
 *
 */

const void setTorState(byte id, byte* state)           
{   
  
  SERIAL_DEBUGC("SET STAT ");
  if ((uint8_t) *state >= CMD_OPEN){
    //power up motor
    digitalWrite(GATE_MOTOR_POWER_ON_PIN,  LOW); //it is inverted
    digitalWrite(MAINS_POWER_RELAY_PIN, LOW);
    tor_status[0] = STAT_OPENING;
    keep_charger_on = true;
  }else{
    digitalWrite(GATE_MOTOR_POWER_ON_PIN,  HIGH);
    tor_status[0] = STAT_CLOSING;
    keep_charger_on = false;
    //digitalWrite(MAINS_POWER_RELAY_PIN, LOW);
  }
  SERIAL_DEBUG(tor_status[0]);
}
/*
const void openDoor(byte rId){
  FLASH_LED(2);
  SERIAL_DEBUG("OPEN");
  SERIAL_DEBUG(rId);
  tor_status[1] = STAT_OPEN;
};
const void sendImpulse(byte rId){
  FLASH_LED(2);
  SERIAL_DEBUG("SEND PULSE");
  SERIAL_DEBUG(rId);
};
*/
const void getDoorStatus(byte rId){
  FLASH_LED(3);
  SERIAL_DEBUGC("GET STAT ");
  SERIAL_DEBUG(tor_status[0]);
};

const void getCaps(byte rId){
  SERIAL_DEBUG("CAPS");
  SERIAL_DEBUG(rId);
};

const void pollRealState(){
  if (digitalRead(DOOR_CLOSED_INPUT_SENSOR) == 0){
    tor_status[0] = STAT_CLOSED;
  }
  if (digitalRead(DOOR_OPENED_INPUT_SENSOR) == 0){
    tor_status[0] = STAT_OPENED;
  }
  else tor_status[0] = STAT_UNKNOWN;
}
