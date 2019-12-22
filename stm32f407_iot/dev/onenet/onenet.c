/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	onenet.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-05-27
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		OneNETƽ̨Ӧ��ʾ��
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f4xx.h"

//�����豸
#include "net_device.h"

//Э���ļ�
#include "onenet.h"
#include "fault.h"
#include "edpkit.h"

//Ӳ������
#include "sys_delay.h"
//#include "usart.h"
//#include "delay.h"
//#include "led.h"
//#include "beep.h"
//#include "spilcd.h"
#include "ov7670.h"

//ͼƬ�����ļ�
#include "image_2k.h"

//C��
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


ONETNET_INFO oneNetInfo = {"25739329", "iCljma3PqyPYAqNdIHpxtS79d60=",
							"183.230.40.39", "876",
							1,
							NULL, 0, 0, 0, 1, 0};



//==========================================================
//	�������ƣ�	OneNet_DevLink
//
//	�������ܣ�	��onenet��������
//
//	��ڲ�����	devid�������豸��devid���ƷID
//				auth_key�������豸��masterKey��apiKey���豸��Ȩ��Ϣ
//
//	���ز�����	��
//
//	˵����		��onenetƽ̨�������ӣ��ɹ������oneNetInfo.netWork����״̬��־
//==========================================================
void OneNet_DevLink(const char* devid, const char* auth_key)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};				//Э���
	
	unsigned char timeOut = 200;
	
	printf("OneNet_DevLink\r\n"
                        "DEVID: %s,     APIKEY: %s\r\n"
                        , devid, auth_key);

#if 1
	if(EDP_PacketConnect1(devid, auth_key, 256, &edpPacket) == 0)	//����devid �� apikey��װЭ���
#else
	if(EDP_PacketConnect2(proid, auth_key, 256, &edpPacket) == 0)	//���ݲ�Ʒid �� ��Ȩ��Ϣ��װЭ���
#endif
	
	{
		NET_DEVICE_SendData(edpPacket._data, edpPacket._len);		//�ϴ�ƽ̨
		//NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);//��������
		
		EDP_DeleteBuffer(&edpPacket);								//ɾ��
		
		while(!oneNetInfo.netWork && --timeOut)
		{
			OneNet_RevPro(oneNetInfo.cmd_ptr);
			delay_ms(10);
		}
	}
	else
		printf("WARN:	EDP_PacketConnect Failed\r\n");
	
	if(oneNetInfo.netWork)											//�������ɹ�
	{
		oneNetInfo.errCount = 0;
	}
	else
	{
		if(++oneNetInfo.errCount >= 5)								//��������趨�����󣬻�δ����ƽ̨
		{
			oneNetInfo.netWork = 0;
			oneNetInfo.errCount = 0;
			faultType = faultTypeReport = FAULT_NODEVICE;			//���ΪӲ������
		}
	}
	
}

//==========================================================
//	�������ƣ�	OneNet_PushData
//
//	�������ܣ�	PUSHDATA
//
//	��ڲ�����	dst_devid�������豸��devid
//				data����������
//				data_len�����ݳ���
//
//	���ز�����	0-���ͳɹ�	1-ʧ��
//
//	˵����		�豸���豸֮���ͨ��
//==========================================================
_Bool OneNet_PushData(const char* dst_devid, const char* data, unsigned int data_len)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};							//Э���
	
	if(!oneNetInfo.netWork)														//�������δ���� �� ��Ϊ�����շ�ģʽ
		return 1;
	
	if(EDP_PacketPushData(dst_devid, data, data_len, &edpPacket) == 0)
	{
		//NET_DEVICE_SendData(edpPacket._data, edpPacket._len);					//�ϴ�ƽ̨
		NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);			//��������
		
		EDP_DeleteBuffer(&edpPacket);											//ɾ��
	}
	else
		printf("WARN:	OneNet_PushData Failed\r\n");
	
	return 0;

}

