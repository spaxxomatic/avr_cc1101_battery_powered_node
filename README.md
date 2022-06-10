# avr_cc1101_battery_powered_node

A generic radio node for home / remote automation. 
It uses a CC1101 radio module + Arduino (Atmel328 - based boards)
Uses a modified panastaqmp cc1101 library and parts of the SWAP protocol (http://www.panstamp.org/) by Daniel Berenguer

Main features:
* low power - the CPU goes into sleep and will wake up on radio reception 
* battery powered - it supervises the batt status and turns on a charger on low voltage. 
* sends regular heartbeat signals including the state and batt voltage
* Node ID and radio channel are configurable over radio. If not configured, device will send config requests over the broadcast channel.  


Todos:

* code is targeted to be used as a garage door controller with status feedback. I should separate the automation logic and set up the code structure as a geeneric  radio node + automation plug-in. It remains to be seen if I'll be ever find the time for that

Bugs / Problems:

* cc1101 seems to be quite a buggy beast. Maybe I just bought some China production fall-offs, but sensitivity is by far not so good as the datasheet claims.
 Radio needs to be resetet when it receives data junk, which might put the internal state machine to an unknown state. This is quite sorted out by checking the state and resetting the radio, and now it runs stable over years. Still, the sensitivity of the device seems to randomly vary. It might have something to do with the calibration cycles, but more research might be needed to find the root cause. 

 All in all, go for Lora if you need more than 100m range