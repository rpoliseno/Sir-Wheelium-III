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
#define COMMAND_ADJUST_SPEED  (0x30)
#define COMMAND_ADJUST_ANGLE  (0x31)
#define COMMAND_LAUNCH        (0x32)


//Global Functions
extern void Protocol__ReceivedCommand(SIR_WHEELIUM_CMD nextCommand);
#endif
