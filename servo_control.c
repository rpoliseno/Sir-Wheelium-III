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
#include "stm8l15x_tim1.h"

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

//Defines for servo timing requirements
#define PRESCALER             (0x000B)
#define FOURTY_TWO_MS         (61091)
#define NO_REPETITION         (0)
#define ONE_MS_IN_TIMER_TICKS (1455)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
SERVO_MOTORS ServoMotors;
/* Private function prototypes -----------------------------------------------*/
void initTimerForServos(void);
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
   
   //initialize timer needed for servos
   initTimerForServos();
  
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

/* Private function ----------------------------------------------------------*/

/**
  * @brief  Sets timer value for specified motor based on angle and motor type
  * @param  SERVO_MOTOR const * const motor
  * @retval 
  */
void initTimerForServos(void)
{
  //enable timer
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, ENABLE);
  //we're using this timer with timebased functionality
  //prescaler = 0x0B = 11 => 45.055 mS before timer rolls over (need 42mS period)
  TIM1_TimeBaseInit((UINT16)PRESCALER, 
                    TIM1_CounterMode_Up, 
                    (UINT16)FOURTY_TWO_MS, 
                    (UINT8)NO_REPETITION);
  //enable the interrupt
  TIM1_ITConfig(TIM1_IT_Update);
  //enable the timer
  TIM1_Cmd(ENABLE);
}

/**
  * @brief  Sets timer value for specified motor based on angle and motor type
  * @param  SERVO_MOTOR const * const motor
  * @retval 
  */
void setServoOutput(SERVO_MOTOR const * const motor)
{
   UINT32 temp32 = 0;
   UINT16 captureCompareValue = 0;

   //scale up the angle
   temp32 = (motor->angle * (UINT16)ONE_MS_IN_TIMER_TICKS);   
   
   if (motor->type == ONE_EIGHTY_DEGREE)
   {
      //divide by 0x8000 by shifting 15
      captureCompareValue = (UINT16)(temp32 >> 15);
   }
   else
   {
      //divide by 0x10000 by shifting by 16
      captureCompareValue = (UINT16)(temp32 >> 16);
   }

   //the first mS is required to be high
   captureCompareValue += (UINT16)ONE_MS_IN_TIMER_TICKS;
   
   //set the appropriate capture compare value
   switch (motor->name)
   {
      case SERVO_LOADING:
      {
         TIM1_SetCompare1(captureCompareValue);
      }
      break;
      case SERVO_LOADING:
      {
         TIM1_SetCompare2(captureCompareValue);
      }
      break;
      case SERVO_LOADING:
      {
         TIM1_SetCompare3(captureCompareValue);
      }
      break;
      case SERVO_LOADING:
      {
         TIM1_SetCompare4(captureCompareValue);
      }
      break;
      default
      {
        //just something to breakpoint on
        //we should never reach this
        captureCompareValue = 0;
      }
      break;
   }
}