//==========================================================
//	�������ƣ�	OneNet_SendData
//
//	�������ܣ�	�ϴ����ݵ�ƽ̨
//
//	��ڲ�����	type���������ݵĸ�ʽ
//
//	���ز�����	SEND_TYPE_OK-���ͳɹ�	SEND_TYPE_DATA-��Ҫ����
//
//	˵����		
//==========================================================
unsigned char OneNet_SendData(FORMAT_TYPE type, char *devid, char *apikey, DATA_STREAM *streamArray, unsigned short streamArrayCnt)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};											//Э���
	
	_Bool status = SEND_TYPE_OK;
	short body_len = 0;
	
	if(!oneNetInfo.netWork)
		return SEND_TYPE_DATA;
	
	if(type < 1 && type > 5)
		return SEND_TYPE_DATA;
	
	printf("\r\nTips:	OneNet_SendData-EDP_TYPE%d\r\n", type);
	
	if(type != kTypeBin)																		//�������ļ���ȫ���������ã�����Ҫִ����Щ
	{
		body_len = DSTREAM_GetDataStream_Body_Measure(type, streamArray, streamArrayCnt, 0);	//��ȡ��ǰ��Ҫ���͵����������ܳ���
		
		if(body_len > 0)
		{
			if(EDP_PacketSaveData(devid, body_len, NULL, (SaveDataType)type, &edpPacket) == 0)	//���
			{
				body_len = DSTREAM_GetDataStream_Body(type, streamArray, streamArrayCnt, edpPacket._data, edpPacket._size, edpPacket._len);
				
				if(body_len > 0)
				{
					edpPacket._len += body_len;
					//NET_DEVICE_SendData(edpPacket._data, edpPacket._len);						//�ϴ����ݵ�ƽ̨
					NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);				//��������
					printf("Send %d Bytes\r\n", edpPacket._len);
				}
				else
					printf("WARN:	DSTREAM_GetDataStream_Body Failed\r\n");
				
				EDP_DeleteBuffer(&edpPacket);													//ɾ��
			}
			else
				printf("WARN:	EDP_NewBuffer Failed\r\n");
		}
		else if(body_len < 0)
			return SEND_TYPE_OK;
		else
			status = SEND_TYPE_DATA;
	}
	else
	{
		OneNet_SendData_Picture(devid, (char *)camera_buffer, 320*200*2+54);
		status = SEND_TYPE_OK;
	}
	
	faultTypeReport = FAULT_NONE;																			//����֮��������
	
	return status;
	
}

//==========================================================
//	�������ƣ�	OneNet_SendData_EDPType2
//
//	�������ܣ�	�ϴ����������ݵ�ƽ̨
//
//	��ڲ�����	devid���豸ID(�Ƽ�ΪNULL)
//				picture��ͼƬ����
//				pic_len��ͼƬ���ݳ���
//
//	���ز�����	��
//
//	˵����		���ǵ����豸����������ʱ������ʹ�������豸��͸��ģʽ
//				�ϴ�ͼƬ�ǣ�ǿ�ҽ���devid�ֶ�Ϊ�գ�����ƽ̨�ὫͼƬ�����·����豸
//==========================================================
#define PKT_SIZE 1024
#define PKT_NAME "IOT_PIC_TEST"
void OneNet_SendData_Picture(char *devid, const char* picture, unsigned int pic_len)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};					//Э���
	char type_bin_head[25];

	sprintf(type_bin_head, "{\"ds_id\":\"%s\"}", PKT_NAME);						//ͼƬ����ͷ
	unsigned char *pImage = (unsigned char *)picture;
	
	if(EDP_PacketSaveData(devid, pic_len, type_bin_head, kTypeBin, &edpPacket) == 0)
	{	
		printf("Send %d Bytes\r\n", edpPacket._len);
		NET_DEVICE_SendData(edpPacket._data, edpPacket._len);			//�ϴ����ݵ�ƽ̨
		//NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);	//��������
		
		EDP_DeleteBuffer(&edpPacket);									//ɾ��
		
		printf("image len = %d\r\n", pic_len);
		
		while(pic_len > 0)
		{
			delay_ms(100);												//��ͼʱ��ʱ�������һ�㣬�����������һ����ʱ
			
			if(pic_len >= PKT_SIZE)
			{
				NET_DEVICE_SendData(pImage, PKT_SIZE);					//���ڷ��ͷ�Ƭ
				
				pImage += PKT_SIZE;
				pic_len -= PKT_SIZE;
			}
			else
			{
				NET_DEVICE_SendData(pImage, (unsigned short)pic_len);	//���ڷ������һ����Ƭ
				pic_len = 0;
			}
		}
		
		printf("image send ok\r\n");
	}
	else
		printf("EDP_PacketSaveData Failed\r\n");

}

//==========================================================
//	�������ƣ�	OneNet_HeartBeat
//
//	�������ܣ�	������������
//
//	��ڲ�����	��
//
//	���ز�����	SEND_TYPE_OK-���ͳɹ�	SEND_TYPE_HEART-��Ҫ����
//
//	˵����		
//==========================================================
unsigned char OneNet_SendData_Heart(void)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};		//Э���
	
	if(!oneNetInfo.netWork)									//�������Ϊ���� �� ��Ϊ�����շ�ģʽ
		return SEND_TYPE_HEART;
	
	if(EDP_PacketPing(&edpPacket))
		return SEND_TYPE_HEART;
	
	oneNetInfo.heartBeat = 0;
	
	//NET_DEVICE_SendData(edpPacket._data, edpPacket._len);	//��ƽ̨�ϴ���������
	NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);//��������
	
	EDP_DeleteBuffer(&edpPacket);							//ɾ��
	
	return SEND_TYPE_OK;
	
}

