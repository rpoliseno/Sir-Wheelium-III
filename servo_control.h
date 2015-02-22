/**
  ******************************************************************************
  * @file    servo_control.h
  * @author  Mike Barber
  * @date    21-February-2015
  * @brief   
  ******************************************************************************
*/
#ifndef __SERVO_CONTROL_H
#define __SERVO_CONTROL_H

/* Includes ------------------------------------------------------------------*/
#include "Sir_Wheelium_III.h"

/* Public datatypes ----------------------------------------------------------*/
typedef enum
{
   SERVO_LOADING = 0,
   SERVO_FIRING,
   SERVO_HORIZONTAL_AIM,
   SERVO_VERTICLE_AIM,
   NUM_SERVO_MOTORS
} SERVO_NAME;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public function prototypes ------------------------------------------------*/
extern void ServoModule_Init(void);
extern bool ServoModule_SetServoAngle(UINT8 servo, UINT16 angle);
extern void ServoModule_DeassertServoOutputs(UINT8 captureCompareNumTriggered);
extern void ServoModule_AssertServoOutputs(void);

#endif /* __SERVO_CONTROL_H */