/**
  ******************************************************************************
  * @file    Bluetooth.c
  * @author  Derek Thrasher
  * @version V0.1
  * @date    21-February-2015
  * @brief   Bluetooth Module
  ******************************************************************************

  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "stm8_eval.h"



void Bluetooth__Initialize()
{
  STM_EVAL_COMInit(COM_TypeDef.Com1,38400,USART_WordLength_TypeDef.USART_WordLength_8b,
                      USART_StopBits_TypeDef.USART_StopBits_1 ,
                      USART_Parity_TypeDef.USART_Parity_No,
                      USART_Mode_TypeDef.USART_Mode_Rx));
}