/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	net_device.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-08-05
	*
	*	�汾�� 		V1.2
	*
	*	˵���� 		�����豸Ӧ�ò�
	*
	*	�޸ļ�¼��	V1.1��1.ƽ̨IP��PORTͨ����������ķ�ʽȷ��������˲�ͬЭ�������豸������ͨ�õ����⡣
	*					  2.ȡ�����ֶ��������磬�ϵ�ȴ�wifiģ���Զ����ӣ������ɹ���ʹ��OneNET���ںŽ���������
	*					  3.NET_DEVICE_SendCmd����������mode���������Ƿ������������ķ���ֵ��
	*				V1.2��1.ȡ��V1.1�еĹ���3��
	*					  2.���������ݻ�ȡ�ӿڣ�����rb���ơ�
	*					  3.ȡ����͸��ģʽ��
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f4xx.h"

//�����豸
#include "net_device.h"
#include "net_io.h"

//Ӳ������
#include "sys_delay.h"
//#include "delay.h"
//#include "led.h"
//#include "key.h"
//#include "usart.h"

//#if(NET_TIME_EN == 1)
//#include "clock.h"
//#endif

//C��
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


NET_DEVICE_INFO netDeviceInfo = {0, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 1, 0};


GPS_INFO gps;


//==========================================================
//	�������ƣ�	NET_DEVICE_IO_Init
//
//	�������ܣ�	��ʼ�������豸IO��
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		��ʼ�������豸�Ŀ������š������շ����ܵ�
//==========================================================
void NET_DEVICE_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_MODE,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_RESET,ENABLE); 
	
	//RESET
    GPIO_InitStructure.GPIO_Pin = PIN_RESET;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIO_RESET, &GPIO_InitStructure);
	
	//MODE
	GPIO_InitStructure.GPIO_Pin = PIN_MODE;
    GPIO_Init(GPIO_MODE, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIO_RESET,PIN_RESET);
	GPIO_SetBits(GPIO_MODE,PIN_MODE);
	
	NET_IO_Init();											//�����豸����IO���ʼ��
	
	NET_DEVICE_Reset();
		
	netDeviceInfo.reboot = 0;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_GetTime
