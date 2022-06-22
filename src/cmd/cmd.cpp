#include "cmd.h"
#include "Arduino.h"
#include "spaxstack.h"
#include "../include/devicedefs.h"
#include "debug.h"
#include "timermacros.h"
#include "../include/battery.h"
volatile unsigned int cmd_vars[MAX_VAR];

COMMAND_STRUCTUR COMMAND_TABELLE[] = 
{
	{"re",command_reset}, 
	{"fs",command_factory_settings}, 
	{"sa",command_radio_addr},
	{"sc",command_radio_channel},
	{"dd",command_enable_debug},
	{"pi",command_ping},
	{"aa",command_alarm},
	{"st",command_stat},
	{"sd",command_send_device_typedef},
	{"sl",command_activate_sleep},
	{"??",command_help},
	{{00},NULL} 
};

PROGMEM const char helptext[] = {
		"re Restart\r\n"
		"sa Set radio address\r\n"
		"sc Set radio channel\r\n"
		"fs Reset eeprom to factory settings\r\n"
		"dd Enable debug output\r\n"
		"pi Ping\r\n"
		"aa Alarm\r\n"
		"st Dump status\r\n"
		"sd Send device id\r\n"
		"sl Activate sleep\r\n"
		"?? HELP\r\n"				
		"\r\n"
};

bool bPrintChannel=false;

/*
   if (rcv_char == '6'){
    commstack.cc1101.offset_freq0 ++;
    bAdj = 1;
   } else if (rcv_char == '4'){
    commstack.cc1101.offset_freq0 --;
    bAdj = 1;
   } else if (rcv_char == 'D'){
     commstack.bDebug = true;
   ;
  if (bAdj){
    commstack.cc1101.adjustFreq(commstack.cc1101.offset_freq1, commstack.cc1101.offset_freq0 ,true);
    bAdj = 0;
    commstack.report_freq();
   };
*/

#define MAX_PARAM_LENGTH 16
static char cmd[2]; 
static char param[MAX_PARAM_LENGTH]; 
static int bufpos = 0;

int readline() {    
	char readch = Serial.read();
    int rpos;
    if (readch > 0) {
        switch (readch) {
            case ' ':
				Serial.print(' ');
				break; // ignore spaces
			case '\n': // ignore newline
                break;
            case '\r': // cr
                rpos = bufpos;
                bufpos = 0;  // Reset position index ready for next time
				Serial.print(readch);
				return rpos;
            default:
				if (bufpos < 2){ //first two chars are the command
					cmd[bufpos++] = readch;
					*param = 0; //set param empty until eventually one comes after the cmd
				}else{
					if (bufpos < MAX_PARAM_LENGTH + 2 - 1) {
						param[bufpos-2] = readch;
						param[bufpos-1] = 0;
						bufpos++;
					}
				}
				Serial.print(readch);
        }
    }
    return 0;
}
//------------------------------------------------------------------------------
//Decode command
unsigned char check_serial_cmd ()
{
	
	if (Serial.available() == 0) 
		return 0;
  	
	if (readline() > 0) {	
		char *string_pointer_tmp;
		unsigned char cmd_index = 0;
		//Serial.println(" ");
		//Kommando in Tabelle suchen
		while(1)
		{
			if (COMMAND_TABELLE[cmd_index].cmd[0] == cmd[0])
			{
				if (COMMAND_TABELLE[cmd_index].cmd[1] == cmd[1])
					break;
			}
			if (COMMAND_TABELLE[++cmd_index].fp == NULL) {
				Serial.println("?");
				return 0;
			}
		}
		
		//Evaluate possible incoming parameters
		if (*param != 0){//posible params comming
			char* pptr = param;
			for (unsigned char a = 0; a<MAX_VAR; a++)
			{ 
				string_pointer_tmp = strsep(&pptr ,"., ");
				cmd_vars[a] = strtol(string_pointer_tmp,NULL,0);
			}
		}else{ //empty param array
			for (unsigned char a = 0; a<MAX_VAR;a++)
			{ 
				cmd_vars[a] = 0;
			}		
		}
		//Exec command
		FLASH_LED(1);
		COMMAND_TABELLE[cmd_index].fp();
		return(1); 
	}
	return 0;
}

//------------------------------------------------------------------------------
//Reset 
void command_reset (void)
{
	Serial.println("Reboot");
	commstack.reset();
}

void command_enable_debug (void)
{
	Serial.print("Dbg ");
	commstack.bDebug = !commstack.bDebug ;
	Serial.println(commstack.bDebug);
}

void command_ping (void)
{
	uint8_t no_of_pings = cmd_vars[0];
	if (no_of_pings == 0){
		no_of_pings = 1;
	}
	while (no_of_pings-- > 0){
		commstack.ping();
		delay(500);
	}
}

void command_send_device_typedef (void)
{
	SEND_DEVICE_TYPEDEF(DEVICE_TYPE);
}

void command_alarm (void)
{
	commstack.sendControlPkt(SWAPFUNCT_DEVICETYPE, SWAP_MASTER_ADDRESS, 0, 1);
}

void command_activate_sleep (void)
{
	Serial.println("Sleep mode active");
	commstack.bSleepActivated = true;
}

void command_factory_settings (void)
{
	Serial.println("Rst to factory");
	eepromToFactoryDefaults();
	RESET();
}

//------------------------------------------------------------------------------

void command_radio_addr (void)
{
	if (cmd_vars[0] > 255){
		Serial.println("Invalid addr");
	}else{
		Serial.print("Set radio addr to ");
		Serial.println(cmd_vars[0]);
		commstack.cc1101.setDevAddress(cmd_vars[0], true);
	}
}

//------------------------------------------------------------------------------

void command_radio_channel (void)
{
	if (cmd_vars[0] > 255){
		Serial.println("Invalid channel");
	}else{
		Serial.print("Set channel to ");
		Serial.println(cmd_vars[0]);
		commstack.cc1101.setChannel(cmd_vars[0], true);
	}
}
//------------------------------------------------------------------------------
//print helptext
void command_help (void)
{
	char data;
	PGM_P helptest_pointer = helptext;
	do{
		data = pgm_read_byte(helptest_pointer++);
		Serial.print(data);
	}while(data != 0);
}

void command_stat ()
{
	commstack.dump_regs();
	Serial.print("BATT ");
	Serial.print(checkBateryState());
	Serial.println("mV");
}

