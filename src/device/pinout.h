 // The ACTIVITY_LED is wired to the Arduino Output 4
#define LEDOUTPUT 4

//the MAINS_POWER_RELAY_PIN is connected to the relay that switches on the power source
//When switched on, the charger starts loading the battery

#define GATE_MOTOR_POWER_ON_PIN 5 
#define MAINS_POWER_RELAY_PIN 6
#define PULSE_SWITCH 3
//#define PULSE_SWITCH 15

//There are two additional SCR mains switches connected to A1 and A2
//A1 is pin 15
//A2 is pin 16
#define SSR_SWITCH_1 15 
#define SSR_SWITCH_2 16

#define DOOR_CLOSED_INPUT_SENSOR 8
#define DOOR_OPENED_INPUT_SENSOR 7

