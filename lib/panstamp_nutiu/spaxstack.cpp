#include "spaxstack.h"
#include "commonregs.h"
#include "coroutine.h"
#include "calibration.h"
#include "wdt.h"
#include "protocol.h"
#include "debug.h"

//#define enableIRQ_GDO0()          attachInterrupt(0, isrGDO0event, FALLING);
#define enableIRQ_GDO0()          attachInterrupt(0, cc1101Interrupt, FALLING);
#define disableIRQ_GDO0()         detachInterrupt(0);

DEFINE_COMMON_REGINDEX_START()
DEFINE_COMMON_REGINDEX_END()


#define SYNCWORD1        0xB5    // Synchronization word, high byte
#define SYNCWORD0        0x47    // Synchronization word, low byte

/**
 * Array of registers
 */
extern REGISTER* regTable[];
extern byte regTableSize;

/** ----------- ISR section --------- **/
/* Handle interrupt from CC1101 (INT0) gdo0 on pin2 */
void cc1101Interrupt(void){
// set the flag that a package is available
  sleep_disable();
  disableIRQ_GDO0();
  commstack.packetAvailable = true;
}

/**
 * 
 * Class constructor
 */
SPAXSTACK::SPAXSTACK(void)
{
  statusReceived = NULL;
  repeater = NULL;
  // a flag that a wireless packet has been received
  packetAvailable = false;
  bEnterSleep = false;
  f_wdt = 0;
  seqNo = 0;
  bDebug = false;
}

void SPAXSTACK::report_freq(void)
{
    Serial.print("Ch ");
    Serial.print(commstack.cc1101.channel);
    Serial.print(" M:");
    Serial.print(commstack.cc1101.offset_freq1, HEX);
    Serial.print(" m:");
    Serial.println(commstack.cc1101.offset_freq0, HEX);    
}

void SPAXSTACK::dump_regs(void)
{
    //dump regs
    /*uint8_t i;
    for (i=0; i <=CC1101_TEST0; i++){
      Serial.println(commstack.cc1101.readReg(i, CC1101_CONFIG_REGISTER));
      delay(4);
      }
    */      
    Serial.print("MARC STATE ");
    Serial.println(commstack.cc1101.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f);
    
    Serial.print ("RSSI ");
    byte rssi=0;
    byte lqi=0;
    byte value=0;

    rssi=commstack.cc1101.readReg(CC1101_RSSI, CC1101_STATUS_REGISTER);

    if (rssi >= 128){
      value = 255 - rssi;
      value /= 2;
      value += 74;
    }else{
      value = rssi/2;
      value += 74;
    }    
    Serial.print (value);
    
    Serial.print ("\tLQI ");    
    lqi=commstack.cc1101.readReg(CC1101_LQI, CC1101_STATUS_REGISTER);
    value = 0x3F - (lqi & 0x3F);
    Serial.print (value);

    Serial.print (" \tpstat ");
    Serial.println(commstack.cc1101.readReg(CC1101_PKTSTATUS, CC1101_STATUS_REGISTER), BIN);    
}
/**
 * enableRepeater
 *
 * Enable repeater mode
 *
 * 'maxHop'  MAximum repeater count. Zero if omitted
 */
void SPAXSTACK::enableRepeater(byte maxHop)
{
  if (repeater == NULL)
  {
    static REPEATER repe;
    repeater = &repe;
    repeater->init(maxHop);
  }

  if (maxHop == 0)
    repeater->enabled = false;
}

/**
 * getRegister
 *
 * Return pointer to register with ID = regId
 *
 * 'regId'  Register ID
 */
REGISTER * getRegister(byte regId)
{
  if (regId >= regTableSize)
    return NULL;
  return regTable[regId]; 
}

//bring AVR to sleep. It will be woken up by the radio on packet receive
void enterDeepSleepWithRx(){
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    //WDTCSR = 1<<WDP0 | 1<<WDP3; // set new watchdog timeout prescaler to 8.0 seconds    
    WDTCSR = WDPS_1S ;
    WDTCSR |= _BV(WDIE); // Enable the WD interrupt (no reset)
    
    sleep_enable();
    attachInterrupt(0, cc1101Interrupt, HIGH);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();
    sleep_bod_disable();
    sei();
    sleep_cpu();
    // wake up here ->
    sleep_disable();
}

void SPAXSTACK::enterSleep(){
  enableIRQ_GDO0();
  if (bEnterSleep) enterDeepSleepWithRx();
}

/**
 * ISR(WDT_vect)
 *
 * Watchdog ISR. Called whenever a watchdog interrupt occurs
 */
ISR(WDT_vect){
    commstack.f_wdt +=1;
}

