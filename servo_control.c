/**
   ******************************************************************************
   * @file    servo_control.c
   * @author  Mike Barber
   * @date    21-February-2015
   * @brief   
   ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "servo_control.h"

/* Private datatypes ---------------------------------------------------------*/

typedef enum
{
   ONE_EIGHTY_DEGREE = 0,  /*will only accept angles between 0x0000 and 0x7FFF*/
   THREE_SIXTY_DEGREE
} SERVO_TYPE;

typedef struct
{
   SERVO_NAME name;
   SERVO_TYPE type;
   UINT16 angle;
} SERVO_MOTOR;

typedef struct
{
   SERVO_MOTOR Motors[4];
} SERVO_MOTORS;

/* Private defines -----------------------------------------------------------*/
#define ONE_EIGHTY_DEGREES (0x7FFF)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
SERVO_MOTORS ServoMotors;
/* Private function prototypes -----------------------------------------------*/
void setServoOutput(SERVO_MOTOR const * const motor); 
/* Public functions-----------------------------------------------------------*/

/**
  * @brief  Initialize the Servo Module
  * @param  None
  * @retval None
  */
void InitServoModule(void)
{
   UINT8 i;
  
   //initialize motor array
   ServoMotors = 
      (SERVO_MOTORS){
         (SERVO_MOTOR){SERVO_LOADING,         ONE_EIGHTY_DEGREE,   0x0000},
         (SERVO_MOTOR){SERVO_FIRING,          THREE_SIXTY_DEGREE,  0x0000},
         (SERVO_MOTOR){SERVO_HORIZONTAL_AIM,  THREE_SIXTY_DEGREE,  0x7FFF},
         (SERVO_MOTOR){SERVO_VERTICLE_AIM,    ONE_EIGHTY_DEGREE,   0x3FFF}
      };
  
   //send servos to their initial positions
   for(i = 0; i < NUM_SERVO_MOTORS; i++)
   {
      setServoOutput(&(ServoMotors.Motors[i]));
   }
}

/**
  * @brief  Set the angle for a specific servo motor
  * @param  SERVO_NAME servo
  *         UINT16 angle
  * @retval FALSE if the angle was invalid for the motor specified, otherwise TRUE
  */
BOOLEAN SetServoAngle(SERVO_NAME servo, UINT16 angle)
{
  BOOLEAN retVal = FALSE;
  
  //only send the motor to the level if it is valid for this motor type
  if ((ServoMotors.Motors[servo].type == THREE_SIXTY_DEGREE) ||
      ((ServoMotors.Motors[servo].type == ONE_EIGHTY_DEGREE) &&
       (ServoMotors.Motors[servo].angle <= ONE_EIGHTY_DEGREES)))
  {
      ServoMotors.Motors[servo].angle = angle;
      setServoOutput(&ServoMotors.Motors[servo]);
      retVal = TRUE;
  }
  return retVal;
}

/**
  * @brief  Sets timer value for specified motor based on angle and motor type
  * @param  SERVO_MOTOR const * const motor
  * @retval 
  */
void setServoOutput(SERVO_MOTOR const * const motor)
{
   
}