//
//	�������ܣ�	��ȡ����ʱ��
//
//	��ڲ�����	��
//
//	���ز�����	UTC��ֵ
//
//	˵����		������NTPЭ���������
//				NTP��������UDPЭ�飬�˿�123
//
//				�Ѳ��Կ��õ�NTP������-2017-11-07
//				1.cn.pool.ntp.org		��Դ�����ϳ��ģ���֪��������(ע�⡰1.��������ţ���������һ����)
//				cn.ntp.org.cn			��Դ���й�
//				edu.ntp.org.cn			��Դ���й�������
//				tw.ntp.org.cn			��Դ���й�̨��
//				us.ntp.org.cn			��Դ������
//				sgp.ntp.org.cn			��Դ���¼���
//				kr.ntp.org.cn			��Դ������
//				de.ntp.org.cn			��Դ���¹�
//				jp.ntp.org.cn			��Դ���ձ�
//==========================================================
unsigned int NET_DEVICE_GetTime(void)
{
	
	unsigned int second = 0;
	
#if(NET_TIME_EN == 1)
	struct tm *localTime;
	unsigned char timeOut = 200;
	char *dataPtr = NULL;
	
	unsigned char time_buffer[48];

	NET_DEVICE_Close();
	
	printf("AT+CIPSTART=\"UDP\",\"edu.ntp.org.cn\",123\r\n");
	if(NET_DEVICE_SendCmd("AT+CIPSTART=\"UDP\",\"edu.ntp.org.cn\",123\r\n", "CONNECT") == 0)
	{
		memset(time_buffer, 0, sizeof(time_buffer));
		
		time_buffer[0] = 0xE3;							//LI, Version, Mode
		time_buffer[1] = 0;								//��ʾ����ʱ�ӵĲ��ˮƽ
		time_buffer[2] = 6;								//��λsigned integer����ʾ������Ϣ֮��������
		time_buffer[3] = 0xEB;							//��ʾ����ʱ�Ӿ��ȣ���ȷ�����ƽ����
		
		NET_DEVICE_SendData(time_buffer, sizeof(time_buffer));
		netDeviceInfo.cmd_ipd = NULL;
		
		while(--timeOut)
		{
			if(netDeviceInfo.cmd_ipd != NULL)
				break;
			
			delay_ms(10);
		}
		
		if(netDeviceInfo.cmd_ipd)
		{
			dataPtr = netDeviceInfo.cmd_ipd;
			
			NET_DEVICE_Close();
			
			if(((*dataPtr >> 6) & 0x03) == 3)			//bit6��bit7ͬΪ1��ʾ��ǰ���ɶ�ʱ����������������״̬��
				return second;
			
			second = *(dataPtr + 40) << 24 | *(dataPtr + 41) << 16 | *(dataPtr + 42) << 8 | *(dataPtr + 43);
			second -= 2208960000UL;						//ʱ������
			
			localTime = localtime(&second);
			
//			printf("UTC Time: %d-%d-%d %d:%d:%d\r\n",
//									localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
//									localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
			
//			for(timeOut = 0; timeOut < sizeof(time_buffer); timeOut++)
//			{
//				printf("%X  ", *dataPtr++);
//				if((timeOut + 1) % 8 == 0)
//					printf("\r\n");
//					
//			}
//			printf("\r\nsecond: %u\r\n", second);
		}
	}
#endif
	
	return second;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_GetLinkIP
//
//	�������ܣ�	��ȡʹ��Э��ĵ�¼IP��PORT
//
//	��ڲ�����	protocol��Э���
//				ip�����淵��IP�Ļ�����
//				port�����淵��port�Ļ�����
//
//	���ز�����	0-�ɹ�		1-ʧ��
//
//	˵����		1-edp	2-nwx	3-jtext		4-Hiscmd
//				5-jt808			6-modbus	7-mqtt
//				8-gr20			9-reg		10-HTTP(�Զ���)
//				��ȡIP����֧��HTTPЭ�飬�����Զ���һ����־
//==========================================================
_Bool NET_DEVICE_GetLinkIP(unsigned char protocol, char *ip, char *port)
{
	
	char *dataPtr = NULL;
	char send_buf[128];
	unsigned char count = 0, timeOut = 200;
	
	if(protocol == 10)													//�����HTTPЭ��
	{
		strcpy(ip, "183.230.40.33");
		strcpy(port, "80");
		
		return 0;
	}
	
	NET_DEVICE_Close();
	
	printf("AT+CIPSTART=\"TCP\",\"183.230.40.33\",80\r\n");
	if(NET_DEVICE_SendCmd("AT+CIPSTART=\"TCP\",\"183.230.40.33\",80\r\n", "CONNECT") == 0)
	{
		memset(send_buf, 0, sizeof(send_buf));
		snprintf(send_buf, sizeof(send_buf), "GET http://api.heclouds.com/s?t=%d&v=public HTTP/1.1\r\n"
												"api-key:=sUT=jsLGXkQcUz3Z9EaiNQ80U0=\r\n"
												"Host:api.heclouds.com\r\n\r\n",
												protocol);
		
		NET_DEVICE_SendData((unsigned char *)send_buf, strlen(send_buf));
		
		while(--timeOut)
		{
			dataPtr = strstr(netDeviceInfo.cmd_ipd, "no-cache");				//�ҵ����Ĺؼ���
			
			if(dataPtr != NULL)
				break;
			
			delay_ms(10);
		}
		
		if(dataPtr != NULL)
		{
			if(strstr(dataPtr, "unsupportted") != NULL)							//��֧�ֵ�Э������
			{
				printf("��֧�ָ�Э������\r\n");
			}
			else
			{
				dataPtr += 12;													//������������
				
				if(*dataPtr < '0' || *dataPtr > '9')
				{
					printf("can't find a available IP\r\n");
				}
				else
				{
					while(*dataPtr != ':')										//����IP
						ip[count++] = *dataPtr++;
					
					dataPtr++;
					
					strcpy(port, dataPtr);										//����PORT
					
					printf("Get: ip: %s, port: %s\r\n", ip, port);
				}
			}
		}
		
		NET_DEVICE_Close();
	}
	else
		return 1;
	
	return 0;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_Exist
//
//	�������ܣ�	�����豸���ڼ��
//
//	��ڲ�����	��
//
//	���ز�����	���ؽ��
//
//	˵����		0-�ɹ�		1-ʧ��
//==========================================================
_Bool NET_DEVICE_Exist(void)
{

	unsigned char timeOut = 10;
#if(PHONE_AP_MODE == 0)
	unsigned char cfgTimeOut = 0;
#endif
	_Bool status = 1;
	unsigned char key_count = 0, wait_count = 0, wifi_status = 0;
	
	char cfgBuffer[40];
	
	while(timeOut--)												//�ȴ�
	{
		delay_ms(1000);												//����ȴ�
		
//		if(KeyScan(GPIOC, KEY0) == KEYDOWN)							//������������ģʽ
//			key_count++;
//		else
//			key_count = 0;
		
		wifi_status = NET_DEVICE_Check();							//��ȡ״̬
		if(wifi_status == 3)
			wait_count++;
		
		if(wifi_status || key_count >= 2)							//����յ�����
		{
			if(wifi_status == 2)
			{
				status = 0;
				break;
			}
			else if(wait_count >= 10 || key_count >= 2)
			{
				wait_count = 0;
				key_count = 0;
#if(PHONE_AP_MODE == 0)
				NET_DEVICE_SendCmd("AT+CWSMARTSTART=2\r\n", "OK");
				printf("��ʹ��OneNET΢�Ź��ں�����SSID��PSWD\r\n");
				
				while(1)
				{
//					Led4_Set(LED_ON);Led5_Set(LED_ON);Led6_Set(LED_ON);Led7_Set(LED_ON);
					
					if(strstr((char *)netDeviceInfo.cmd_resp, "SMART SUCCESS"))
					{
						printf("�յ�:\r\n%s\r\n", strstr((char *)netDeviceInfo.cmd_resp, "SSID:"));
						status = 1;
//						Led4_Set(LED_OFF);Led5_Set(LED_OFF);Led6_Set(LED_OFF);Led7_Set(LED_OFF);
						break;
					}
					else
					{
						if(++cfgTimeOut >= 15)													//��ʱʱ��--15s
						{
							cfgTimeOut = 0;
//							Led4_Set(LED_OFF);Led5_Set(LED_OFF);Led6_Set(LED_OFF);Led7_Set(LED_OFF);
							break;
						}
					}
					
//					delay_ms(500);
//					Led4_Set(LED_OFF);Led5_Set(LED_OFF);Led6_Set(LED_OFF);Led7_Set(LED_OFF);
//					delay_ms(500);
				}
				
				if(cfgTimeOut != 0)																//���Ϊ0�����ǳ�ʱ�˳�
				{
					cfgTimeOut = 0;
					while(NET_DEVICE_Check() != 2)												//�ȴ�WIFI����
					{
						if(++cfgTimeOut >= 10)
						{
							printf("���볬ʱ,����WIFI����\r\n");
							break;
						}
						
						delay_ms(1000);
					}
				}
#else
				printf("STA Tips:	Link Wifi\r\n");
				
				sprintf(cfgBuffer, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", WIFI_NAME, WIFI_PASSWORD );
				
				if(NET_DEVICE_SendCmd(cfgBuffer, "WIFI GOT IP"))
				{
//					Led7_Set(LED_ON);
//					delay_ms(500);
//					
//					Led7_Set(LED_OFF);
//					delay_ms(500);
                    delay_ms(5000);
				}
				
				if(NET_DEVICE_Check() == 2)
				{
					status = 0;
					break;
				}
#endif
			}
			else
				status = 1;
		}
	}
	
	return status;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_Init
//
//	�������ܣ�	�����豸��ʼ��
//
//	��ڲ�����	protocol��Э���(�ο�NET_DEVICE_GetLinkIP��˵��)
//				ip��IP��ַ����ָ��
//				port���˿ڻ���ָ��
//
//	���ز�����	���س�ʼ�����
//
//	˵����		0-�ɹ�		1-ʧ��
//==========================================================
_Bool NET_DEVICE_Init(unsigned char protocol, char *ip, char *port)
{
	
	_Bool status = 1;
	char cfgBuffer[32];
	
	netDeviceInfo.netWork = 0;

	switch(netDeviceInfo.initStep)
	{
		case 0:
			
#if(NET_TIME_EN == 1)
			if(!netDeviceInfo.net_time)
			{
				netDeviceInfo.net_time = NET_DEVICE_GetTime();
				if(++netDeviceInfo.err >= 3)
				{
					netDeviceInfo.err = 0;
					netDeviceInfo.initStep++;
				}
				
				delay_ms(500);
			}
			else
#endif
				netDeviceInfo.initStep++;
		break;
			
		case 1:
		
			if(!NET_DEVICE_GetLinkIP(protocol, ip, port))
					netDeviceInfo.initStep++;
		
		break;
		
		case 2:
			
			memset(cfgBuffer, 0, sizeof(cfgBuffer));
			
			strcpy(cfgBuffer, "AT+CIPSTART=\"TCP\",\"");
			strcat(cfgBuffer, ip);
			strcat(cfgBuffer, "\",");
			strcat(cfgBuffer, port);
			strcat(cfgBuffer, "\r\n");
			printf("STA Tips:	%s", cfgBuffer);
		
			if(NET_DEVICE_SendCmd(cfgBuffer, "CONNECT"))					//����ƽ̨��������CONNECT�������ʧ�ܻ����ѭ����
			{
//				Led5_Set(LED_ON);
//				delay_ms(500);
//				
//				Led5_Set(LED_OFF);
//				delay_ms(500);
				
				if(++netDeviceInfo.err >= 10)
				{
					netDeviceInfo.err = 0;
					printf("����IP��ַ��PORT�Ƿ���ȷ\r\n");
				}
			}
			else
				netDeviceInfo.initStep++;
		
		break;
		
		default:
			
			status = 0;
			netDeviceInfo.netWork = 1;
			printf("Tips:	ESP8266 STA_Mode OK\r\n");
		
		break;
	}
	
	return status;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_Reset
//
//	�������ܣ�	�����豸��λ
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void NET_DEVICE_Reset(void)
{

	printf("Tips:	ESP8266_Reset\r\n");
	
	NET_DEVICE_RST_ON;		//��λ
	delay_ms(250);
	
	NET_DEVICE_RST_OFF;		//������λ
	delay_ms(1000);

}

//==========================================================
//	�������ƣ�	NET_DEVICE_Close
//
//	�������ܣ�	�ر���������
//
//	��ڲ�����	��
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����		
//==========================================================
_Bool NET_DEVICE_Close(void)
{
	
	_Bool result = 1;
	
	printf("Tips:	CLOSE\r\n");

	if(NET_DEVICE_SendCmd("AT+CIPCLOSE\r\n", "OK"))
		result = 0;
	else
		result = 1;
	
	delay_ms(50);
	
	return result;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_ReLink
//
//	�������ܣ�	����ƽ̨
//
//	��ڲ�����	ip��IP��ַ����ָ��
//				port���˿ڻ���ָ��
//
//	���ز�����	�������ӽ��
//
//	˵����		0-�ɹ�		1-ʧ��
//==========================================================
_Bool NET_DEVICE_ReLink(char *ip, char *port)
{
	
	_Bool status = 0;
	char cfgBuffer[48];
	
	NET_DEVICE_Close();

	memset(cfgBuffer, 0, sizeof(cfgBuffer));
			
	strcpy(cfgBuffer, "AT+CIPSTART=\"TCP\",\"");
	strcat(cfgBuffer, ip);
	strcat(cfgBuffer, "\",");
	strcat(cfgBuffer, port);
	strcat(cfgBuffer, "\r\n");
	printf("Tips:	%s", cfgBuffer);

	status = NET_DEVICE_SendCmd(cfgBuffer, "CONNECT");		//��������
	
	return status;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_CmdHandle
//
//	�������ܣ�	���������Ƿ���ȷ
//
//	��ڲ�����	cmd����Ҫ���͵�����
//
//	���ز�����	��
//
//	˵����		�����ɹ���ָ����NULL
//==========================================================
void NET_DEVICE_CmdHandle(char *cmd)
{
	
	if(strstr(cmd, netDeviceInfo.cmd_hdl) != NULL)
		netDeviceInfo.cmd_hdl = NULL;
	
	netDeviceInfo.cmd_resp = cmd;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_SendCmd
//
//	�������ܣ�	�������豸����һ��������ȴ���ȷ����Ӧ
//
//	��ڲ�����	cmd����Ҫ���͵�����
//				res����Ҫ��������Ӧ
//
//	���ز�����	�������ӽ��
//
//	˵����		0-�ɹ�		1-ʧ��
//==========================================================
_Bool NET_DEVICE_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;
	
	NET_IO_Send((unsigned char *)cmd, strlen(cmd));			//д��������豸
	
	if(res == NULL)											//���Ϊ�գ���ֻ�Ƿ���
		return 0;
	
	netDeviceInfo.cmd_hdl = res;							//��Ҫ���ѵĹؼ���
	
	while((netDeviceInfo.cmd_hdl != NULL) && --timeOut)		//�ȴ�
		delay_ms(10);
	
	if(timeOut > 0)
		return 0;
	else
		return 1;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_SendData
//
//	�������ܣ�	ʹ�����豸�������ݵ�ƽ̨
//
//	��ڲ�����	data����Ҫ���͵�����
//				len�����ݳ���
//
//	���ز�����	0-�������	1-����ʧ��
//
//	˵����		
//==========================================================
_Bool NET_DEVICE_SendData(unsigned char *data, unsigned short len)
{
	
	char cmdBuf[40];
	unsigned char time_out = 200;
	
	while(!netDeviceInfo.send_ok && --time_out)		//�ȴ���һ�ε����ݳɹ�����
		delay_ms(10);
	
	if(time_out == 0)
		return 1;
	
	netDeviceInfo.send_ok = 0;
	
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//��������
	if(!NET_DEVICE_SendCmd(cmdBuf, ">"))			//�յ���>��ʱ���Է�������
	{
		NET_IO_Send(data, len);  					//�����豸������������
	}
	
	delay_ms(50);									//�ȴ�һ��
	
	return 0;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_Read
//
//	�������ܣ�	��ȡһ֡����
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
unsigned char *NET_DEVICE_Read(void)
{

	return NET_IO_Read();

}

//==========================================================
//	�������ƣ�	NET_DEVICE_GetIPD
//
//	�������ܣ�	��ȡƽ̨���ص�����
//
//	��ڲ�����	dataPtr��ԭʼ����ָ��
//
//	���ز�����	ƽ̨���ص�ԭʼ����
//
//	˵����		��ͬ�����豸���صĸ�ʽ��ͬ����Ҫȥ����
//				��ESP8266�ķ��ظ�ʽΪ	"+IPD,x:yyy"	x�������ݳ��ȣ�yyy����������
//==========================================================
unsigned char *NET_DEVICE_GetIPD(unsigned char *dataPtr)
{
	
	char *dataPtr_tp = (char *)dataPtr;
	
	if(dataPtr_tp != NULL)
	{
		dataPtr_tp = strstr(dataPtr_tp, "IPD,");					//������IPD��ͷ
		if(dataPtr_tp == NULL)										//���û�ҵ���������IPDͷ���ӳ٣�������Ҫ�ȴ�һ�ᣬ�����ᳬ���趨��ʱ��
		{
			//printf("\"IPD\" not found\r\n");
		}
		else
		{
			dataPtr_tp = strchr(dataPtr_tp, ':');					//�ҵ�':'
			if(dataPtr_tp != NULL)
			{
				dataPtr_tp++;
				return (unsigned char *)(dataPtr_tp);
			}
			else
				return NULL;
			
		}
	}

	return NULL;													//��ʱ��δ�ҵ������ؿ�ָ��

}

//==========================================================
//	�������ƣ�	NET_DEVICE_Check
//
//	�������ܣ�	��������豸����״̬
//
//	��ڲ�����	��
//
//	���ز�����	����״̬
//
//	˵����		
//==========================================================
unsigned char NET_DEVICE_Check(void)
{

	if(NET_DEVICE_SendCmd("AT+CIPSTATUS\r\n", "OK") == 0)			//����״̬���
	{
		if(strstr(netDeviceInfo.cmd_resp, "STATUS:2"))				//���IP
		{
			printf("ESP8266 Got IP\r\n");
			return 2;
		}
		else if(strstr(netDeviceInfo.cmd_resp, "STATUS:3"))			//��������
		{
			printf("ESP8266 Connect OK\r\n");
			return 2;
		}
		else if(strstr(netDeviceInfo.cmd_resp, "STATUS:4"))			//ʧȥ����
		{
			printf("ESP8266 Lost Connect\r\n");
			return 2;
		}
		else if(strstr(netDeviceInfo.cmd_resp, "STATUS:5"))			//�������
		{
			//printf("ESP8266 Lost\r\n");			//�豸��ʧ
			return 3;
		}
		else
			return 3;
	}
	else
		return 3;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_ReConfig
//
//	�������ܣ�	�豸�����豸��ʼ���Ĳ���
//
//	��ڲ�����	����ֵ
//
//	���ز�����	��
//
//	˵����		�ú������õĲ����������豸��ʼ������õ�
//==========================================================
void NET_DEVICE_ReConfig(unsigned char step)
{

	netDeviceInfo.initStep = step;

}

/******************************************************************************************
										��Ϣ����
******************************************************************************************/

//==========================================================
//	�������ƣ�	NET_DEVICE_CheckListHead
//
//	�������ܣ�	��鷢������ͷ�Ƿ�Ϊ��
//
//	��ڲ�����	��
//
//	���ز�����	0-��	1-��Ϊ��
//
//	˵����		
//==========================================================
_Bool NET_DEVICE_CheckListHead(void)
{

	if(netDeviceInfo.head == NULL)
		return 0;
	else
		return 1;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_GetListHeadBuf
//
//	�������ܣ�	��ȡ��������Ҫ���͵�����ָ��
//
//	��ڲ�����	��
//
//	���ز�����	��ȡ��������Ҫ���͵�����ָ��
//
//	˵����		
//==========================================================
unsigned char *NET_DEVICE_GetListHeadBuf(void)
{

	return netDeviceInfo.head->buf;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_GetListHeadLen
//
//	�������ܣ�	��ȡ��������Ҫ���͵����ݳ���
//
//	��ڲ�����	��
//
//	���ز�����	��ȡ��������Ҫ���͵����ݳ���
//
//	˵����		
//==========================================================
unsigned short NET_DEVICE_GetListHeadLen(void)
{

	return netDeviceInfo.head->dataLen;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_AddDataSendList
//
//	�������ܣ�	�ڷ�������β����һ����������
//
//	��ڲ�����	buf����Ҫ���͵�����
//				dataLen�����ݳ���
//
//	���ز�����	0-�ɹ�	����-ʧ��
//
//	˵����		�첽���ͷ�ʽ
//==========================================================
unsigned char NET_DEVICE_AddDataSendList(unsigned char *buf ,unsigned short dataLen)
{
	
	struct NET_SEND_LIST *current = (struct NET_SEND_LIST *)NET_MallocBuffer(sizeof(struct NET_SEND_LIST));
																//�����ڴ�
	
	if(current == NULL)
		return 1;
	
	current->buf = (unsigned char *)NET_MallocBuffer(dataLen);	//�����ڴ�
	if(current->buf == NULL)
	{
		NET_FreeBuffer(current);								//ʧ�����ͷ�
		return 2;
	}
	
	if(netDeviceInfo.head == NULL)								//���headΪNULL
		netDeviceInfo.head = current;							//headָ��ǰ������ڴ���
	else														//���head��ΪNULL
		netDeviceInfo.end->next = current;							//��endָ��ǰ������ڴ���
	
	memcpy(current->buf, buf, dataLen);							//��������
	current->dataLen = dataLen;
	current->next = NULL;										//��һ��ΪNULL
	
	netDeviceInfo.end = current;								//endָ��ǰ������ڴ���
	
	return 0;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_DeleteDataSendList
//
//	�������ܣ�	������ͷɾ��һ������
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
_Bool NET_DEVICE_DeleteDataSendList(void)
{
	
	struct NET_SEND_LIST *next = netDeviceInfo.head->next;	//��������ͷ����һ�����ݵ�ַ
	
	netDeviceInfo.head->dataLen = 0;
	netDeviceInfo.head->next = NULL;
	NET_FreeBuffer(netDeviceInfo.head->buf);				//�ͷ��ڴ�
	NET_FreeBuffer(netDeviceInfo.head);						//�ͷ��ڴ�
	
	netDeviceInfo.head = next;								//����ͷָ����һ������
	
	return 0;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_GetWifiIP
//
//	�������ܣ�	��ȡ����IP��ַ��MAC
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
_Bool NET_DEVICE_GetWifiIP(void)
{
	char *dataPtr_ip = NULL;
	char *dataPtr_mac = NULL;
	u8 timeOut = 200;
	
	NET_DEVICE_SendCmd("AT+CIFSR\r\n", "+CIFSR:");
	{
		while(--timeOut)
		{
			dataPtr_ip = strstr(netDeviceInfo.cmd_resp, "STAIP");
			dataPtr_mac = strstr(netDeviceInfo.cmd_resp, "STAMAC");
			*(strstr(dataPtr_ip, ",")) = ':';
			*(strstr(dataPtr_ip, "\r\n")) = 0;
			*(strstr(dataPtr_mac, ",")) = ':';
			*(strstr(dataPtr_mac, "\r\n")) = 0;
			
			if((dataPtr_ip != NULL) && (dataPtr_mac != NULL))
			{
				printf("%s\r\n%s\r\n", dataPtr_ip, dataPtr_mac);
				break;
			}
			
			delay_ms(10);
		}
	}
	
	return 0;
}
