#include "battery.h"
#include "Arduino.h"
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool keep_charger_on=false;

extern byte batt_voltage[2];
extern uint8_t cnt_send_batt_state;

void enable_mains_power(bool state){
  if (keep_charger_on) digitalWrite(MAINS_POWER_RELAY_PIN, LOW);
  Serial.print("CHRG ");
  digitalWrite(MAINS_POWER_RELAY_PIN, !state); //inverted
  state?Serial.println("ON"):Serial.println("OFF");
  //send state change to master
}

uint16_t checkBateryState() {
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

  uint16_t result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  //return result; // Vcc in millivolts
  if (result >= BATT_FULL_LEVEL) { //batt loaded
    enable_mains_power(false);
    //Serial.println("B F");
  }else if (result <= BATT_RECHARGE_LEVEL) { //batt shall be recharged
    enable_mains_power(true);
    //Serial.println("B l");
  }else{
    //Serial.println("B ok");
  }
  batt_voltage[1] = result>>8;
  batt_voltage[0] = result & 0xFF;
  if (cnt_send_batt_state > 0) cnt_send_batt_state --;
  return result;
}
