/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  OV7670TEST_H
#define  OV7670TEST_H

/* Includes -------------------------------------------------------------------*/
#include "stm32f4xx.h"


/* Defines --------------------------------------------------------------------*/


/* Types ----------------------------------------------------------------------*/


/* Variables ------------------------------------------------------------------*/


/* Functions ------------------------------------------------------------------*/
void OV7670_USART_Init(void);
void ShanWai_SendCamera(u16 *camera_buf, u16 length_w, u16 length_h);

#endif /* OV7670TEST_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
