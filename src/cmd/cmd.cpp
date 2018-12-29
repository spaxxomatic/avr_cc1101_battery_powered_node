#include "cmd.h"
#include "Arduino.h"
#include "spaxstack.h"
#include "debug.h"

volatile unsigned int cmd_vars[MAX_VAR];

//Format of entries in the command table :
// {XX {command string - 2 alphanumeric chars} , handler}, //comment *UDP* {means command can be called by the available to the UDP client and will be added to the client interface}


COMMAND_STRUCTUR COMMAND_TABELLE[] = // Befehls-Tabelle
{
	{"re",command_reset}, //[Reset] *UDP* 
	{"fs",command_factory_settings}, //[Reset] *UDP* 
	//{"BL",command_bootloader}, //[Bootloader]  *UDP*
	{"sa",command_radio_addr},
	{"dd",command_enable_debug},
	{"pi",command_ping},
	{"st",command_stat},
	{"??",command_help},//[Help]
	{{00},NULL} 
};


PROGMEM const char helptext[] = {
		"re Restart\r\n"
		"sa Set radio address\r\n"
		"fs Reset eeprom to factory settings\r\n"
		"dd Enable debug output\r\n"
		"pi Ping\r\n"
		"st Dump status\r\n"
		"?? HELP\r\n"				
		"\r\n"
};

bool bPrintChannel=false;

/*
void serial_cmd() {
  if (Serial.available() > 0) {
	/*	  
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
   }else if (rcv_char == 'q'){
    commstack.bEnterSleep = true;
   }else if (rcv_char == 'w'){
    commstack.bEnterSleep = false;
   } else if (rcv_char == 'D'){
     commstack.bDebug = true;
   } else if (rcv_char == 'A'){
     //set device address
   }
   ;
  if (bPrintChannel){
    bPrintChannel = false;
    commstack.report_freq();
  }
  if (bAdj){
    commstack.cc1101.adjustFreq(commstack.cc1101.offset_freq1, commstack.cc1101.offset_freq0 ,true);
    bAdj = 0;
    commstack.report_freq();
   };
  }
}
*/
#define MAX_PARAM_LENGTH 16
static char cmd[2]; 
static char param[MAX_PARAM_LENGTH]; 
static int bufpos = 0;

int readline() {    
	char readch = Serial.read();
	Serial.print(readch);
    int rpos;
    if (readch > 0) {
        switch (readch) {
            case ' ':
				break; // ignore spaces
			case '\n': // ignore newline
                break;
            case '\r': // cr
                rpos = bufpos;
                bufpos = 0;  // Reset position index ready for next time
                return rpos;
            default:
				if (bufpos < 2){ //first two chars are the command
					cmd[bufpos++] = readch;
				}else{
					if (bufpos < MAX_PARAM_LENGTH + 2 - 1) {
						param[bufpos-2] = readch;
						param[bufpos-1] = 0;
						bufpos++;
					}
				}
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
		Serial.println(" ");
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
		
		//Variablen finden und auswerten
		if (*param != NULL){//posible params comming
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
	RESET();
}

void command_enable_debug (void)
{
	Serial.print("Dbg ");
	commstack.bDebug = !commstack.bDebug ;
	Serial.println(commstack.bDebug);
}


void command_ping (void)
{
	commstack.ping();
}

void command_factory_settings (void)
{
	Serial.println("Rst to factory");
	eepromToFactoryDefaults();
	RESET();
}

/*
void command_bootloader (void)
{
	//produce a reset by activating the watchdog
	wdt_enable(WDTO_120MS) ;while(1){};
}
*/
//------------------------------------------------------------------------------
//print/edit own IP
void command_radio_addr (void)
{
	if (cmd_vars[0] > 255){
		SERIAL_DEBUG("Invalid addr");
	}else{
		SERIAL_DEBUGC("Set radio addr to ");
		SERIAL_DEBUG(cmd_vars[0]);
		commstack.cc1101.setDevAddress(cmd_vars[0], true);
	}
}

//------------------------------------------------------------------------------
//print helptext
void command_help (void)
{
	char data;
	PGM_P helptest_pointer = helptext;
	do
	{
		data = pgm_read_byte(helptest_pointer++);
		SERIAL_DEBUGC(data);
	}while(data != 0);
}

void command_stat ()
{
	commstack.dump_regs();
}
