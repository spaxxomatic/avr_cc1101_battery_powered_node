
#include "Arduino.h"
#include "spaxstack.h"
#include "debug.h"
#include "utils.h"

void command_trigger_pulse ()
{
		Serial.print("Pulse motor unit");
		SEND_MOTORUNIT_PULSE;		
}