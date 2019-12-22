/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	net_IO.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-08-05
	*
	*	�汾�� 		V1.3
	*
	*	˵���� 		�����豸����IO��
	*
	*	�޸ļ�¼��	V1.1������DMA���͹���
	*				V1.2������DMA���չ��ܡ�IDLE�ж�
	*				V1.3������RingBuffer����
	*						˵����	rbд�룺�ڴ��ڽ������ѭ��д�롣
	*								rb��ȡ������˼������һ֡������
	*										��β��ӽ��������ϲ�Ӧ
	*										�ø��ݽ���������ȡ����
	*										�����ݡ�
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f4xx.h"

//�����豸����IO
#include "net_io.h"

//Ӳ������
#include "sys_delay.h"
//#include "delay.h"
//#include "usart.h"

#include "onenet.h"


NET_IO_INFO netIOInfo = {{0}, {0}, 0, 0, 0};


const unsigned char tag[] = {0, 127, 255};			//�Զ��������
const unsigned char tag_num = sizeof(tag);

/*
************************************************************
*	�������ƣ�	NET_USART_Init
*
*	�������ܣ�	��ʼ�������豸�ײ㴮��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		�ײ�������շ�����
************************************************************
*/
void NET_USART_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStruct;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_IO_WIFI,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_USART_WIFI,ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	/* USART3 GPIO������ */
	GPIO_InitStructure.GPIO_Pin=PIN_WIFI;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIO_WIFI, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIO_WIFI,AF_WIFI_TX,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIO_WIFI,AF_WIFI_RX,GPIO_AF_USART3);

	//USART3 IRQ
	NVIC_InitStruct.NVIC_IRQChannel = WIFI_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	USART_InitStructure.USART_BaudRate = BAUDRATE_WIFI;         
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 
	USART_Init(NET_IO, &USART_InitStructure);

	USART_Cmd(NET_IO,ENABLE);
	
	USART_ITConfig(NET_IO, USART_IT_IDLE, ENABLE);
	USART_ITConfig(NET_IO, USART_IT_RXNE, ENABLE);
}

/*
************************************************************
*	�������ƣ�	NET_IO_Init
*
*	�������ܣ�	��ʼ�������豸IO������
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		�ײ�������շ�����
************************************************************
*/
void NET_IO_Init(void)
{

//	Usart2_Init(115200);
	NET_USART_Init();
#if(USART_DMA_RX_EN == 1)
	USARTx_ResetMemoryBaseAddr(NET_IO, (unsigned int)netIOInfo.buf, RB_BUFFER_SIZE, USART_RX_TYPE);
#endif

}

/*
************************************************************
*	�������ƣ�	NET_IO_AddTag2End
*
*	�������ܣ�	��ÿһ֡����ĩβ�����Զ���Ľ�����
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		��ӽ��������Է����ϴ�����һ֡��������
************************************************************
*/
void NET_IO_AddTag2End(void)
{

	unsigned char i = 0;
	
	for(; i < tag_num; i++)								//��һ֡����β��ӽ�����
	{
		netIOInfo.buf[netIOInfo.write_pos++] = tag[i];
		netIOInfo.write_pos %= RB_BUFFER_SIZE;
	}
	
#if(USART_DMA_RX_EN == 1)
	DMA_Cmd(DMA1_Channel6, DISABLE);												//�ر�DMA

	DMA_SetCurrDataCounter(DMA1_Channel6, RB_BUFFER_SIZE - netIOInfo.write_pos);	//��������ʣ���ֽ�
	DMA1_Channel6->CMAR = (unsigned int)(netIOInfo.buf + netIOInfo.write_pos);		//�������õ�ǰDMAָ���ڴ��ָ��
	
	DMA_Cmd(DMA1_Channel6, ENABLE);													//ʹ��MDA
#endif
	
	netIOInfo.data_packet++;							//���ݰ�ָ������

}

/*
************************************************************
*	�������ƣ�	NET_IO_CheckTag
*
*	�������ܣ�	��ȡ������һ֡���ݴ�С
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		���ݽ��������ж�һ֡���ݵĳ���
************************************************************
*/
unsigned short NET_IO_CheckTag(void)
{
	
	unsigned short bytes = 0, read_pos = netIOInfo.read_pos;
	unsigned char i = 0;
	
	while(i < tag_num)											//���δ�ҵ�����������
	{
		if(netIOInfo.buf[read_pos] == tag[i])					//�ҵ��˵�һ��
			i++;												//���
		else													//����ͽ�������ƥ��
		{
			i = 0;												//��ͷ���
			if(netIOInfo.buf[read_pos] == tag[i])				//�Ƚϵ�ǰ����ǲ��ǵ�һ��������
				i++;
		}
		
		bytes++;
		read_pos++;
		
		bytes %= RB_BUFFER_SIZE;								//���������鷶Χ
		read_pos %= RB_BUFFER_SIZE;
	}
	
	return bytes;

}

/*
************************************************************
*	�������ƣ�	NET_IO_Send
*
*	�������ܣ�	��������
*
*	��ڲ�����	str����Ҫ���͵�����
*				len�����ݳ���
*
*	���ز�����	��
*
*	˵����		�ײ�����ݷ�������
*
************************************************************
*/
void NET_IO_Send(unsigned char *str, unsigned short len)
{
	
#if(USART_DMA_TX_EN == 0)
	unsigned short count = 0;
	
	for(; count < len; count++)
	{
		USART_SendData(NET_IO, *str++);									//��������
		while(USART_GetFlagStatus(NET_IO, USART_FLAG_TC) == RESET);		//�ȴ��������
	}
#else
	unsigned int mAddr = (unsigned int)str;
	
	while(DMA_GetFlagStatus(DMA1_FLAG_TC7) == RESET);					//�ȴ�ͨ��7�������
	DMA_ClearFlag(DMA1_FLAG_TC7);										//���ͨ��7������ɱ�־
	
	USARTx_ResetMemoryBaseAddr(NET_IO, mAddr, len, USART_TX_TYPE);
#endif

}

