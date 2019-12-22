/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	net_IO.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-08-05
	*
	*	版本： 		V1.3
	*
	*	说明： 		网络设备数据IO层
	*
	*	修改记录：	V1.1：增加DMA发送功能
	*				V1.2：增加DMA接收功能、IDLE中断
	*				V1.3：引入RingBuffer机制
	*						说明：	rb写入：在串口接收里边循环写入。
	*								rb读取：核心思想是在一帧完整数
	*										据尾添加结束符，上层应
	*										用根据结束符来获取完整
	*										的数据。
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f4xx.h"

//网络设备数据IO
#include "net_io.h"

//硬件驱动
#include "sys_delay.h"
//#include "delay.h"
//#include "usart.h"

#include "onenet.h"


NET_IO_INFO netIOInfo = {{0}, {0}, 0, 0, 0};


const unsigned char tag[] = {0, 127, 255};			//自定义结束符
const unsigned char tag_num = sizeof(tag);

/*
************************************************************
*	函数名称：	NET_USART_Init
*
*	函数功能：	初始化网络设备底层串口
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		底层的数据收发驱动
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
	
	/* USART3 GPIO口配置 */
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
*	函数名称：	NET_IO_Init
*
*	函数功能：	初始化网络设备IO驱动层
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		底层的数据收发驱动
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
*	函数名称：	NET_IO_AddTag2End
*
*	函数功能：	在每一帧数据末尾加上自定义的结束符
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		添加结束符，以方便上传处理一帧完整数据
************************************************************
*/
void NET_IO_AddTag2End(void)
{

	unsigned char i = 0;
	
	for(; i < tag_num; i++)								//在一帧数据尾添加结束符
	{
		netIOInfo.buf[netIOInfo.write_pos++] = tag[i];
		netIOInfo.write_pos %= RB_BUFFER_SIZE;
	}
	
#if(USART_DMA_RX_EN == 1)
	DMA_Cmd(DMA1_Channel6, DISABLE);												//关闭DMA

	DMA_SetCurrDataCounter(DMA1_Channel6, RB_BUFFER_SIZE - netIOInfo.write_pos);	//重新设置剩余字节
	DMA1_Channel6->CMAR = (unsigned int)(netIOInfo.buf + netIOInfo.write_pos);		//重新设置当前DMA指向内存的指针
	
	DMA_Cmd(DMA1_Channel6, ENABLE);													//使能MDA
#endif
	
	netIOInfo.data_packet++;							//数据包指针增加

}

/*
************************************************************
*	函数名称：	NET_IO_CheckTag
*
*	函数功能：	获取完整的一帧数据大小
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		根据结束符来判断一帧数据的长度
************************************************************
*/
unsigned short NET_IO_CheckTag(void)
{
	
	unsigned short bytes = 0, read_pos = netIOInfo.read_pos;
	unsigned char i = 0;
	
	while(i < tag_num)											//如果未找到完整结束符
	{
		if(netIOInfo.buf[read_pos] == tag[i])					//找到了第一个
			i++;												//标记
		else													//如果和结束符不匹配
		{
			i = 0;												//从头检查
			if(netIOInfo.buf[read_pos] == tag[i])				//比较当前这个是不是第一个结束符
				i++;
		}
		
		bytes++;
		read_pos++;
		
		bytes %= RB_BUFFER_SIZE;								//不超过数组范围
		read_pos %= RB_BUFFER_SIZE;
	}
	
	return bytes;

}

/*
************************************************************
*	函数名称：	NET_IO_Send
*
*	函数功能：	发送数据
*
*	入口参数：	str：需要发送的数据
*				len：数据长度
*
*	返回参数：	无
*
*	说明：		底层的数据发送驱动
*
************************************************************
*/
void NET_IO_Send(unsigned char *str, unsigned short len)
{
	
#if(USART_DMA_TX_EN == 0)
	unsigned short count = 0;
	
	for(; count < len; count++)
	{
		USART_SendData(NET_IO, *str++);									//发送数据
		while(USART_GetFlagStatus(NET_IO, USART_FLAG_TC) == RESET);		//等待发送完成
	}
#else
	unsigned int mAddr = (unsigned int)str;
	
	while(DMA_GetFlagStatus(DMA1_FLAG_TC7) == RESET);					//等待通道7传输完成
	DMA_ClearFlag(DMA1_FLAG_TC7);										//清除通道7传输完成标志
	
	USARTx_ResetMemoryBaseAddr(NET_IO, mAddr, len, USART_TX_TYPE);
#endif

}

