/**
 * Copyright (c) 2018 autonity <contact@autonity.de>
 * 
 * This file is part of the spaxxity project.
 * 
 * spaxxity is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 * 
 * spaxxity is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with spaxxity; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 
 * USA
 * 
 * Author: Daniel Berenguer, Lucian Nutiu 
 * Creation date: 04/24/2011
 */

#include "register.h"
#include "swstatus.h"
#include "spaxstack.h"
#include "protocol.h"
byte regIndex = 0;

/**
 * getData
 * 
 * Update and get register value
 */
uint8_t REGISTER::getData(void) 
{
  // Update register value
  if (updateValue != NULL)
    updateValue(id);
}

/**
 * setData
 * 
 * Set register value
 * 
 * 'data'	New register value
 */
uint8_t REGISTER::setData(byte *data) 
{
  // Update register value
  if (setValue != NULL)
    return setValue(id, data);
  else return ERR_REGISTER_HAS_NO_SETTER;
}

/**
 * sendSwapStatus
 * 
 * Send SWAP status message
 */
void REGISTER::sendSwapStatus(byte destAddr, byte packetNo) 
{
  delay(SWAP_TX_DELAY);
  SWSTATUS packet = SWSTATUS(id, value, length);
  packet.destAddr = destAddr;
  packet.packetNo = packetNo;
  packet.send();
}

