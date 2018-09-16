//#include "EEPROM.h"
#include "protocol.h"
#include "wdt.h"
//#include "cc1101.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "spaxstack.h"
#include "device/garagentor.h"

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

#define BATT_FULL_LEVEL 4100 //we do not want to fully load the batt because the cc1101 is directly connected to the battery (with a 0,7 drop diode)
#define BATT_RECHARGE_LEVEL 3500

void enable_mains_power(boolean state){
  Serial.print("B CHARGING ");
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

  Serial.print("CC1101_PARTNUM "); //cc1101=0
  Serial.println(commstack.cc1101.readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER));
  Serial.print("CC1101_VERSION "); //cc1101=4
  Serial.println(commstack.cc1101.readReg(CC1101_VERSION, CC1101_STATUS_REGISTER));
  Serial.print("CC1101_MARCSTATE ");
  Serial.println(commstack.cc1101.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f);
  
  commstack.getAddress();
  //attachInterrupt(0, cc1101signalsInterrupt, FALLING);
  //attachInterrupt(0, cc1101signalsInterrupt, LOW);
}

byte ReadLQI(){
  byte lqi=0;
  byte value=0;
  lqi=(commstack.cc1101.readReg(CC1101_LQI, CC1101_STATUS_REGISTER));
  value = 0x3F - (lqi & 0x3F);
  return value;
}

byte ReadRSSI(){
  byte rssi=0;
  byte value=0;

  rssi=(commstack.cc1101.readReg(CC1101_RSSI, CC1101_STATUS_REGISTER));

  if (rssi >= 128){
    value = 255 - rssi;
    value /= 2;
    value += 74;
  }else{
    value = rssi/2;
    value += 74;
  }
  return value;
}

bool bAdj = false;
bool bPrintData = false;

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

bool bReply=false;
bool bPrintChannel=false;

void serial_cmd() {
  if (Serial.available() > 0) {
   int rcv_char = Serial.read();
   if (rcv_char == '+'){
    commstack.cc1101.offset_freq1 ++;
    bAdj = 1;
   } else if (rcv_char == '-'){
    commstack.cc1101.offset_freq1 --;
    bAdj = 1;
   } else if (rcv_char == '6'){
    commstack.cc1101.offset_freq0 ++;
    bAdj = 1;
   } else if (rcv_char == '4'){
    commstack.cc1101.offset_freq0 --;
    bAdj = 1;
   } else if (rcv_char == '8'){
    commstack.cc1101.setChannel(commstack.cc1101.channel+1, true);
    bPrintChannel = true;
   } else if (rcv_char == '2'){
    commstack.cc1101.setChannel(commstack.cc1101.channel-1, true);
    bPrintChannel = true;
   } else if (rcv_char == 's'){
      commstack.ping();
   }else if (rcv_char == 'i'){
    //bEnableWor = !bEnableWor;
   }else if (rcv_char == 'q'){
    commstack.bEnterSleep = true;
   }else if (rcv_char == 'w'){
    commstack.bEnterSleep = false;
   } else if (rcv_char == 'd'){
    //dump regs
    for (i=0; i <=CC1101_TEST0; i++){
      Serial.println(commstack.cc1101.readReg(i, CC1101_CONFIG_REGISTER));
      delay(4);
      }
   }else if (rcv_char == 'm'){
    Serial.println(commstack.cc1101.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f);
   }else if (rcv_char == 'p'){
    bPrintData = !bPrintData ;
   }else if (rcv_char == 'r'){
      Serial.print ("RSSI ");
      Serial.print (ReadRSSI());
      Serial.print ("\tLQI ");
      Serial.print (ReadLQI());
      Serial.print (" \tpstat ");
      Serial.println(commstack.cc1101.readReg(CC1101_PKTSTATUS, CC1101_STATUS_REGISTER), BIN);
   }
   ;
  if (bPrintChannel){
    bPrintChannel = false;
    Serial.print ("Chan ");
    Serial.println (commstack.cc1101.channel);
  }
  if (bAdj){
    commstack.cc1101.adjustFreq(commstack.cc1101.offset_freq1, commstack.cc1101.offset_freq0 ,true);
    bAdj = 0;
    Serial.print(" M:");
    Serial.print(commstack.cc1101.offset_freq1, HEX);
    Serial.print(" m:");
    Serial.println(commstack.cc1101.offset_freq0, HEX);
   };
  }

}

void dump_rssi(){
    Serial.print ("RSSI ");
    Serial.print (ReadRSSI());
    Serial.print ("LQI ");
    Serial.print (ReadLQI());
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
  serial_cmd();
  //commstack.receive_loop();
  
  if (bReply){
    delay(300);
    //send_data(0xAA);
    bReply = false;
  }
  // Enable wireless reception interrupt
  commstack.enterSleep();
}
