#include "protocol.h"
#include "wdt.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "spaxstack.h"
#include "device/garagentor.h"
#include "debug.h"
#include "device/battery.h"
#include "cmd/cmd.h"

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
  Serial.print("\nSTART \n CC1101 P "); //cc1101=0
  Serial.print(commstack.cc1101.readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER));
  Serial.print(" V "); //cc1101=0
  Serial.println(commstack.cc1101.readReg(CC1101_VERSION, CC1101_STATUS_REGISTER));
  setupActivityTimer();
  torstate_init();
  FLASH_LED(2);
}

bool bAdj = false;

void wdt_loop(){
  //Check the door state periodically. If changed, the send flag will be set and the state will be sent 
  if (commstack.f_wdt%WDT_CYCLES_CHECK_DOOR_STATE == 0){
    pollRealDoorState();
  }
  if (commstack.f_wdt >= WDT_CYCLES_CHECK_BAT){ //Check batt state each WDT_CYCLES_CHECK_BAT seconds and return
    commstack.f_wdt = 0;
    checkBateryState();
  }
}

void loop(){
  wdt_loop();
  check_serial_cmd(); //serial command avail?
  handleDoorStateEvent(); //send status
  sendBattState(); //send batt state from time to time
  commstack.receive_loop();
  // Enable wireless reception interrupt and eventually enter sleep
  commstack.enterSleep();
}
