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
 * Creation date: 07/03/2011
 */

#ifndef _NVOLAT_H
#define _NVOLAT_H

#include "Arduino.h"

/**
 * EEPROM addresses
 */
#define EEPROM_FREQ_CHANNEL       0x00   // 1-byte register
#define EEPROM_NOT_USED           0x01   // 1-byte register
#define EEPROM_SYNC_WORD          0x02   // 2-byte register
#define EEPROM_DEVICE_ADDR        0x04   // 1-byte register
#define EEPROM_TX_INTERVAL        0x05   // 2-byte register

//nutiu add calibration param
#define EEPROM_OFFSET_FREQ1        0x06   // 1-byte register
#define EEPROM_OFFSET_FREQ0        0x07   // 1-byte register

#define EEPROM_FIRST_CUSTOM       0x20

#endif
