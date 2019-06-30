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
 * Creation date: 03/03/2011
 */

#include "swpacket.h"
#include "debug.h"
#include "spaxstack.h"

/**
 * SWPACKET
 * 
 * Class constructor
 * 
 * 'packet'	Raw CC1101 packet
 */
SWPACKET::SWPACKET(CCPACKET* packet) 
{
  destAddr = packet->data[0];
  srcAddr = packet->data[1];
  hop = (packet->data[2] >> 4) & 0x0F;
  encrypted = bitRead(packet->data[2], 1);
  
  //todo: implement flag ACK REQUEST ackReq = (packet->data[2] >> 4) & 0x0F;
  packetNo = packet->data[3];
  function = packet->data[4];
  regAddr = packet->data[5];
  regId = packet->data[6];
  value.is_string = bitRead(packet->data[2], 0);
  value.data = packet->data + 7;
  value.length = packet->length - SWAP_DATA_HEAD_LEN - 1;
}

/**
 * SWPACKET
 * 
 * Class constructor
 */
SWPACKET::SWPACKET(void) 
{
}

/**
 * send
 * 
 * Send SWAP packet. Do up to SWAP_NB_TX_TRIES retries if necessary
 *
 * Return:
 *  True if the transmission succeeds
 *  False otherwise
 */
boolean SWPACKET::send(void)
{
  CCPACKET packet;
  byte i;
  boolean res;
  delay(SWAP_TX_DELAY);
  packet.length = value.length + SWAP_DATA_HEAD_LEN + 1;
  packet.data[0] = destAddr;
  packet.data[1] = srcAddr;
  packet.data[2] = (hop << 4) & 0xF0;
  if (encrypted) bitSet(packet.data[2], 1);
  if (request_ack) bitSet(packet.data[2], 2);
  packet.data[3] = packetNo; 
  packet.data[4] = function;
  packet.data[5] = regAddr;
  packet.data[6] = regId;

  for(i=0 ; i<value.length ; i++)
    packet.data[i+7] = value.data[i];

  i = SWAP_NB_TX_TRIES;
  
  while(!(res = commstack.cc1101.sendData(packet)) && i>1){
    i--;
    delay(SWAP_TX_DELAY);
  }

  //commstack.stackState = STACKSTATE_WAIT_ACK;
  return res;
}

void SWPACKET::crypt(bool decrypt) 
{
  return;
  /*TODO
  if (decrypt)
    nonce ^= swap.encryptPwd[9];

  function ^= swap.encryptPwd[11] ^ nonce;
  srcAddr ^= swap.encryptPwd[10] ^ nonce;
  regAddr ^= swap.encryptPwd[8] ^ nonce;
  regId ^= swap.encryptPwd[7] ^ nonce;

  /* payload encryption not implemented yet
  static uint8_t newData[CCPACKET_DATA_LEN];
  byte i, j = 0;  
  for(i=0 ; i<value.length ; i++)
  {
    newData[i] = value.data[i] ^ swap.encryptPwd[j] ^ nonce;
    j++;
    if (j == 11)  // Don't re-use last byte from password
      j = 0;
  }
  if (value.length > 0)
    value.data = newData;
  */
/*
  if (!decrypt)
    nonce ^= swap.encryptPwd[9];
*/    
}



