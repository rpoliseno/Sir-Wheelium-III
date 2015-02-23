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
       case COMMAND_HALT_MOTORS:
         {
           // shut off the motors
           MotorControl_SetSpeed(MOTOR_TYPE_TOP, 0x0000);
           MotorControl_SetSpeed(MOTOR_TYPE_BOTTOM, 0x0000);
           break;
         }
      case COMMAND_ADJUST_SPEED:
      {
        MotorControl_SetSpeed((MOTOR_TYPES)(nextCommand.DATA[0]), (UINT16)((UINT16)(nextCommand.DATA[1] << 8) | (nextCommand.DATA[2])));
        break;
      }
      case COMMAND_ADJUST_ANGLE:
      {
        ServoModule_SetServoAngle(nextCommand.DATA[0], (UINT16)((UINT16)(nextCommand.DATA[1] << 8) | (nextCommand.DATA[2])));
        break;
      }
      case COMMAND_LAUNCH:
      {
        ServoModule_Fire(nextCommand.DATA[0]);
        break;
      }
      default:
      {
        break;
      }
    }
}