/**
  ******************************************************************************
  * @file    CLK/CLK_SYSCLKSwitch/main.c
  * @author  MCD Application Team
  * @version V1.5.2
  * @date    30-September-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "stm8l1526_eval.h"
#include "stm8l1526_eval_glass_lcd.h"
#include "timing_delay.h"
#include "servo_control.h"
#include "Bluetooth.h"
#include "Protocol.h"

/** @addtogroup STM8L15x_StdPeriph_Examples
  * @{
  */

/** @addtogroup CLK_SYSCLKSwitch
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void CLK_Config(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
   /* CLK configuration -------------------------------------------*/
  CLK_Config(); 

  /* Init TIM2 to generate 1 ms time base update interrupt */
  TimingDelay_Init();

  /* Init the servo control module */
  ServoModule_Init();
  
  /* Enable Interrupts */
  enableInterrupts();

  /* Initialize LEDs mounted on STM8L152X-EVAL board */
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);

  /* Initialize push-buttons mounted on STM8L152X-EVAL board */
//  STM_EVAL_PBInit(BUTTON_RIGHT, BUTTON_MODE_EXTI);
//  STM_EVAL_PBInit(BUTTON_LEFT, BUTTON_MODE_EXTI);
//  STM_EVAL_PBInit(BUTTON_UP, BUTTON_MODE_EXTI);
//  STM_EVAL_PBInit(BUTTON_DOWN, BUTTON_MODE_EXTI);
  
   Bluetooth__Initialize();
   /* Enable general interrupts */
   enableInterrupts();
  

  while (1)
  {
    /* Toggle LED4 */
    STM_EVAL_LEDToggle(LED4);
    Delay(1);
    Protocol__ReceivedCommand(Bluetooth__GetCommand());
      
   }
 }


/**
  * @brief  Configure System Clock 
  * @param  None
  * @retval None
  */
static void CLK_Config(void)
{
  /* Select HSE as system clock source */
  CLK_SYSCLKSourceSwitchCmd(ENABLE);
  CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
  /* system clock prescaler: 1*/
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
  while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSI)
  {}
}
#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* Infinite loop */
  while (1)
  {}
}

/**
  * @}
  */

/**
  * @}
  */

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