/*
************************************************************
*	函数名称：	NET_IO_Read
*
*	函数功能：	读取接收的数据
*
*	入口参数：	无
*
*	返回参数：	获取本帧数据指针
*
*	说明：		
************************************************************
*/
unsigned char *NET_IO_Read(void)
{
	
	unsigned short read_num = 0, i = 0;
	unsigned char *buf = (void *)0;
	
	if(netIOInfo.data_packet > 0)													//当有完整数据包可读时
	{
		if(--netIOInfo.data_packet == 65535)
			netIOInfo.data_packet = 0;
		
		read_num = NET_IO_CheckTag();												//计算需要读取的数据长度
		
		if(netIOInfo.write_pos <= netIOInfo.read_pos)								//有新数据、且循环了一圈*********************
		{
			if(netIOInfo.read_pos + read_num < RB_BUFFER_SIZE)						//当前这帧数据还未超过rb
			{
				buf = netIOInfo.buf + netIOInfo.read_pos;
			}
			else																	//如果数据前段出现在rb尾，后段出现在rb首
			{
				unsigned short num = RB_BUFFER_SIZE - netIOInfo.read_pos;			//计算rb尾需要读的数据长度
				
				if(read_num >= RB_BUFFER2_SIZE)										//如果大于二级缓存容量
					return (void *)0;
				
				for(i = 0; i < num; i++)											//复制rb尾的有效数据到二级缓存
					netIOInfo.buf_tp[i] = netIOInfo.buf[i + netIOInfo.read_pos];
				
				for(i = 0; i < read_num - num; i++)									//复制rb首的有效数据到二级缓存
					netIOInfo.buf_tp[i + num] = netIOInfo.buf[i];
				
				buf = netIOInfo.buf_tp;
			}
		}
		else																		//有新数据、且还未循环一圈*******************
		{
			buf = netIOInfo.buf + netIOInfo.read_pos;
		}
		
		netIOInfo.read_pos += read_num;												//移动读指针
		netIOInfo.read_pos %= RB_BUFFER_SIZE;										//确保读指针不超过该rb范围
	}
	
	return buf;

}

/*
************************************************************
*	函数名称：	DMA1_Channel6_IRQHandler
*
*	函数功能：	DMA1_通道6_中断函数
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	USART3_IRQHandler
*
*	函数功能：	USART3中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void USART3_IRQHandler(void)
{
	
#if(USART_DMA_RX_EN == 0)
	if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != RESET)			//接收中断
	{
		USART_ClearFlag(USART3, USART_FLAG_RXNE);
		
		netIOInfo.buf[netIOInfo.write_pos++] = USART3->DR;				//写入rb
		netIOInfo.write_pos %= RB_BUFFER_SIZE;							//控制在rb范围内
	}
#endif
	
	if(USART_GetFlagStatus(USART3, USART_FLAG_IDLE) != RESET)			//空闲中断
	{
		USART3->DR;														//读取数据注意：这句必须要，否则不能够清除中断标志位
		USART_ClearFlag(USART3, USART_IT_IDLE);
		
#if(USART_DMA_RX_EN == 1)
		netIOInfo.write_pos = RB_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);
#endif
		
		NET_IO_AddTag2End();											//收到一帧完整数据，在数据尾添加结束符
		OneNET_CmdHandle();
	}
}

/************** 额外添加 **************/
#include "sys_cfg.h"
#include "dcmi.h"

#define LED1_TOGGLE GPIO_ToggleBits(GPIOE,GPIO_Pin_0);
#define LED2_TOGGLE GPIO_ToggleBits(GPIOE,GPIO_Pin_1);

#define NET_TIME_DELAY	180  //180s数据间隔

u16 net_send_time = 0;

void TIM3_IRQHandler(void)
{
	//清中断标识
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	//---------------- 中断处理  ------------------//
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
		else if ((net_send_time % 25) == 0)   //每25s发送心跳请求
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
