/*
*  File - Protocol
*  Author - Derek Thrasher
*  Date Created - 2/21/215
*/

//Includes
#include "Protocol.h"
#include "servo_control.h"
#include "MotorControl.h"

/*
* \brief Process received Commands
*/
void Protocol__ReceivedCommand(SIR_WHEELIUM_CMD nextCommand)
{
  switch(nextCommand.CMD)
    {
       case 0:
         {
           break;
         }
      case COMMAND_ADJUST_SPEED:
      {
        MotorControl_SetSpeed((UINT16)((UINT16)(nextCommand.DATA[0] << 8) | (nextCommand.DATA[1])));
        break;
      }
      case COMMAND_ADJUST_ANGLE:
      {
        ServoModule_SetServoAngle(nextCommand.DATA[0], (UINT16)((UINT16)(nextCommand.DATA[1] << 8) | (nextCommand.DATA[2])));
        break;
      }
      case COMMAND_LAUNCH:
      {
        break;
      }
       case 0x33:
         {
           break;
         }
       default:
         {
           break;
         }
    }
}