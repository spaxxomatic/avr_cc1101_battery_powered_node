
#ifndef _DEVICEDEFS_H
#define _DEVICEDEFS_H

#define DEVICEID_GARAGENTOR_RECEIVER 1
#define DEVICEID_HUMIDITY_SENSOR 2
#define DEVICEID_GENERIC_SENSOR 3
#define DEVICEID_REMOTE_CONTROL 4
#define DEVICEID_PRESENCE_DETECTOR 5

#ifndef DEVICE_TYPE
#error DEVICE_TYPE must be provided as a compiler flag by the environment
#endif

#if DEVICE_TYPE == DEVICEID_GARAGENTOR_RECEIVER
#include "../devices/garagentor/pinout.h"
#include "../devices/garagentor/product.h"
#endif

#if DEVICE_TYPE == DEVICEID_GENERIC_SENSOR
#include "../devices/sensornode/pinout.h"
#include "../devices/sensornode/pinout.h"
#endif

#if DEVICE_TYPE == DEVICEID_REMOTE_CONTROL
#error Not implemented yet
#endif

#if DEVICE_TYPE == DEVICEID_PRESENCE_DETECTOR
#error Not implemented yet
#endif

#endif //_DEVICEDEFS_H