//==========================================================
//	�������ƣ�	OneNet_HeartBeat_Check
//
//	�������ܣ�	������������������
//
//	��ڲ�����	��
//
//	���ز�����	0-�ɹ�	1-�ȴ�
//
//	˵����		���ڵ���ʱ����runCountÿ���˺�������һ�ε�ʱ������
//				�ﵽ�趨���޼��������־λ�Ƿ����
//				����ʱ����Բ���̫��ȷ
//==========================================================
_Bool OneNet_Check_Heart(void)
{
	
	static unsigned char runCount = 0;
	
	if(!oneNetInfo.netWork)
		return 1;

	if(oneNetInfo.heartBeat == 1)
	{
		runCount = 0;
		oneNetInfo.errCount = 0;
		
		return 0;
	}
	
	if(++runCount >= 40)
	{
		runCount = 0;
		
		printf("HeartBeat TimeOut: %d\r\n", oneNetInfo.errCount);
		oneNetInfo.sendData = SEND_TYPE_HEART;		//������������
		
		if(++oneNetInfo.errCount >= 3)
		{
			unsigned char errType = 0;
			
			oneNetInfo.errCount = 0;
			
			errType = NET_DEVICE_Check();											//�����豸״̬���
			if(errType == CHECK_CONNECTED || errType == CHECK_CLOSED || errType == CHECK_GOT_IP)
				faultTypeReport = faultType = FAULT_PRO;								//���ΪЭ�����
			else if(errType == CHECK_NO_DEVICE)
				faultTypeReport = faultType = FAULT_NODEVICE;							//���Ϊ�豸����
			else
				faultTypeReport = faultType = FAULT_NONE;								//�޴���
		}
	}
	
	return 1;

}

//==========================================================
//	�������ƣ�	OneNET_CmdHandle
//
//	�������ܣ�	��ȡƽ̨rb�е�����
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNET_CmdHandle(void)
{
	
	unsigned char *dataPtr = NULL, *ipdPtr = NULL;		//����ָ��

	dataPtr = NET_DEVICE_Read();						//�ȴ�����

	if(dataPtr != NULL)									//������Ч
	{
		ipdPtr = NET_DEVICE_GetIPD(dataPtr);			//����Ƿ���ƽ̨����
		if(ipdPtr != NULL)
		{
			netDeviceInfo.send_ok = 1;
			
			if(netDeviceInfo.netWork)
				oneNetInfo.cmd_ptr = ipdPtr;			//���ƽ̨�·�����
			else
				netDeviceInfo.cmd_ipd = (char *)ipdPtr;
		}
		else
		{
			if(strstr((char *)dataPtr, "SEND OK") != NULL)
			{
				netDeviceInfo.send_ok = 1;
			}
			else if (strstr((char *)dataPtr, "WIFI DISCONNECT") != NULL )
			{
				printf("WARN:	WIFI�Ͽ���׼������\r\n");
				checkInfo.NET_DEVICE_OK = DEV_ERR;
				oneNetInfo.netWork = 0;
				NET_DEVICE_ReConfig(0);
			}
			else if(strstr((char *)dataPtr, "CLOSE") != NULL && netDeviceInfo.netWork)
			{
				printf("WARN:	���ӶϿ���׼������\r\n");
				oneNetInfo.netWork = 0;
				NET_DEVICE_ReConfig(0);
			}
			else
				NET_DEVICE_CmdHandle((char *)dataPtr);
		} 
	}

}