/**
 * setup_watchdog
 * 
 * 'time'	Watchdog timer value
 */
void SPAXSTACK::setup_watchdog(byte time) 
{
  byte bb;

  bb = time & 7;
  if (time > 7)
    bb|= (1<<5);

  bb|= (1<<WDCE);

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);    // Enable Watchdog interrupt
}

/**
 * Timer 2 (RTC) ISR routine
 */
ISR(TIMER2_OVF_vect)
{
}

/**
 * setup_rtc
 *
 * Setup software (Timer 2) RTC
 *
 * 'time'   Timer2 prescaler
 *
 *          RTC_1S = 128 for 1 sec
 *          RTC_2S = 256 for 2 sec
 *          RTC_8S = 1024 for 8 sec
 */
void SPAXSTACK::setup_rtc(byte time)
{
  // Set timer 2 to asyncronous mode (32.768KHz crystal)
  ASSR = (1 << AS2);

  TCCR2A = 0x00;  // Normal port operation
  // (256 cycles) * (prescaler) / (32.768KHz clock speed) = N sec
  TCCR2B = time;  // Timer 2 prescaler

  while (ASSR & (_BV(TCN2UB) | _BV(TCR2AUB) | _BV(TCR2BUB))) {}    // Wait for the registers to be updated    
  TIFR2 = _BV(OCF2B) | _BV(OCF2A) | _BV(TOV2);                     // Clear the interrupt flags

  TIMSK2 = 0x01;  // Enable timer2A overflow interrupt
}

/**
 * init
 * 
 * Initialize commstack board
 */
void SPAXSTACK::init() 
{
  // Calibrate internal RC oscillator
  rtcCrystal = rcOscCalibrate();
  // Setup CC1101
  cc1101.init();
  //first start after flashing, the EEPROM is not set yet
  //freq offset must be set to 0
  if (cc1101.offset_freq0 == 0xFF && cc1101.offset_freq1 == 0xFF ){ 
    Serial.println("Resetting freq regs to 0"); 
    cc1101.adjustFreq(0x00, 0x00 ,true);
  }  
  cc1101.setSyncWord(SYNCWORD1, SYNCWORD0);
  uint8_t devaddr = EEPROM.read(EEPROM_DEVICE_ADDR);
  cc1101.setDevAddress(devaddr, false);
  cc1101.setCarrierFreq(CFREQ_433);
  cc1101.disableAddressCheck(); //if not specified, will only display "packet received"

  // Read periodic Tx interval from EEPROM
  txInterval[0] = EEPROM.read(EEPROM_TX_INTERVAL);
  txInterval[1] = EEPROM.read(EEPROM_TX_INTERVAL + 1);

  delayMicroseconds(50);  

  // Enter RX state
  cc1101.setRxState();

  // Attach callback function for GDO0 (INT0)
  enableIRQ_GDO0();

  // Default values
  sentPacketNo = 0;
  errorCode = 0;
  stackState = SYSTATE_RXON;
}


/**
 * reset
 * 
 * Reset commstack
 */
void SPAXSTACK::reset() 
{
  // Tell the network that our spaxxity is restarting
  stackState = SYSTATE_RESTART;
  // Reset commstack
  wdt_disable();  
  wdt_enable(WDTO_15MS);
  while (1) {}
}

/**
 * sleepWd
 * 
 * Put commstack into Power-down state during "time".
 * This function uses the internal watchdog timer in order to exit (interrupt)
 * from the power-down state
 * 
 * 'time'	Sleeping time:
 *  WDTO_15MS  = 15 ms
 *  WDTO_30MS  = 30 ms
 *  WDTO_60MS  = 60 ms
 *  WDTO_120MS  = 120 ms
 *  WDTO_250MS  = 250 ms
 *  WDTO_500MS  = 500 ms
 *  WDTO_1S = 1 s
 *  WDTO_2S = 2 s
 *  WDTO_4S = 4 s
 *  WDTO_8S = 8 s
 */
void SPAXSTACK::sleepWd(byte time) 
{
  // Power-down CC1101
  cc1101.setPowerDownState();
  // Power-down commstack
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  setup_watchdog(time);
  delayMicroseconds(10);
  // Disable ADC
  ADCSRA &= ~(1 << ADEN);
  // Unpower functions
  PRR = 0xFF;
  //power_all_disable();
  //clock_prescale_set(clock_div_8);
  // Enter sleep mode
  sleep_mode();

  // ZZZZZZZZ...

  // Wake-up!!
  wakeUp(false);
}

