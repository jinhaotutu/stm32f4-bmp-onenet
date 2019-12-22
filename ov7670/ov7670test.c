/**
  ******************************************************************************
  * @file    ov7670test.c 
  * @author  jinhao
  * @version V1.0.0
  * @date    22-April-2016
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
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
  
/* Includes -------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "sys_cfg.h"
#include "sys_delay.h"

#include "ov7670.h"

/* Defines --------------------------------------------------------------------*/


/* Variables ------------------------------------------------------------------*/
				
/* Functions ------------------------------------------------------------------*/			

void USART_SendByre(USART_TypeDef* USARTx, u8 data)
{
	USART_SendData(USARTx, data);
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
}

void USART_Send2Byre(USART_TypeDef* USARTx, u16 data)
{
	USART_SendByre(USART2, (u8)data);
	USART_SendByre(USART2, (u8)(data>>8));
}

void OV7670_USART_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;	
   
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	/* USART2 GPIOø⁄≈‰÷√ */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);
	
	USART_InitStructure.USART_BaudRate = 256000;         
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 
	USART_Init(USART2, &USART_InitStructure);

	USART_Cmd(USART2,ENABLE);
	
	USART_Send2Byre(USART2, 0);
}

void ShanWai_SendCamera(u16 *camera_buf, u16 length_w, u16 length_h)
{
	u16 i=0;

	USART_Send2Byre(USART2, 0xFE01);

	for (i=0;i<length_w*length_h;i++)
	{
		USART_Send2Byre(USART2, camera_buf[i]);
	}

	USART_Send2Byre(USART2, 0x01FE);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
