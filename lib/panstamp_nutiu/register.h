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

#ifndef _REGISTER_H
#define _REGISTER_H

#include "Arduino.h"

extern byte regIndex;

/**
 * Class: REGISTER
 * 
 * Description:
 * Register class
 */
class REGISTER
{
  public:
    /**
     * Register id
     */
    const byte id;
    
    /**
     * Register value
     */
    byte *value;
    
    /**
     * Data length
     */
    const byte length;
    private:
      /**
       * Pointer to the register "updater" function
       *
       *  'rId'  Register ID     
       */
      const uint8_t (*updateValue)(byte rId);

      /**
       * Pointer to the register "setter" function
       *
       *  'rId'  Register ID     
       *  'v'    New register value
       */
      const uint8_t (*setValue)(byte rId, byte *v);
    public:
    /**
     * REGISTER
     * 
     * Constructor
     * 
     * 'val'	      Pointer to the register value
     * 'len'	      Length of the register value
     * 'getValH'    Pointer to the getter function
     * 'setValH'    Pointer to the setter function
     */
    REGISTER(byte *val, const byte len, const uint8_t (*updateValH)(byte rId), 
        const uint8_t (*setValH)(byte rId, byte *v)
        ):id(regIndex++), value(val), length(len), updateValue(updateValH), setValue(setValH)
        {
        };

    /**
     * getData
     * 
     * Update and get register value
     * 
     */
    uint8_t getData();

    /**
     * setData
     * 
     * Set register value
     * 
     * 'data'	New register value
     */
    uint8_t setData(byte *data);

    /**
     * sendSwapStatus
     * 
     * Send SWAP status message
     */
    void sendSwapStatus(byte destAddr, byte packetNo);

    /**
     * setRegValue
     *
     * Set register value from different data formats
     * Use this method to simplify LE to BE conversion
     *
     * 'val'   New register value
     */
    template<class T> uint8_t setRegValue(T val)
    {
      uint8_t i;

      for(i=0 ; i<length ; ++i)
      {
        value[i] = val & 0xFF;
        val >>= 8;
      }
      return 0;
    }
};

/**
 * Array of registers
 */
extern REGISTER* regTable[];

/**
 * Extern global functions
 */
extern void setupRegisters(void);

#endif

