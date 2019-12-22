/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  SYS_DELAY_H
#define  SYS_DELAY_H

/* Includes -------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Defines --------------------------------------------------------------------*/


/* Types ----------------------------------------------------------------------*/


/* Variables ------------------------------------------------------------------*/


/* Functions ------------------------------------------------------------------*/
void Delay_Configuration(void);
void delay_ms(vu32 nTime);
void delay_us(vu32 nTime);

#endif /* SYS_DELAY_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
