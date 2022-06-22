#ifndef _CMD_H_
	#define _CMD_H_	
		
	#include <avr/io.h>
	#include <string.h>
	#include <stdlib.h>
	#include <avr/eeprom.h>
	#include <avr/pgmspace.h>
	
	typedef struct
	{
		char cmd[3]; 				        
		void(*fp)(void);  	// Zeiger auf Funktion
		char* descr;
	} COMMAND_STRUCTUR;	
	
	#define MAX_VAR	2
	#define HELPTEXT	1
	
	unsigned char check_serial_cmd ();
	extern void write_eeprom_ip (unsigned int);
	
	//reset the unit
	extern void command_reset		(void);
	extern void command_radio_addr 			(void);
	extern void command_radio_channel 			(void);
	extern void command_help  			(void);
	extern void command_factory_settings (void);
	extern void command_enable_debug (void);
	extern void command_ping (void);
	extern void command_alarm (void);
	extern void command_stat (void);
	extern void command_sendstat (void);
	extern void command_activate_sleep (void);	
	extern void command_send_device_typedef(void);
	#define RESET() {asm("ldi r30,0"); asm("ldi r31,0"); asm("ijmp");}
	
#endif //_CMD_H_


