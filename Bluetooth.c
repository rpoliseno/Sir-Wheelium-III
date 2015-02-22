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
#include "stm8l1526_eval.h"
#include "stm8l15x_usart.h"
#include "Bluetooth.h"

//local array accessed globally through GET
#define COMMAND_FIFO_SIZE (32)
#define COMMAND_FIFO_BIT_MASK (0x1F)
#define COMMAND_FIFO_START_INDEX (0)

static SIR_WHEELIUM_CMD CommandFIFO[COMMAND_FIFO_SIZE];
static uint8_t CommandFIFOReadIndex = 0;
static uint8_t CommandFIFOWriteIndex = 0;

/*
*\brief Initialize the Bluetooth module with USART1
*/
void Bluetooth__Initialize()
{
  STM_EVAL_COMInit(COM1,9600,USART_WordLength_8b,
                      USART_StopBits_1 ,
                      USART_Parity_No,
                      (USART_Mode_Rx));
   //
   // We are configured for buffered output so enable the master interrupt
   // for this UART and the receive interrupts.  We don't actually enable the
   // transmit interrupt in the UART itself until some data has been placed
   // in the transmit buffer.
   //
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  //IntEnable(INT_UART3);
}

/*
*\brief Returns a command if there is one that needs to be proccessed
*/
SIR_WHEELIUM_CMD Bluetooth__GetCommand()
{
  SIR_WHEELIUM_CMD CMD = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  if(CommandFIFOReadIndex != CommandFIFOWriteIndex)
  {
    CMD = CommandFIFO[CommandFIFOReadIndex++];
    STM_EVAL_LEDOn(LED3);
  }
  else
  {
    //We have processed all of our commands! YAY we are fast!!!
    /* Toggle LED3 */
    STM_EVAL_LEDOff(LED3);
  }
  //Wrap the index back to COMMAND_FIFO_START_INDEX
  CommandFIFOReadIndex &= COMMAND_FIFO_BIT_MASK;
  return CMD;
}

/*
*\brief Adding command to FIFO
*/
void addCommandToFifo(SIR_WHEELIUM_CMD addedCMD)
{
   //Don't add command if you are going to overwrite the next command
   if(((CommandFIFOWriteIndex+1) & COMMAND_FIFO_BIT_MASK) != CommandFIFOReadIndex)
   {
   CommandFIFO[CommandFIFOWriteIndex++] = addedCMD;
   }
   else
   {
    //ERROR Our Command Fifo is about to overwrite our next command
   }
   //wrap the index back to COMMAND_FIFO_START_INDEX
   CommandFIFOWriteIndex &= COMMAND_FIFO_BIT_MASK;
}

/*
*\brief Process the interrupt and add command to the FIFO once you have recieve FIXED Length bytes (6)
*/
void Bluetooth__RxInterrupt()
{
   uint8_t readData;
   static uint8_t RxIndex = 0;
   static SIR_WHEELIUM_CMD RxCommand;

   //Received Byte
   if(USART_GetITStatus(USART1, USART_IT_RXNE))
   {
      USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
      //read 1 byte and fill in the SIR_WHEELIAM_CMD
      readData = USART_ReceiveData8(USART1);
      switch(RxIndex)
      {
         case 0:
         {
            RxCommand.CMD = readData;
            break;
         }
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
         {
            RxCommand.DATA[RxIndex - 1] = readData;
            //Lets reset the Index on the last byte of the comand
            //Add command to the FIFO

            break;
         }
         default:
         {
            RxIndex = 0;
            break;
         }
      }
      
      if(RxIndex >= 5)
      {
         RxIndex = 0;
         addCommandToFifo(RxCommand);
      }
      else
      {
         RxIndex++;
      }
       
   }
   if(USART_GetITStatus(USART1, USART_IT_IDLE))
   {
      USART_ClearITPendingBit(USART1,USART_IT_IDLE);
      RxIndex = 0;
      USART_ITConfig(USART1, USART_IT_IDLE, DISABLE);  
   }
}