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


#include <Arduino.h>
#include "garagentor.h"
#include "protocol.h"
#include "debug.h"
#include "nvolat.h"
#include "utils.h"
#include "include/battery.h"


#define EEPROM_STATE EEPROM_FIRST_CUSTOM + 1

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
static byte out1[1];
static byte out2[1];
byte batt_voltage[2];
uint8_t cnt_send_batt_state=0;

const uint8_t setTorState(byte id, byte *state);
const uint8_t setOutState(byte id, byte *state);

static byte expected_state;

static char* dbgStr;

REGISTER regDoorState(tor_status, sizeof(tor_status), &getDoorStatus, &setTorState);
REGISTER regOut1State(out1, sizeof(out1), NULL, &setOutState);
REGISTER regOut2State(out2, sizeof(out2), NULL, &setOutState);
REGISTER regBattState(batt_voltage, sizeof(batt_voltage), NULL, NULL);

/**
 * Initialize table of registers
 */
DECLARE_REGISTERS_START()
  &regDoorState,  
  &regOut1State,
  &regOut2State,
  &regBattState,
DECLARE_REGISTERS_END()

#define STATE_TRACK_CHANGE 0xFF
//2 minutes counter for tracking the door state inputs (waiting for end position)
#define AWAIT_STATE(s) trackDoorStateCnt = 2*60*8;  \
expected_state = s; \
bitSet(TIMSK1, OCIE1A); \
commstack.bEnterSleepAllowed = false 

#define TRACK_STATE() trackDoorStateCnt = 2*60*8;  \
expected_state = STATE_TRACK_CHANGE; \
bitSet(TIMSK1, OCIE1A); \
commstack.bEnterSleepAllowed = false 

#define EVENT_EXPECTED_STATE_REACHED bSendState = true; bTriggerShutdownMotorUnit = true; 
#define NOTIFY_STATE_CHANGE bSendState = true;

//after the sensor indicates the closed door, this timer will trigger the shutdown of the motor unit
#define TRIGGER_SHUTDOWN_MOTORUNIT motorShutdownContdownCnt = 10*8; \
bitSet(TIMSK1, OCIE1A); 

#define UNTRIGGER_SHUTDOWN_MOTORUNIT motorShutdownContdownCnt = 0


static bool bSendState = false;
static bool bTriggerShutdownMotorUnit = false; 
static byte alarm = 0;

const void torstate_init(){
  pinMode(GATE_MOTOR_POWER_ON_PIN, OUTPUT);
  pinMode(MAINS_POWER_RELAY_PIN, OUTPUT);
  pinMode(SSR_SWITCH_1, OUTPUT);
  pinMode(PULSE_SWITCH, OUTPUT);
  pinMode(SSR_SWITCH_2, OUTPUT);
  pinMode(DOOR_CLOSED_INPUT_SENSOR, INPUT);
  digitalWrite(DOOR_CLOSED_INPUT_SENSOR, HIGH);  //pullup
  pinMode(DOOR_OPENED_INPUT_SENSOR, INPUT);
  digitalWrite(DOOR_OPENED_INPUT_SENSOR, HIGH);  //pullup
  pollRealDoorState();
}

extern bool keep_charger_on; // comes from battery.cpp

void enablePowerAndTriggerPulse(){
  UNTRIGGER_SHUTDOWN_MOTORUNIT;
  if (digitalRead(GATE_MOTOR_POWER_ON_PIN) == LOW){ //motor unit power is on
    SEND_MOTORUNIT_PULSE;
  }else{
      digitalWrite(GATE_MOTOR_POWER_ON_PIN,  LOW); //it is inverted
      digitalWrite(MAINS_POWER_RELAY_PIN, LOW);
      keep_charger_on = true;
      SEND_DELAYED_MOTORUNIT_PULSE;
  }
}

