#ifndef __UART4_H
#define __UART4_H

#include "system.h"

void UART4_Init( void );
void UART4_SendChar( uint8_t cData );
void UART4_DMA_SentData(uint8_t *cData , int size);




/*
A5 00 90 00 00 8E CD 18 45 A4 45 7B 45 AB 4D
A5 00 90 00 00 2B 56 1D 45 8D 68 7B 00 00 00
A5 00 90 00 00 68 40 1E 45 AF 62 7B 45 7A B7
*/


#endif