/**
 * sleepRtc
 * 
 * Put commstack into Power-down state during "time".
 * This function uses Timer 2 connected to an external 32.768KHz crystal
 * in order to exit (interrupt) from the power-down state
 * 
 * 'time'	Sleeping time:
 *  RTC_250MS  = 250 ms
 *  RTC_500MS  = 500 ms
 *  RTC_1S = 1 s
 *  RTC_2S = 2 s
 *  RTC_8S = 8 s
 */
void SPAXSTACK::sleepRtc(byte time) 
{
  // Power-down CC1101
  cc1101.setPowerDownState();
  // Power-down commstack
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  sleep_enable();
  setup_rtc(time);
  delayMicroseconds(10);
  // Disable ADC
  ADCSRA &= ~(1 << ADEN);
  // Unpower functions
  PRR = 0xFF;
  // Enter sleep mode
  sleep_mode();

  // ZZZZZZZZ...

  // Wake-up!!
  wakeUp(false);
}

/**
 * wakeUp
 *
 * Wake from sleep mode
 *
 * 'rxOn' Enter RX_ON state after waking up
 */
void SPAXSTACK::wakeUp(bool rxOn) 
{
  // Exit from sleep
  sleep_disable();
  //wdt_disable();
  // Re-enable functions
  //clock_prescale_set(clock_div_1);
  power_all_enable();
  // Enable ADC
  ADCSRA |= (1 << ADEN);
  
  // If 32.768 KHz crystal enabled
  if (rtcCrystal)
  {
    // Disable timer2A overflow interrupt
    TIMSK2 = 0x00;
  }

  // Reset CC1101 IC
  cc1101.wakeUp();

  if (rxOn)
    stackState = SYSTATE_RXON;
}

/**
 * goToSleep
 *
 * Sleep whilst in power-down mode. This function currently uses sleepWd in a loop
 */
void SPAXSTACK::enterSleepWithRadioOff(void)
{
  // Get the amount of seconds to sleep from the internal register
  int intInterval = txInterval[0] * 0x100 + txInterval[1];
  int i, loops;
  byte minTime;
  
  // No interval? Then return
  if (intInterval == 0)
    return;

  // Search the maximum sleep time passed as argument to sleepWd that best
  // suits our desired interval
  if (intInterval % 8 == 0)
  {
    loops = intInterval / 8;
    
    if (rtcCrystal)
      minTime = RTC_8S;
    else
      minTime = WDTO_8S;
  }
  else if (intInterval % 4 == 0)
  {
    if (rtcCrystal)
    {
      loops = intInterval / 2;
      minTime = RTC_2S;
    }
    else
    {
      loops = intInterval / 4;
      minTime = WDTO_4S;
    }
  }
  else if (intInterval % 2 == 0)
  {
    loops = intInterval / 2;
    if (rtcCrystal)    
      minTime = RTC_2S;
    else
      minTime = WDTO_2S;
  }
  else
  {
    loops = intInterval;
    if (rtcCrystal)
      minTime = RTC_1S;
    else
      minTime = WDTO_1S;
  }

  stackState = SYSTATE_RXOFF;

  // Sleep
  for (i=0 ; i<loops ; i++)
  {
    // Exit sleeping loop?
    if (stackState == SYSTATE_RXON)
      break;

    if (rtcCrystal)
      sleepRtc(minTime);
    else
      sleepWd(minTime);
  }
  stackState = SYSTATE_RXON;
}


/**
 * getInternalTemp
 * 
 * Read internal (ATMEGA328 only) temperature sensor
 * Reference: http://playground.arduino.cc/Main/InternalTemperatureSensor
 * 
 * Return:
 * 	Temperature in degrees Celsius
 */
long SPAXSTACK::getInternalTemp(void) 
{
  unsigned int wADC;
  long t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celcius.
  return (t);
}

/**
 * setTxInterval
 * 
 * Set interval for periodic transmissions
 * 
 * 'interval'	New periodic interval. 0 for asynchronous devices
 * 'save'     If TRUE, save parameter in EEPROM
 */
void SPAXSTACK::setTxInterval(byte* interval, bool save)
{
  memcpy(txInterval, interval, sizeof(txInterval));

  // Save in EEPROM
  if (save)
  {
    EEPROM.write(EEPROM_TX_INTERVAL, interval[0]);
    EEPROM.write(EEPROM_TX_INTERVAL + 1, interval[1]);
  }
}

void SPAXSTACK::sendAck(void){ 
  //SWACK swack(master_address);  
};

/**
 * waitState
 * 
 * Waits that the RX IRQ sets the stack state to the given state. 
 * 
 * 'state'	Expected state
 */
boolean SPAXSTACK::waitState(cor_state* cs){  
  while (cs->wait_resp_timeout-- != 0 ){
    if (stackState == cs->state) return true;  
    showActivity(); 
  }
  return false;
}

