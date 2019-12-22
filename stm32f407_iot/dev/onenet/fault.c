/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	fault.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2016-11-23
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		���������
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//Э��
#include "onenet.h"
#include "fault.h"

//�����豸
#include "net_device.h"

//Ӳ������
//#include "usart.h"
//#include "selfcheck.h"

//C��
#include <stdio.h>

CHECK_INFO checkInfo = {DEV_ERR};

unsigned char faultType = FAULT_NONE;			//��ʼΪ�޴�������
unsigned char faultTypeReport = FAULT_NONE;		//������������õ�


//==========================================================
//	�������ƣ�	Fault_Process
//
//	�������ܣ�	������
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void Fault_Process(void)
{
	
	switch(faultType)
	{
		case FAULT_NONE:
			
			printf("WARN:	FAULT_NONE\r\n");
		
		break;
		
		case FAULT_REBOOT:											//���������Ĵ���
			
			printf("WARN:	FAULT_REBOOT\r\n");
		
			faultType = FAULT_NONE;
		
		break;
		
		case FAULT_PRO:												//Э�����Ĵ���
			
			printf("WARN:	FAULT_EDP\r\n");
		
			faultType = FAULT_NONE;
			
			NET_DEVICE_ReLink(oneNetInfo.ip, oneNetInfo.port);
			oneNetInfo.netWork = 0;									//������������ƽ̨
		
		break;
		
		case FAULT_NODEVICE:										//�豸����Ĵ���
			
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
