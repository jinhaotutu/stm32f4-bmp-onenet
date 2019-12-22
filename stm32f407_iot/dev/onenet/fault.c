/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	fault.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		网络错误处理
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//协议
#include "onenet.h"
#include "fault.h"

//网络设备
#include "net_device.h"

//硬件驱动
//#include "usart.h"
//#include "selfcheck.h"

//C库
#include <stdio.h>

CHECK_INFO checkInfo = {DEV_ERR};

unsigned char faultType = FAULT_NONE;			//初始为无错误类型
unsigned char faultTypeReport = FAULT_NONE;		//报告错误类型用的


//==========================================================
//	函数名称：	Fault_Process
//
//	函数功能：	错误处理
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void Fault_Process(void)
{
	
	switch(faultType)
	{
		case FAULT_NONE:
			
			printf("WARN:	FAULT_NONE\r\n");
		
		break;
		
		case FAULT_REBOOT:											//死机重启的处理
			
			printf("WARN:	FAULT_REBOOT\r\n");
		
			faultType = FAULT_NONE;
		
		break;
		
		case FAULT_PRO:												//协议出错的处理
			
			printf("WARN:	FAULT_EDP\r\n");
		
			faultType = FAULT_NONE;
			
			NET_DEVICE_ReLink(oneNetInfo.ip, oneNetInfo.port);
			oneNetInfo.netWork = 0;									//这里设置重连平台
		
		break;
		
		case FAULT_NODEVICE:										//设备出错的处理
			
			printf("WARN:	FAULT_NODEVICE\r\n");
		
			faultType = FAULT_NONE;

			checkInfo.NET_DEVICE_OK = DEV_ERR;
			NET_DEVICE_Reset();
			NET_DEVICE_ReConfig(0);
			
			netDeviceInfo.netWork = 0;
			oneNetInfo.netWork = 0;
		
		break;
		
		default:
			break;
	}

}
