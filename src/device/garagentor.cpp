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


/**
 * Declaration of custom functions
 */
const void closeDoor(byte rId);
const void openDoor(byte rId);
const void sendImpulse(byte rId);
const void getStatus(byte rId);

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
static byte status[2];
REGISTER regCloseDoor(status, sizeof(status), &closeDoor, NULL);
REGISTER regOpenDoor(status, sizeof(status), &openDoor, NULL);
REGISTER regSendImpulse(status, sizeof(status), &sendImpulse, NULL);
REGISTER regGetStatus(status, sizeof(status), &getStatus, NULL);

/**
 * Initialize table of registers
 */
DECLARE_REGISTERS_START()
  &regCloseDoor,
  &regOpenDoor,
  &regSendImpulse,
  &regGetStatus
DECLARE_REGISTERS_END()



/**
 * Definition of custom getter/setter callback functions
 */
 
/**
 * closeDoor
 *
 * Closes the garage door
 *
 */

const void closeDoor(byte rId){
//activate power for the garage door device

//check if door state is not closed 

//send pulse to the input

};
const void openDoor(byte rId){

};
const void sendImpulse(byte rId){

};
const void getStatus(byte rId){

};