/*
************************************************************
*	�������ƣ�	NET_IO_Read
*
*	�������ܣ�	��ȡ���յ�����
*
*	��ڲ�����	��
*
*	���ز�����	��ȡ��֡����ָ��
*
*	˵����		
************************************************************
*/
unsigned char *NET_IO_Read(void)
{
	
	unsigned short read_num = 0, i = 0;
	unsigned char *buf = (void *)0;
	
	if(netIOInfo.data_packet > 0)													//�����������ݰ��ɶ�ʱ
	{
		if(--netIOInfo.data_packet == 65535)
			netIOInfo.data_packet = 0;
		
		read_num = NET_IO_CheckTag();												//������Ҫ��ȡ�����ݳ���
		
		if(netIOInfo.write_pos <= netIOInfo.read_pos)								//�������ݡ���ѭ����һȦ*********************
		{
			if(netIOInfo.read_pos + read_num < RB_BUFFER_SIZE)						//��ǰ��֡���ݻ�δ����rb
			{
				buf = netIOInfo.buf + netIOInfo.read_pos;
			}
			else																	//�������ǰ�γ�����rbβ����γ�����rb��
			{
				unsigned short num = RB_BUFFER_SIZE - netIOInfo.read_pos;			//����rbβ��Ҫ�������ݳ���
				
				if(read_num >= RB_BUFFER2_SIZE)										//������ڶ�����������
					return (void *)0;
				
				for(i = 0; i < num; i++)											//����rbβ����Ч���ݵ���������
					netIOInfo.buf_tp[i] = netIOInfo.buf[i + netIOInfo.read_pos];
				
				for(i = 0; i < read_num - num; i++)									//����rb�׵���Ч���ݵ���������
					netIOInfo.buf_tp[i + num] = netIOInfo.buf[i];
				
				buf = netIOInfo.buf_tp;
			}
		}
		else																		//�������ݡ��һ�δѭ��һȦ*******************
		{
			buf = netIOInfo.buf + netIOInfo.read_pos;
		}
		
		netIOInfo.read_pos += read_num;												//�ƶ���ָ��
		netIOInfo.read_pos %= RB_BUFFER_SIZE;										//ȷ����ָ�벻������rb��Χ
	}
	
	return buf;

}

/*
************************************************************
*	�������ƣ�	DMA1_Channel6_IRQHandler
*
*	�������ܣ�	DMA1_ͨ��6_�жϺ���
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
//void DMA1_Channel6_IRQHandler(void)
//{

//	if(DMA_GetITStatus(DMA1_IT_TC6) == SET)
//	{

//#if(USART_DMA_RX_EN == 1)
//		netIOInfo.write_pos = 0;
//		USARTx_ResetMemoryBaseAddr(NET_IO, (unsigned int)netIOInfo.buf, RB_BUFFER_SIZE, USART_RX_TYPE);
//#endif
//		DMA_ClearFlag(DMA1_IT_TC6);
//	}

//}

/*
************************************************************
*	�������ƣ�	USART3_IRQHandler
*
*	�������ܣ�	USART3�ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void USART3_IRQHandler(void)
{
	
#if(USART_DMA_RX_EN == 0)
	if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != RESET)			//�����ж�
	{
		USART_ClearFlag(USART3, USART_FLAG_RXNE);
		
		netIOInfo.buf[netIOInfo.write_pos++] = USART3->DR;				//д��rb
		netIOInfo.write_pos %= RB_BUFFER_SIZE;							//������rb��Χ��
	}
#endif
	
	if(USART_GetFlagStatus(USART3, USART_FLAG_IDLE) != RESET)			//�����ж�
	{
		USART3->DR;														//��ȡ����ע�⣺������Ҫ�������ܹ�����жϱ�־λ
		USART_ClearFlag(USART3, USART_IT_IDLE);
		
#if(USART_DMA_RX_EN == 1)
		netIOInfo.write_pos = RB_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);
#endif
		
		NET_IO_AddTag2End();											//�յ�һ֡�������ݣ�������β��ӽ�����
		OneNET_CmdHandle();
	}
}

/************** ������� **************/
#include "sys_cfg.h"
#include "dcmi.h"

#define LED1_TOGGLE GPIO_ToggleBits(GPIOE,GPIO_Pin_0);
#define LED2_TOGGLE GPIO_ToggleBits(GPIOE,GPIO_Pin_1);

#define NET_TIME_DELAY	180  //180s���ݼ��

u16 net_send_time = 0;

void TIM3_IRQHandler(void)
{
	//���жϱ�ʶ
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	//---------------- �жϴ���  ------------------//
	if(oneNetInfo.netWork == 1)
	{
		if (oneNetInfo.sendData != SEND_TYPE_PICTURE)
		{
			net_send_time++;
		}
		
		if (net_send_time == 1)
		{
			oneNetInfo.sendData = SEND_TYPE_DATA;
			LED2_TOGGLE;
		}
		else if ((net_send_time % 25) == 0)   //ÿ25s������������
		{
			oneNetInfo.sendData = SEND_TYPE_HEART;
			LED2_TOGGLE;
		}
		else if (net_send_time == NET_TIME_DELAY/2)
		{
			if (oneNetInfo.sendData == SEND_TYPE_OK)
			{
				DCMI_Start();
				LED2_TOGGLE;
			}
		}
		else if (net_send_time == NET_TIME_DELAY)
		{
			net_send_time = 0;
		}
	}
	
	LED1_TOGGLE;
	OneNet_Check_Heart();
}  
