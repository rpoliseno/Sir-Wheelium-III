/**
 * @file MotorControl.h
 * @brief Interface for motor driver
 * @copyright (C)2015 Lutron Electronics Co., Inc.
 *    All rights reserved.
 *    The copyright notice above does not evidence any actual or intended
 *    publication of such source code. This file and the information contained
 *    herein are confidential and proprietary to Lutron Electronics Co., Inc.
 *    Unauthorized possession or use of this file or the information contained
 *    herein is prohibited. No reproduction may be made of this file without
 *    the express written permission of Lutron Electronics Co., Inc.
 *
 * @defgroup UIPanelList List of panels for the DTIW product
 * @brief List of UI panels that can be switched between for product operation
 * @{
 */
#ifndef UIPANELLIST_H_
#define UIPANELLIST_H_

/* =============================================================================
 *   Include Files
 * =============================================================================
 */

/* =============================================================================
 *   Public Defines and Macros
 * =============================================================================
 */

/* =============================================================================
 *   Public Data Types
 * =============================================================================
 */
/**
 * @brief Different spin mode configurations
 */
typedef enum
{
   MOTOR_TYPE_TOP = 0,
   MOTOR_TYPE_BOTTOM,
   NUM_MOTOR_TYPES
} MOTOR_TYPES;

/* =============================================================================
 *   Public Constants
 * =============================================================================
 */

/* ============================================================================
 *   Public Function Prototypes
 * =============================================================================
 */

void MotorControl_Init(void);
void MotorControl_SetSpeed(MOTOR_TYPES motor_to_change, uint16_t percentageTopSpeed);


/** @} */
#endif /*UIPANELLIST_H_*/
