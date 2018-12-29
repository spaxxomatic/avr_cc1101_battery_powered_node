//#include "EEPROM.h"
#include "protocol.h"
#include "wdt.h"
//#include "cc1101.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "spaxstack.h"
#include "device/garagentor.h"
#include "cmd/cmd.h"
 
// The ACTIVITY_LED is wired to the Arduino Output 4
#define LEDOUTPUT 4

//the MAINS_POWER_RELAY_PIN is connected to the relay that switches on the power source
//by that reloading the battery
//A1 is pin 15 
#define GATE_MOTOR_POWER_ON_PIN 15 
//A2 is pin 16
#define MAINS_POWER_RELAY_PIN 16

byte b;
byte i;
long counter=0;

#define BATT_FULL_LEVEL 4100 //we do not want to fully load the batt because the cc1101 is directly connected to the battery (with a 0,7V drop diode)
#define BATT_RECHARGE_LEVEL 3500

void enable_mains_power(boolean state){
  Serial.print("CHARGE ");
  if (state) {
    Serial.println("ON");
    digitalWrite(MAINS_POWER_RELAY_PIN, HIGH);
  }else{
    Serial.println("OFF");
    digitalWrite(MAINS_POWER_RELAY_PIN, LOW);
  }
}

long checkBateryState() {
  Serial.print("Ck batt state"); 
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  //return result; // Vcc in millivolts
  if (result >= BATT_FULL_LEVEL) { //batt loaded
    enable_mains_power(false);
  }
  if (result <= BATT_RECHARGE_LEVEL) { //batt shall be recharged
    enable_mains_power(true);
  }
  return result;
}

void setup(){
  Serial.begin(57600);
  // setup the blinker output
  pinMode(LEDOUTPUT, OUTPUT);
  digitalWrite(LEDOUTPUT, LOW);
  // blink once to signal the setup
  flashLed(2);

  commstack.init();

  Serial.print("CC1101 P "); //cc1101=0
  Serial.print(commstack.cc1101.readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER));
  Serial.print(" V "); //cc1101=0
  Serial.println(commstack.cc1101.readReg(CC1101_VERSION, CC1101_STATUS_REGISTER));
  
  commstack.getAddress();
}

bool bAdj = false;

#define LED_PIN 4

void flashLed( uint8_t no_of_flashes){
  for (int i = 0; i < no_of_flashes; i++){
        digitalWrite(LED_PIN, HIGH);
        delay(30);
        digitalWrite(LED_PIN, LOW);
        delay(20);
  }
}

void showActivity(){
  flashLed(2);
}

#define WDT_CYCLES_CHECK_BAT 10

void wdt_loop(){
  if (commstack.f_wdt >= WDT_CYCLES_CHECK_BAT){ //Check batt state each WDT_CYCLES_CHECK_BAT seconds and return
    commstack.f_wdt = 0;
    checkBateryState();
    //bEnterSleep = true;
  }  
}

void loop(){
  wdt_loop();
  check_serial_cmd();
  commstack.receive_loop();
  // Enable wireless reception interrupt and eventually enter sleep
  commstack.enterSleep();
}
