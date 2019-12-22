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
	
	/* ʹ�� SYSCFG ʱ�� ��ʹ��GPIO�ⲿ�ж�ʱ����ʹ��SYSCFGʱ��*/
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
	
	/* ���� EXTI �ж�Դ ��INT1���� */
	SYSCFG_EXTILineConfig(Infrared_Exti_Port,Infrared_Exti_Pin);
	
	/* ѡ�� EXTI �ж�Դ */
	EXTI_InitStructure.EXTI_Line = Infrared_Exti_Line;
	/* �ж�ģʽ */
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	/* �������½��ش��� */
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
	/* ʹ���ж�/�¼��� */
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

void Infrared_NVIC_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* �����ж�Դ1 */
  	NVIC_InitStruct.NVIC_IRQChannel = Infrared_IRQn;
  	/* ������ռ���ȼ���1 */
  	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  	/* ���������ȼ���1 */
  	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  	/* ʹ���ж�ͨ�� */
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
  	//ȷ���Ƿ������EXTI Line�ж�
	if(EXTI_GetITStatus(Infrared_Exti_Line) != RESET) 
	{
		//����Ƿ���������Ӧ���ߵ�ƽ�������͵�ƽ�˳���Ӧ
		if (Infrared_Read)
		{
			/*******************�ڴ���Ӻ����жϲ���*******************/
			printf("\r\nInto The Infrared.\r\n");
			printf("Infrared test int count : %dp.\r\n", ++Infrared_Count);
			LED3_ON;
			/*******************�ڴ���Ӻ����жϲ���*******************/
		}
		else
		{
			//�˳�����
			printf("\r\nExit The Infrared.\r\n");
			LED3_OFF;
		}
		
		//����жϱ�־λ
		EXTI_ClearITPendingBit(Infrared_Exti_Line);     
	}  
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

