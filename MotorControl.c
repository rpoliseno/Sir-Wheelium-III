/**
 * @file MotorControl.c
 * @brief DeInterface for motor driver
 * @copyright (C)2015 Lutron Electronics Co., Inc.
 *    All rights reserved.
 *    The copyright notice above does not evidence any actual or intended
 *    publication of such source code. This file and the information contained
 *    herein are confidential and proprietary to Lutron Electronics Co., Inc.
 *    Unauthorized possession or use of this file or the information contained
 *    herein is prohibited. No reproduction may be made of this file without
 *    the express written permission of Lutron Electronics Co., Inc.
 *
 * @addtogroup MotorControl
 * @{
 */

/* =============================================================================
 *   Include Files
 * =============================================================================
 */
#include "stm8l15x.h"
#include "stm8_eval.h"
#include "MotorControl.h"
#include "stm8l15x_clk.h"
#include "stm8l15x_gpio.h"
#include "stm8l15x_tim3.h"
#include "HelperFunctions.h"

/* =============================================================================
 *   Private Macros
 * =============================================================================
 */

// GPIO defines
#define TOP_MOTOR_GPIO_PORT             GPIOB
#define BOTTOM_MOTOR_GPIO_PORT          GPIOD
#define TOP_MOTOR_GPIO_PIN          GPIO_Pin_1
#define BOTTOM_MOTOR_GPIO_PIN       GPIO_Pin_0

// Timer Defines
#define TIM3_PERIOD                 (0x7FFF)
#define TIM3_PRESCALER              (1)

#define MOTOR_CONTROL_MAX_SPEED     (0xFFFF)
//Minimum Motor speed is 30% of 0xFFFF
#define MOTOR_CONTROL_MIN_SPEED     (0x4CCC)

#define MOTOR_DEFAULT_SPEED         (0x0000)

#define MOTOR_DEFAULT_SPIN          (DIVIDE_BY_TEN(TIM3_PERIOD)) // ~10% spin differential
#define MOTOR_DEFAULT_SPIN_MODE     (SPIN_MODE_BACKSPIN)

/* =============================================================================
 *   Private Data Types
 * =============================================================================
 */

typedef struct
{
   uint16_t speed;                  // Speed without spin applied.
   __IO uint16_t topMotorSpeed;     // Controls duty cycle of CCR1 signal
   __IO uint16_t bottomMotorSpeed;  // Controls duty cycle of CCR2 signal
} MOTOR_CONTROL_TYPE;

/* =============================================================================
 *   Public Variables (use extern in .h)
 * =============================================================================
 */

/* =============================================================================
 *   Private Variables (static)
 * =============================================================================
 */

MOTOR_CONTROL_TYPE motor;

/* =============================================================================
 *   Public Constants (use extern in .h)
 * =============================================================================
 */

/* =============================================================================
 *   Private Function Prototypes (static)
 * =============================================================================
 */

static void GPIO_Config(void);
static void TIM3_Config(void);

static void updateMotorSpeed(MOTOR_TYPES motor_to_set);

/* =============================================================================
 *   Private Constants (static)
 * =============================================================================
 */

/* =============================================================================
 *   Extern Functions (use extern in .h)
 * =============================================================================
 */

/* =============================================================================
 *   Public Functions (use extern in .h)
 * =============================================================================
 */

void MotorControl_Init(void)
{
   // Default Motor Settings
   motor.speed = MOTOR_DEFAULT_SPEED;
   motor.topMotorSpeed = MOTOR_DEFAULT_SPEED;
   motor.bottomMotorSpeed = MOTOR_DEFAULT_SPEED;
   
   //Initialize PWM outputs
   GPIO_Config();
   TIM3_Config();
}

