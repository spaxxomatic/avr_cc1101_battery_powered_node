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
  digitalWrite(LEDOUTPUT, LOW);
  // blink once to signal the setup
  pinMode(GATE_MOTOR_POWER_ON_PIN, OUTPUT);
  pinMode(MAINS_POWER_RELAY_PIN, OUTPUT);
  pinMode(SSR_SWITCH_1, OUTPUT);
  pinMode(SSR_SWITCH_2, OUTPUT);
  checkBateryState();
  commstack.init();
  enable_wdt();
  Serial.print("\nSTART \n CC1101 P "); //cc1101=0
  Serial.print(commstack.cc1101.readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER));
  Serial.print(" V "); //cc1101=0
  Serial.println(commstack.cc1101.readReg(CC1101_VERSION, CC1101_STATUS_REGISTER));
  setupActivityTimer();
  FLASH_LED(2);
}

bool bAdj = false;

#define WDT_CYCLES_CHECK_BAT 10

void wdt_loop(){
  if (commstack.f_wdt >= WDT_CYCLES_CHECK_BAT){ //Check batt state each WDT_CYCLES_CHECK_BAT seconds and return
    commstack.f_wdt = 0;
    checkBateryState();
  }  
}

void loop(){
  wdt_loop();
  check_serial_cmd(); //serial command avail?
  commstack.receive_loop();
  // Enable wireless reception interrupt and eventually enter sleep
  commstack.enterSleep();
}