//==========================================================
//	�������ƣ�	OneNet_RevPro
//
//	�������ܣ�	ƽ̨�������ݼ��
//
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};	//Э���
	
	char *cmdid_devid = NULL;
	unsigned short cmdid_len = 0;
	char *req = NULL;
	unsigned int req_len = 0;
	unsigned char type = 0;
	
	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;
	
	type = EDP_UnPacketRecv(cmd);
	switch(type)										//�ж���pushdata���������·�
	{
		case CONNRESP:
		
			switch(EDP_UnPacketConnectRsp(cmd))
			{
				case 0:
					printf("Tips:	���ӳɹ�\r\n");
					oneNetInfo.netWork = 1;
				break;
				
				case 1:printf("WARN:	����ʧ�ܣ�Э�����\r\n");break;
				case 2:printf("WARN:	����ʧ�ܣ��豸ID��Ȩʧ��\r\n");break;
				case 3:printf("WARN:	����ʧ�ܣ�������ʧ��\r\n");break;
				case 4:printf("WARN:	����ʧ�ܣ��û�ID��Ȩʧ��\r\n");break;
				case 5:printf("WARN:	����ʧ�ܣ�δ��Ȩ\r\n");break;
				case 6:printf("WARN:	����ʧ�ܣ���Ȩ����Ч\r\n");break;
				case 7:printf("WARN:	����ʧ�ܣ�������δ����\r\n");break;
				case 8:printf("WARN:	����ʧ�ܣ����豸�ѱ�����\r\n");break;
				case 9:printf("WARN:	����ʧ�ܣ��ظ��������������\r\n");break;
				
				default:printf("ERR:	����ʧ�ܣ�δ֪����\r\n");break;
			}
		
		break;
			
		case DISCONNECT:
		
			printf("WARN:	���ӶϿ���׼�������������룺%d\r\n", cmd[2]);
			oneNetInfo.netWork = 0;
			NET_DEVICE_ReConfig(0);
		
		break;
		
		case PINGRESP:
		
			printf("Tips:	HeartBeat OK\r\n");
			oneNetInfo.heartBeat = 1;
		
		break;
		
		case PUSHDATA:									//��pushdata��
			
			result = EDP_UnPacketPushData(cmd, &cmdid_devid, &req, &req_len);
		
			if(result == 0)
				printf("src_devid: %s, req: %s, req_len: %d\r\n", cmdid_devid, req, req_len);
			
		break;
		
		case CMDREQ:									//�������
			
			result = EDP_UnPacketCmd(cmd, &cmdid_devid, &cmdid_len, &req, &req_len);
			
			if(result == 0)								//����ɹ������������ظ������
			{
				EDP_PacketCmdResp(cmdid_devid, cmdid_len, req, req_len, &edpPacket);
				printf("cmdid: %s, req: %s, req_len: %d\r\n", cmdid_devid, req, req_len);
			}
			
		break;
			
		case SAVEACK:
			
			if(cmd[3] == MSG_ID_HIGH && cmd[4] == MSG_ID_LOW)
			{
				printf("Tips:	Send %s\r\n", cmd[5] ? "Err" : "Ok");
			}
			else
				printf("Tips:	Message ID Err\r\n");
			
		break;
			
		default:
			result = -1;
		break;
	}
	
	if(result == -1)
		return;
	
	dataPtr = strchr(req, '}');							//����'}'
	
	if(dataPtr != NULL)									//����ҵ���
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//�ж��Ƿ����·��������������
		{
			numBuf[num++] = *dataPtr++;
		}
		
		num = atoi((const char *)numBuf);				//תΪ��ֵ��ʽ
		
//		if(strstr((char *)req, "redled"))				//����"redled"
//		{
//			if(num == 1)								//�����������Ϊ1������
//			{
//				Led4_Set(LED_ON);
//			}
//			else if(num == 0)							//�����������Ϊ0�������
//			{
//				Led4_Set(LED_OFF);
//			}
//		}
//														//��ͬ
//		else if(strstr((char *)req, "greenled"))
//		{
//			if(num == 1)
//			{
//				Led5_Set(LED_ON);
//			}
//			else if(num == 0)
//			{
//				Led5_Set(LED_OFF);
//			}
//		}
//		else if(strstr((char *)req, "yellowled"))
//		{
//			if(num == 1)
//			{
//				Led6_Set(LED_ON);
//			}
//			else if(num == 0)
//			{
//				Led6_Set(LED_OFF);
//			}
//		}
//		else if(strstr((char *)req, "blueled"))
//		{
//			if(num == 1)
//			{
//				Led7_Set(LED_ON);
//			}
//			else if(num == 0)
//			{
//				Led7_Set(LED_OFF);
//			}
//		}
//		else if(strstr((char *)req, "beep"))
//		{
//			if(num == 1)
//			{
//				Beep_Set(BEEP_ON);
//			}
//			else if(num == 0)
//			{
//				Beep_Set(BEEP_OFF);
//			}
//		}
//		else if(strstr((char *)req, "lcdbl"))
//		{
//			SPILCD_BL_Ctl(num);
//		}
	}
	
	if(type == PUSHDATA && result == 0)					//�����pushdata �� ����ɹ�
	{
		EDP_FreeBuffer(cmdid_devid);					//�ͷ��ڴ�
		EDP_FreeBuffer(req);
	}
	else if(type == CMDREQ && result == 0)				//���������� �� ����ɹ�
	{
		EDP_FreeBuffer(cmdid_devid);					//�ͷ��ڴ�
		EDP_FreeBuffer(req);
														//�ظ�����
		//NET_DEVICE_SendData(edpPacket._data, edpPacket._len);//�ϴ�ƽ̨
		NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);//��������
		EDP_DeleteBuffer(&edpPacket);					//ɾ��
		oneNetInfo.sendData = 1;						//������ݷ���
	}

}