void MotorControl_SetSpeed(MOTOR_TYPES motor_to_set, uint16_t percentageTopSpeed)
{
  //Cap the motor speeds
  //We need to scale the percentage between our max and min motor speeds
  //(((percentageTopSpeed - 1)/0xFFFF) * (MaxMotorSpeed - MinMotorSpeed)) + MinMotorSpeed
  //e.g. 1% = (0) + MinMotorSpeed = MinMotorspeed
  uint32_t scaledPercentage = 0; 
  if(percentageTopSpeed > 0)
  {
     scaledPercentage = (uint32_t)((MOTOR_CONTROL_MAX_SPEED - MOTOR_CONTROL_MIN_SPEED) + 1);
     scaledPercentage *= (uint32_t)percentageTopSpeed;
     scaledPercentage = scaledPercentage >> 16;
     scaledPercentage += MOTOR_CONTROL_MIN_SPEED;
  }
  else
  {
    scaledPercentage = 0;
  }
   //(Top Speed / 0xFFFF) * TIM3_Period = topSpeed% * Period
   uint16_t speed = (uint16_t)((uint32_t)(((uint32_t)TIM3_PERIOD + 1) * scaledPercentage) >> 16);

   motor.speed = speed;
   // Update CCRs with updated info
   updateMotorSpeed(motor_to_set);
}

/* =============================================================================
 *   Private Functions (static)
 * =============================================================================
 */

// NOTE: Function assumes that spinOffset < speed
static void updateMotorSpeed(MOTOR_TYPES motor_to_set)
{
   switch (motor_to_set)
   {
      case MOTOR_TYPE_TOP:
         {
            motor.topMotorSpeed = motor.speed;
            TIM3_ClearITPendingBit(TIM3_IT_CC1);
            TIM3_SetCompare1(motor.topMotorSpeed);
         }
         break;
      case MOTOR_TYPE_BOTTOM:
         {
            motor.bottomMotorSpeed = motor.speed;
            TIM3_ClearITPendingBit(TIM3_IT_CC2);
            TIM3_SetCompare2(motor.bottomMotorSpeed);
         }
         break;
      default:
         {
         }
         break;
   }
}

static void GPIO_Config(void)
{
  /* GPIOB configuration: TIM3 channel 1 (PB1) and channel 2 (PB2)*/
  GPIO_Init(TOP_MOTOR_GPIO_PORT, TOP_MOTOR_GPIO_PIN, GPIO_Mode_Out_PP_Low_Fast);
  GPIO_Init(BOTTOM_MOTOR_GPIO_PORT, BOTTOM_MOTOR_GPIO_PIN, GPIO_Mode_Out_PP_Low_Fast);
}

static void TIM3_Config(void)
{
   CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, ENABLE);
   
   /*
  - TIM3CLK = 2 MHz
  - TIM3 counter clock = TIM3CLK / TIM3_PRESCALER+1 = 2 MHz/1+1 = 1 MHz
  */
  /* Time base configuration */
  TIM3_TimeBaseInit((TIM3_Prescaler_TypeDef)TIM3_PRESCALER, TIM3_CounterMode_Up, (uint16_t)TIM3_PERIOD);

  /*
  - The TIM3 CCR1 register value is equal to 32768: 
  - CC1 update rate = TIM3 counter clock / topMotorSpeed = 30.51 Hz,
  - So the TIM3 Channel 1 generates a periodic signal with a frequency equal to 15.25 Hz.
  */
  /* Toggle Mode configuration: Channel1 */
  TIM3_OC1Init(TIM3_OCMode_PWM2, 
               TIM3_OutputState_Enable, 
               motor.topMotorSpeed, 
               TIM3_OCPolarity_Low,  
               TIM3_OCIdleState_Set);
  
  TIM3_OC1PreloadConfig(DISABLE);

  /*
  - The TIM3 CCR2 register is equal to 16384:
  - CC2 update rate = TIM3 counter clock / bottomMotor_CCR1_Val = 61.03 Hz
  - So the TIM3 channel 2 generates a periodic signal with a frequency equal to 30.51 Hz.
  */
  /* Toggle Mode configuration: Channel2 */
  TIM3_OC2Init(TIM3_OCMode_PWM2, 
               TIM3_OutputState_Enable,
               motor.bottomMotorSpeed, 
               TIM3_OCPolarity_Low,  
               TIM3_OCIdleState_Set);
  
  TIM3_OC2PreloadConfig(DISABLE);

  TIM3_ARRPreloadConfig(ENABLE);

  /* Enable TIM3 outputs */
  TIM3_CtrlPWMOutputs(ENABLE);
  /* TIM3 enable counter */
  TIM3_Cmd(ENABLE);
}


/** @} */