const uint8_t setTorState(byte id, byte* state)           
{     
  SERIAL_DEBUGC("RQ STAT:");
  uint8_t cmd = (uint8_t) *state;
  SERIAL_DEBUGC(cmd);
  SERIAL_DEBUGC(" ACT:");
  SERIAL_DEBUG(tor_status[0]);
  if (cmd == CMD_OPEN && tor_status[0] == STAT_OPENED) return 0; //nothing to do, door is already open
  if (cmd == CMD_CLOSE && tor_status[0] == STAT_CLOSED) return 0; //nothing to do, door is already open
  
  commstack.bSleepActivated = false; //deactivate sleep. This is activated only when the door reaches the closed position
  switch(cmd) {
    case CMD_CLOSE:
      enablePowerAndTriggerPulse();
      AWAIT_STATE(STAT_CLOSED);
      break;    
    case CMD_PULSE:
      //best case the power is on already, so send the pulse without delay 
      enablePowerAndTriggerPulse();
      TRACK_STATE();
      break;
    case CMD_OPEN:
      enablePowerAndTriggerPulse();
      AWAIT_STATE(STAT_OPENED);
      break;
    case CMD_POWER_MOTORUNIT:
      digitalWrite(GATE_MOTOR_POWER_ON_PIN,  LOW); 
      break;
    default:       //unknown command
      return ERR_UNKNOWN_COMMAND;
  }
  return 0;
}

const uint8_t setOutState(byte id, byte* state)           
{     
  SERIAL_DEBUGC("SSR STAT "); 
  if (id == regOut1State.id){
    digitalWrite(SSR_SWITCH_1, *state); 
    out1[0] = *state;
  }
  if (id == regOut2State.id){
    digitalWrite(SSR_SWITCH_2, *state); 
    out2[0] = *state;
  }  
  SERIAL_DEBUG (*state); 
  return 0;
}

const uint8_t getDoorStatus(byte rId){
  //FLASH_LED(3);
  SERIAL_DEBUGC("GET STAT ");
  SERIAL_DEBUG(tor_status[0]);
  return tor_status[0];
}

const void sendBattState(){
  if (cnt_send_batt_state == 0){
    cnt_send_batt_state = CNT_SEND_BATT_STATUS;
    regBattState.sendSwapStatus(SWAP_MASTER_ADDRESS,0);
  }
}

const void handleDoorStateEvent(){ 
  if (bSendState){
    regDoorState.sendSwapStatus(SWAP_MASTER_ADDRESS,0);
    bSendState = false;
  }
  if (bTriggerShutdownMotorUnit){
    if (digitalRead(GATE_MOTOR_POWER_ON_PIN) == LOW){ //motor unit power is on
      if (tor_status[0] == STAT_CLOSED) { //if the expected state is closed and we reached the state, enter closed state 
        keep_charger_on = false;
        TRIGGER_SHUTDOWN_MOTORUNIT;
      }
    }
    bTriggerShutdownMotorUnit = false;
  }
  if (alarm){
    commstack.sendControlPkt(SWAPFUNCT_ALARM, SWAP_MASTER_ADDRESS, 0, alarm);
    alarm = 0;
  }
  if (dbgStr != 0){
    SERIAL_DEBUGC(dbgStr);
    SERIAL_DEBUGC( " State: ");
    SERIAL_DEBUGC( tor_status[0]);
    SERIAL_DEBUGC( " exp ");
    SERIAL_DEBUG( expected_state);
    dbgStr = 0;
  }
}

const bool isDoorStateChanged(){
  byte thisState = 0;
  if (digitalRead(DOOR_CLOSED_INPUT_SENSOR) == 0){
    thisState = STAT_CLOSED;
  } else if (digitalRead(DOOR_OPENED_INPUT_SENSOR) == 0){
    thisState = STAT_OPENED;
  }
  else thisState = STAT_UNKNOWN;    
  if (tor_status[0] != thisState){
    tor_status[0] = thisState;
    dbgStr = "STAT CH";
    return 1;
  }
  return 0;
}

const void triggerAlarm(byte reason){
  if (expected_state != STATE_TRACK_CHANGE){ //in track mode (pulsed mode) there is no need to send an alarm when the timer expires
    //tor_status[0] = reason;
    alarm = reason;
  }
};

const bool pollRealDoorState(){
  bool bStateReached = false;
  if (isDoorStateChanged()){    
    if (expected_state == tor_status[0]){ //expected pos reached, trigger send state
      //dbgStr = "N EXP STATE REACHED";
      EVENT_EXPECTED_STATE_REACHED;
      bStateReached = true;
    }else{
      //dbgStr = "N STATE CH";
      NOTIFY_STATE_CHANGE;
    }
  };
  return bStateReached;
}

