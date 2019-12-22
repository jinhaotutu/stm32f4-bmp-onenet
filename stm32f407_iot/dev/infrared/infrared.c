/**
  ******************************************************************************
  * @file    infrared.c 
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
  
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdio.h>

#include "infrared.h"
#include "sys_cfg.h"

/* Defines --------------------------------------------------------------------*/


/* Variables ------------------------------------------------------------------*/


/* Functions ------------------------------------------------------------------*/
void Infrared_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(Infrared_RCC,ENABLE); 
	
	/* 使能 SYSCFG 时钟 ，使用GPIO外部中断时必须使能SYSCFG时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitStructure.GPIO_Pin=Infrared_Pin;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_DOWN;
    GPIO_Init(Infrared_GPIO, &GPIO_InitStructure);
}

void Infrared_EXTI_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	
	/* 连接 EXTI 中断源 到INT1引脚 */
	SYSCFG_EXTILineConfig(Infrared_Exti_Port,Infrared_Exti_Pin);
	
	/* 选择 EXTI 中断源 */
	EXTI_InitStructure.EXTI_Line = Infrared_Exti_Line;
	/* 中断模式 */
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	/* 上升沿下降沿触发 */
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
	/* 使能中断/事件线 */
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

void Infrared_NVIC_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* 配置中断源1 */
  	NVIC_InitStruct.NVIC_IRQChannel = Infrared_IRQn;
  	/* 配置抢占优先级：1 */
  	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  	/* 配置子优先级：1 */
  	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  	/* 使能中断通道 */
  	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStruct);
}

void Infrared_Init(void)
{
	Infrared_GPIO_Init();
	Infrared_EXTI_Init();
	Infrared_NVIC_Init();
}

u16 Infrared_Count = 0;

void Infrared_IRQHandler(void)
{
  	//确保是否产生了EXTI Line中断
	if(EXTI_GetITStatus(Infrared_Exti_Line) != RESET) 
	{
		//检测是否产生红外感应，高电平产生，低电平退出感应
		if (Infrared_Read)
		{
			/*******************在此添加红外中断操作*******************/
			printf("\r\nInto The Infrared.\r\n");
			printf("Infrared test int count : %dp.\r\n", ++Infrared_Count);
			LED3_ON;
			/*******************在此添加红外中断操作*******************/
		}
		else
		{
			//退出红外
			printf("\r\nExit The Infrared.\r\n");
			LED3_OFF;
		}
		
		//清除中断标志位
		EXTI_ClearITPendingBit(Infrared_Exti_Line);     
	}  
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

