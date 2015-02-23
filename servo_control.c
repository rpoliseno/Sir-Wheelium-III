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
#include "stm8l15x_gpio.h"
#include "timing_delay.h"

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
   UINT16 captureCompareVal;
   UINT8 gpio_pin;
} SERVO_MOTOR;

typedef struct
{
   SERVO_MOTOR Motors[4];
} SERVO_MOTORS;

/* Private defines -----------------------------------------------------------*/
#define ONE_EIGHTY_DEGREES (0x7FFF)
#define FIRING_SERVO_FIRING_POSITION   (0x0000)
#define FIRING_SERVO_LOADING_POSITION  (0x6000)
#define FIRING_SERVO_DELAY             (750)

//Defines for servo timing requirements
#define CLK_FREQUENCY                     (16000000)
#define PRESCALER                         (0x000F)
#define SERVO_UPDATE_TIME_IN_MS           (62)
#define SERVO_UPDATE_TIME_IN_TIMER_TICKS  (SERVO_UPDATE_TIME_IN_MS*ONE_MS_IN_TIMER_TICKS)//(61091)
#define NO_REPETITION                     (0)
#define ONE_MS_IN_TIMER_TICKS             ((CLK_FREQUENCY/PRESCALER)/1000) ///(1455)

#define TIMER_INTERRUPTS      (  TIM1_IT_Update | \
                                 TIM1_IT_CC1    | \
                                 TIM1_IT_CC2    | \
                                 TIM1_IT_CC3    | \
                                 TIM1_IT_CC4)
                              
#define SERVO_GPIO_PORT                GPIOA

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
SERVO_MOTORS ServoMotors;
/* Private function prototypes -----------------------------------------------*/
void timerForServos_Init(void);
void setServoOutput(SERVO_MOTOR * const motor); 
/* Public functions-----------------------------------------------------------*/

/**
  * @brief  Initialize the Servo Module
  * @param  None
  * @retval None
  */
void ServoModule_Init(void)
{
   UINT8 i;
   
   //initialize timer needed for servos
   timerForServos_Init();
   
   //initialize motor array
   ServoMotors = 
      (SERVO_MOTORS){
         (SERVO_MOTOR){SERVO_LOADING,         THREE_SIXTY_DEGREE,  0x0000, 0, GPIO_Pin_3},   //currently unused
         (SERVO_MOTOR){SERVO_FIRING,          ONE_EIGHTY_DEGREE,   FIRING_SERVO_LOADING_POSITION, 0, GPIO_Pin_2},
         (SERVO_MOTOR){SERVO_HORIZONTAL_AIM,  THREE_SIXTY_DEGREE,  0x7FFF, 0, GPIO_Pin_4},
         (SERVO_MOTOR){SERVO_VERTICLE_AIM,    ONE_EIGHTY_DEGREE,   0x3FFF, 0, GPIO_Pin_5}
      };
  
   //send servos to their initial positions
   for(i = 0; i < NUM_SERVO_MOTORS; i++)
   {
      //initialize the appropriate outputs
      GPIO_Init(SERVO_GPIO_PORT, ServoMotors.Motors[i].gpio_pin, GPIO_Mode_Out_PP_Low_Fast);
      setServoOutput(&(ServoMotors.Motors[i]));
   }
}

/**
  * @brief  Set the angle for a specific servo motor
  * @param  SERVO_NAME servo
  *         UINT16 angle
  * @retval FALSE if the angle was invalid for the motor specified, otherwise TRUE
  */
bool ServoModule_SetServoAngle(UINT8 servo, UINT16 angle)
{
  bool retVal = FALSE;
  
  //only send the motor to the level if it is valid for this motor type
  if ((ServoMotors.Motors[servo].type == THREE_SIXTY_DEGREE) ||
      ((ServoMotors.Motors[servo].type == ONE_EIGHTY_DEGREE) &&
       (angle <= ONE_EIGHTY_DEGREES)))
  {
      ServoMotors.Motors[servo].angle = angle;
      setServoOutput(&ServoMotors.Motors[servo]);
      retVal = TRUE;
  }
  return retVal;
}

/**
  * @brief  Deassert the specified output
  * @param  
  * @retval 
  */
void ServoModule_DeassertServoOutputs(UINT8 captureCompareNumTriggered)
{
  SERVO_GPIO_PORT->ODR &= (UINT8)~ServoMotors.Motors[captureCompareNumTriggered].gpio_pin;
}

/**
  * @brief  Assert all of the servo outputs
  * @param  
  * @retval 
  */
void ServoModule_AssertServoOutputs(void)
{
   UINT8 i = 0;
   
   //assert all outputs
   for (i=0; i<NUM_SERVO_MOTORS; i++)
   {
      SERVO_GPIO_PORT->ODR |= (UINT8)ServoMotors.Motors[i].gpio_pin;
   }   
}

/**
  * @brief  Follows a series of steps to load and fire a set number of balls
  * @param  uint8_t firing_number
  * @retval 
  */
extern void ServoModule_Fire(uint8_t numberToFire)
{
   uint8_t i = 0;
   
   for(i=0; i<numberToFire; i++)
   {
     //we should have a ball loaded into the barrel already
     ServoModule_SetServoAngle((uint8_t)SERVO_FIRING, FIRING_SERVO_FIRING_POSITION);
     Delay(FIRING_SERVO_DELAY);
     ServoModule_SetServoAngle((uint8_t)SERVO_FIRING, FIRING_SERVO_LOADING_POSITION);
     Delay(FIRING_SERVO_DELAY);
   }
}

/* Private function ----------------------------------------------------------*/

/**
  * @brief  Initializes Timer1 and enables the appropriate interrupts
  * @param  
  * @retval 
  */
void timerForServos_Init(void)
{
  //enable timer
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, ENABLE);
  //we're using this timer with timebased functionality
  //prescaler = 0x0B = 11 => 45.055 mS before timer rolls over (need 42mS period)
  TIM1_TimeBaseInit((UINT16)PRESCALER, 
                    TIM1_CounterMode_Up, 
                    (UINT16)SERVO_UPDATE_TIME_IN_TIMER_TICKS, 
                    (UINT8)NO_REPETITION);
  //enable the interrupts
  TIM1_ITConfig((TIM1_IT_TypeDef)TIMER_INTERRUPTS , ENABLE);
  //enable the timer
  TIM1_Cmd(ENABLE);
}

/**
  * @brief  Sets timer value for specified motor based on angle and motor type
  * @param  SERVO_MOTOR const * const motor
  * @retval 
  */
void setServoOutput(SERVO_MOTOR * const motor)
{
   UINT32 temp32 = 0;
   UINT16 captureCompareValue = 0;

   //scale up the angle
   temp32 = ((UINT32)motor->angle * (UINT32)ONE_MS_IN_TIMER_TICKS);   
   
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
   
   motor->captureCompareVal = captureCompareValue;
   
   
   //set the appropriate capture compare value
   switch (motor->name)
   {
      case SERVO_LOADING:
      {
         TIM1_SetCompare1(motor->captureCompareVal);
      }
      break;
      case SERVO_FIRING:
      {
         TIM1_SetCompare2(motor->captureCompareVal);
      }
      break;
      case SERVO_HORIZONTAL_AIM:
      {
         TIM1_SetCompare3(motor->captureCompareVal);
      }
      break;
      case SERVO_VERTICLE_AIM:
      {
         TIM1_SetCompare4(motor->captureCompareVal);
      }
      break;
      default:
      {
        //just something to breakpoint on
        //we should never reach this
        captureCompareValue = 0;
      }
      break;
   }
   TIM1_Cmd(ENABLE);
}