/**
 * getAddress
 * 
 * Sends a broadcast request for a device address. When addr is received, sets it and enables cc1101 packet filtering
 * 
 */
boolean SPAXSTACK::getAddress(void)
{
  // Broadcast addr request
  byte retry = 0;
  cor_state cs = {MAX_WAIT_RESPONSE, STACKSTATE_READY};
  while (retry++ > MAX_RETRY_SEND_DATA){
    stackState = STACKSTATE_WAIT_CONFIG;
    //SWSTATUS packet = SWSTATUS(REGI_DEVADDRESS, 0, length);
    SWQUERY(0,0,REGI_DEVADDRESS).send();
    //Wait for a response. When the status is set to SYSTATE_READY, all went fine
    if (waitState(&cs)){
      SERIAL_DEBUGC("Got address");
      SERIAL_DEBUG(cc1101.devAddress);
      return true;
    }
  }
  // if we're here, no response was received 
  //for the address request broadcast query
  SERIAL_DEBUG("No addr");
  return false;
}

/**
 * ping
 * 
 * Checks if a server can be reached
 */
boolean SPAXSTACK::ping(void) {
  SERIAL_DEBUG("Ping");
  if (SWQUERY(0,0,REGI_DEVADDRESS).send())
  SERIAL_DEBUG("Sent"); //stack state is set to  STACKSTATE_WAIT_ACK
  //TODO: Wait response
  return true;
  //cor_state cs = {MAX_WAIT_RESPONSE, STACKSTATE_READY};
  //return waitState(&cs);
}


/**
 * spaxstack packet decoder
 *
 */
void SPAXSTACK::decodePacket(CCPACKET* ccPacket){
    static SWPACKET swPacket;
    REGISTER *reg;
    swPacket = SWPACKET(ccPacket);
    // Function
    switch(swPacket.function)
    {
        case SWAPFUNCT_ACK:
          if (swPacket.destAddr != cc1101.devAddress){
            if (commstack.stackState == STACKSTATE_WAIT_ACK){
              //check packet no
              if (swPacket.packetNo == sentPacketNo){
                stackState = STACKSTATE_READY;
              }
            }else{
              errorCode = STACKERR_ACK_WITHOUT_SEND;
            }
          }else{
            errorCode = STACKERR_WRONG_DEST_ADDR;
          }
            break;                
        case SWAPFUNCT_CMD:
          // Command not addressed to us?
          if (swPacket.destAddr != cc1101.devAddress)
            break;
          // Destination address and register address must be the same
          if (swPacket.destAddr != swPacket.regAddr)
            break;
          // Valid register?
          if ((reg = getRegister(swPacket.regId)) == NULL)
            break;
          // Filter incorrect data lengths
          if (swPacket.value.length == reg->length)
            reg->setData(swPacket.value.data);
          else
            reg->sendSwapStatus();
          break;
        case SWAPFUNCT_QRY:
          // Only Product Code can be broadcasted
          if (swPacket.destAddr == SWAP_BCAST_ADDR)
          {
            if (swPacket.regId != REGI_PRODUCTCODE)
              break;
          }
          // Query not addressed to us?
          else if (swPacket.destAddr != cc1101.devAddress)
            break;
          // Current version does not support data recording mode
          // so destination address and register address must be the same
          if (swPacket.destAddr != swPacket.regAddr)
            break;
          // Valid register?
          if ((reg = getRegister(swPacket.regId)) == NULL)
            break;
          reg->getData();
          break;
        case SWAPFUNCT_STA:
          // User callback function declared?
          if (statusReceived != NULL)
            statusReceived(&swPacket);
          break;
        default:
          break;
    }
}

void SPAXSTACK::receive_loop(){
  //if (cc1101.rfState == RFSTATE_RX) //needed??
  if(packetAvailable){
    Serial.print("!");
    // clear the flag
    packetAvailable = false;
    CCPACKET packet;
    delay(2); //need a short delay here. Without this there are CRC errors after coming back from sleep mode
    if(cc1101.receiveData(&packet) > 0){
      if(!packet.crc_ok) {
        Serial.println("CRC ERR");
      }
      //showActivity();
      if (bDebug){
        if(packet.length > 0){
          //Serial.print("packet: len ");
          //Serial.print(packet.length);
          Serial.print("D: ");
          for(int j=0; j<packet.length; j++){
            if (j>0) SERIAL_DEBUGC(":");
            SERIAL_DEBUGC(packet.data[j],HEX);
          }
          SERIAL_DEBUG(" ");
          
        }
      }
      decodePacket(&packet);  
    }else{
       Serial.println("No data");
    }
  }
}

/**
 * Pre-instantiate SPAXSTACK object
 */
SPAXSTACK commstack;