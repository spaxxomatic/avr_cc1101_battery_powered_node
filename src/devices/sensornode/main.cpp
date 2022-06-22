
#include "protocol.h"
#include "wdt.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "spaxstack.h"
#include "include/devicedefs.h"
#include "debug.h"
#include "include/battery.h"
#include "timermacros.h"
#include "cmd/cmd.h"
#include "garagentor.h"

byte b;
byte i;
long counter=0;


void setup(){
  Serial.begin(57600);
  // setup the blinker output
  pinMode(LEDOUTPUT, OUTPUT);
  checkBateryState();
  commstack.init();
  enable_wdt();
  Serial.print("\nSTART"); //cc1101=0
  Serial.print(commstack.cc1101.readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER));
  Serial.print(" V "); //cc1101=0
  Serial.println(commstack.cc1101.readReg(CC1101_VERSION, CC1101_STATUS_REGISTER));
  setupActivityTimer();  
  FLASH_LED(2);
}

bool bAdj = false;

const bool pollInputs(){  
  if (isStateChanged()){    
    if (expected_state == tor_status[0]){ trigger send state
      bSendState = true;
      bStateReached = true;
    }else{
      //dbgStr = "N STATE CH";
      NOTIFY_STATE_CHANGE;
    }
  };
  return bStateReached;
}


void wdt_loop(){
  //Check the inputs state periodically. If changed, the send flag will be set and the state change will be picked up by the sendState thread
  if (commstack.f_wdt%WDT_CYCLES_CHECK_DOOR_STATE == 0){
    pollInputs();
  }
  if (commstack.f_wdt >= WDT_CYCLES_CHECK_BAT){ //Check batt state each WDT_CYCLES_CHECK_BAT seconds and return
    commstack.f_wdt = 0;
    checkBateryState();
  }
}


const void handleStateEvents(){ 
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

void loop(){
  wdt_loop();
  check_serial_cmd(); //serial command avail?
  handleStateEvents(); //send status
  sendBattState(); //send batt state from time to time
  commstack.receive_loop();
  // Enable wireless reception interrupt and eventually enter sleep
  commstack.enterSleep();
}
