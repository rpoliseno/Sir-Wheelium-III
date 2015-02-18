
#include "stm8l15x.h"

//prototypes to mkae the compiler happy.
extern int __eeprom_wait_for_last_operation(void);
extern void __eeprom_program_byte(unsigned char __near * dst, unsigned char v);
extern void __eeprom_program_long(unsigned char __near * dst, unsigned long v);