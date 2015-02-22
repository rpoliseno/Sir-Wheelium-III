#ifndef BLUETOOTH_H
#define BLUETOOTH_H

//Includes
#include "stm8l15x.h"


//Structs
typedef struct
{
  uint8_t CMD;
  uint8_t DATA[5];
}SIR_WHEELIUM_CMD;


//////
/////Fucntions
////

extern void Bluetooth__Initialize(void);
extern void Bluetooth__RxInterrupt(void);
extern SIR_WHEELIUM_CMD Bluetooth__GetCommand(void);


#endif