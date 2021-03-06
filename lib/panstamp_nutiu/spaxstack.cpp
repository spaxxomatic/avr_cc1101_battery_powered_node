#include "spaxstack.h"
#include "commonregs.h"
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
  if (commstack.cc1101.rfState != RFSTATE_TX){
    commstack.packetAvailable = true;
  }
  FLASH_LED(2);
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
  bEnterSleepAllowed = false;
  bSleepActivated = false;
  f_wdt = 0;
  seqNo = 0;
  bDebug = false;
}

void SPAXSTACK::report_freq(void)
{
    Serial.print("Ch:");
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
    report_freq();
    Serial.print("MARC STATE ");
    Serial.println(commstack.cc1101.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f);
    
    Serial.print ("RSSI ");
    byte lqi=0;
    byte value=0;
    Serial.print (commstack.cc1101.ReadRSSI());
    
    Serial.print ("LQI ");    
    lqi=commstack.cc1101.readReg(CC1101_LQI, CC1101_STATUS_REGISTER);
    value = 0x3F - (lqi & 0x3F);
    Serial.print (value);

    Serial.print (" \tpstat ");
    Serial.println(commstack.cc1101.readReg(CC1101_PKTSTATUS, CC1101_STATUS_REGISTER), BIN);    
    
    Serial.print ("ADDR ");
    Serial.println(commstack.cc1101.devAddress);    
    
    Serial.print ("CHANNEL ");
    Serial.println(commstack.cc1101.channel);      

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

//enable the watchdog timer
void enable_wdt(){
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    WDTCSR = WDPS_1S ; // set the watchdog timeout prescaler to 1 second
    WDTCSR |= _BV(WDIE); // Enable the WD interrupt (no reset)
}

//bring AVR to sleep. It will be woken up by the radio on packet receive
void enterDeepSleepWithRx(){
    digitalWrite(LEDOUTPUT, LOW); //turn off activity led, just in case it was left on
    enable_wdt();
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
  if (bSleepActivated && bEnterSleepAllowed) enterDeepSleepWithRx();
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
  errorCode = 0;
  stackState = SYSTATE_RXON;
}

#define RESET() {asm("ldi r30,0"); asm("ldi r31,0"); asm("ijmp");}
/**
 * reset
 * 
 * Reset commstack
 */
void SPAXSTACK::reset() 
{
  // Tell the network that our spaxxity is restarting
  stackState = SYSTATE_RESTART;
  Serial.println("RST RQ");
  // Reset commstack
  RESET();
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

  // The returned temperature is in degrees Celsius.
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

void SPAXSTACK::sendControlPkt(byte function, byte dAddr, byte packetNo, byte errorCode){ 
  CCPACKET packet;
  packet.length = 6 ;
  packet.data[0] = dAddr;
  packet.data[1] = commstack.cc1101.devAddress;
  packet.data[2] = 0;
  packet.data[3] = packetNo;
  packet.data[4] = function; //the function
  packet.data[5] = errorCode; //status
  commstack.cc1101.sendData(packet);
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
    //showActivity(); 
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
  if (cc1101.devAddress != 0xFF && cc1101.devAddress != 0xFF) return true;
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

boolean SPAXSTACK::ping(void) {
  SERIAL_DEBUG("Ping");
  if (SWQUERY(0,0,REGI_DEVADDRESS).send())
  SERIAL_DEBUG("Sent"); 
  //TODO: Wait response
  //TODO: set state to  STACKSTATE_WAIT_ACK
  return true;
  //cor_state cs = {MAX_WAIT_RESPONSE, STACKSTATE_READY};
  //return waitState(&cs);
}

/**
 * spaxstack packet decoder
 *
 */
void SPAXSTACK::decodePacket(){
    static SWPACKET swPacket;
    REGISTER *reg;
    swPacket = SWPACKET(&ccReceivedPacket);
    uint8_t errCode = STACKERR_OK;
    SERIAL_DEBUGC(" SRC");
    SERIAL_DEBUGC(swPacket.srcAddr);
    SERIAL_DEBUGC(" DEST ");
    SERIAL_DEBUG(swPacket.destAddr);
    if (swPacket.destAddr == SWAP_BCAST_ADDR){
      //TODO: implement broadcast reply
      //sendAck(swPacket.srcAddr, swPacket.packetNo, STACKERR_OK);
      return;
    }
    if (swPacket.destAddr != cc1101.devAddress){ //packet not addressed to us, exit
      return;
    } 
    switch(swPacket.function)
    {           
        case SWAPFUNCT_CMD:
          // Valid register?
          if ((reg = getRegister(swPacket.regId)) == NULL){
            errCode = STACKERR_REGISTER_NOT_FOUND;
          }else{
          // Filter incorrect data lengths
            if (swPacket.value.length == reg->length){
              errCode = reg->setData(swPacket.value.data);
              if (errCode == 0){
                reg->sendSwapStatus(swPacket.srcAddr, swPacket.packetNo); //reply to the request with this packetno
                return ;
              }
            }else{
              errCode = STACKERR_INVALID_PARM_LENGTH;
            }
          }
          break;
        case SWAPFUNCT_QRY:
          // Valid register?
          if ((reg = getRegister(swPacket.regId)) == NULL){
            errCode = STACKERR_REGISTER_NOT_FOUND;
          }else{
            errCode = reg->getData();
            if (errCode == 0){     
              reg->sendSwapStatus(swPacket.srcAddr, swPacket.packetNo); //reply to the request with this packetno
              return;
            }
          }
          break;
        case SWAPFUNCT_STA:
          errCode =  STACKERR_OK;
          break;
        default:
          errCode =  STACKERR_UNKNOWN_FUNCTION;
    }
    //if we reached this point, everything is fine, send an ACK
    sendControlPkt(SWAPFUNCT_ACK, swPacket.srcAddr, swPacket.packetNo, errCode);
}

void SPAXSTACK::printPacketState(){
  if (crc_err) SERIAL_DEBUG("CRC ERR");
  SERIAL_DEBUGC("RSSI ");
  SERIAL_DEBUG(cc1101.ConvertRSSI(ccReceivedPacket.rssi));
}

void SPAXSTACK::receive_loop(){
  //if (cc1101.rfState == RFSTATE_RX) //needed??
  bool bDecode = false;
  if(packetAvailable){
    crc_err = false;
    Serial.print("!\n");
    // clear the flag
    packetAvailable = false;
    delay(2); //need a short delay here. Without this there are CRC errors after coming back from sleep mode
    if(cc1101.receiveData(&ccReceivedPacket) == cc1101.RX_OK){
      if(!ccReceivedPacket.crc_ok) {
        crc_err = true;
      }else{
        dbgprintPacket('R', &ccReceivedPacket);
        //rssi = cc1101.ReadRSSI();
        bDecode = true;
      }
    }else{
     cc1101.printRxError();
    }
    printPacketState();
    if (bDecode) decodePacket();
  }
  if (cc1101.checkRxState() != 0){  //make sure the radio is in a clean RX state
    SERIAL_DEBUG("?STATE? RESET!");
    RESET();
  }
}

/**
 * Pre-instantiate SPAXSTACK object
 */
SPAXSTACK commstack;