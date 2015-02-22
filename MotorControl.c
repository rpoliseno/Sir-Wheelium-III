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
#define MOTOR_CONTROL_MIN_SPEED     (0x0000)
//Minimum Motor speed is 30% of 0xFFFF
//#define MOTOR_CONTROL_MIN_SPEED     (0x4CCC)

#define MOTOR_DEFAULT_SPEED         (MOTOR_CONTROL_MIN_SPEED)
///This is only 10% at HE - DEREK
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
   uint16_t spinOffset;             // Defines offset between CCR1 and CCR2
   MOTOR_CONTROL_SPIN_MODE spinMode; 
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

static void updateMotorSpeed(void);

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
   motor.topMotorSpeed = MOTOR_DEFAULT_SPEED;
   motor.bottomMotorSpeed = MOTOR_DEFAULT_SPEED;
   motor.spinOffset = MOTOR_DEFAULT_SPIN;
   motor.spinMode = MOTOR_DEFAULT_SPIN_MODE;
   
   //Initialize PWM outputs
   GPIO_Config();
   TIM3_Config();
}

void MotorControl_SetSpeed(uint16_t percentageTopSpeed)
{
  //Cap the motor speeds
  //DO NOT remove these. They may be pointless with 0 and 0xFFFF but we need to protect ourselves
  //  incase we change these values. 
   if(percentageTopSpeed < MOTOR_CONTROL_MIN_SPEED)
   {
     percentageTopSpeed = MOTOR_CONTROL_MIN_SPEED;
   }
   else if(percentageTopSpeed > MOTOR_CONTROL_MAX_SPEED)
   {
     percentageTopSpeed = MOTOR_CONTROL_MAX_SPEED;
   }
   
   //(Top Speed / 0xFFFF) * TIM3_Period = topSpeed% * Period
   uint16_t speed = (uint16_t)((uint32_t)(((uint32_t)TIM3_PERIOD + 1) * (uint32_t)percentageTopSpeed) >> 16);
   
   //Update the spin ofset. Remember it is a percentage
   motor.spinOffset = (DIVIDE_BY_TEN(speed));
   // bounds check top speed with spin offset to ensure it has not been set too low
   if(motor.spinOffset > speed)
   {
      speed = motor.spinOffset + 1;
   }  
   motor.speed = speed + 1;
   // Update CCRs with updated info
   updateMotorSpeed();
}

void MotorControl_SetSpin(uint16_t percentageTopSpin)
{
   // bounds check top speed with spin offset
   if (percentageTopSpin > motor.speed)
   {
      motor.speed = percentageTopSpin + 1;
   }
   motor.spinOffset = percentageTopSpin;
   // Update CCRs with updated info
   updateMotorSpeed();
}

void MotorControl_SetSpinMode(MOTOR_CONTROL_SPIN_MODE mode)
{
   if (mode < SPIN_MODE_NUM_SPIN_MODES)
   {
      motor.spinMode = mode;
   }
   // Update CCRs with updated info
   updateMotorSpeed();
}

/* =============================================================================
 *   Private Functions (static)
 * =============================================================================
 */

// NOTE: Function assumes that spinOffset < speed
static void updateMotorSpeed(void)
{
   // Apply spin offset depending on spin mode
   switch (motor.spinMode)
   {
      case SPIN_MODE_BACKSPIN:
      {
         motor.bottomMotorSpeed = motor.speed;
         motor.topMotorSpeed = motor.speed - motor.spinOffset;
      }
      break;
      case SPIN_MODE_TOPSPIN:
      {
         motor.topMotorSpeed = motor.speed;
         motor.bottomMotorSpeed = motor.speed - motor.spinOffset;
      }
      break;
      case SPIN_MODE_NEUTRAL:
      {
         motor.topMotorSpeed = motor.speed;
         motor.bottomMotorSpeed = motor.speed;
      }
      break;
      default:
      {
         // Shit's fucked.
      }
      break;      
   }
   
   // Set the CCR registers to update speed on each motor
   TIM3_ClearITPendingBit(TIM3_IT_CC1);
   TIM3_SetCompare1(motor.topMotorSpeed);
   TIM3_ClearITPendingBit(TIM3_IT_CC2);
   TIM3_SetCompare2(motor.bottomMotorSpeed);
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
