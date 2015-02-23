/*
*  File - Protocol
*  Author - Derek Thrasher
*  Date Created - 2/21/215
*/




#ifndef PROTOCOL_H
#define PROTOCOL_H

//Includes
#include "Bluetooth.h"

//Commands!
#define COMMAND_HALT_MOTORS         (0x00)
#define COMMAND_ADJUST_SPEED        (0x02)
#define COMMAND_ADJUST_ANGLE        (0x03)
#define COMMAND_LAUNCH              (0x04)

//Global Functions
extern void Protocol__ReceivedCommand(SIR_WHEELIUM_CMD nextCommand);
#endif
