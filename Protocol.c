/*
*  File - Protocol
*  Author - Derek Thrasher
*  Date Created - 2/21/215
*/

//Includes
#include "Protocol.h"

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
        break;
      }
      case COMMAND_ADJUST_ANGLE:
      {
        
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