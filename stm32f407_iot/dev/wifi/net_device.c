/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	net_device.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-08-05
	*
	*	版本： 		V1.2
	*
	*	说明： 		网络设备应用层
	*
	*	修改记录：	V1.1：1.平台IP和PORT通过参数传入的方式确定，解决了不同协议网络设备驱动不通用的问题。
	*					  2.取消了手动配置网络，上电等待wifi模块自动连接，若不成功则使用OneNET公众号进行配网。
	*					  3.NET_DEVICE_SendCmd新增参数“mode”，决定是否清除本次命令的返回值。
	*				V1.2：1.取消V1.1中的功能3。
	*					  2.更改了数据获取接口，适配rb机制。
	*					  3.取消了透传模式。
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f4xx.h"

//网络设备
#include "net_device.h"
#include "net_io.h"

//硬件驱动
#include "sys_delay.h"
//#include "delay.h"
//#include "led.h"
//#include "key.h"
//#include "usart.h"

//#if(NET_TIME_EN == 1)
//#include "clock.h"
//#endif

//C库
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


NET_DEVICE_INFO netDeviceInfo = {0, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 1, 0};


GPS_INFO gps;


//==========================================================
//	函数名称：	NET_DEVICE_IO_Init
//
//	函数功能：	初始化网络设备IO层
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		初始化网络设备的控制引脚、数据收发功能等
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
	
	NET_IO_Init();											//网络设备数据IO层初始化
	
	NET_DEVICE_Reset();
		
	netDeviceInfo.reboot = 0;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetTime
//
//	函数功能：	获取网络时间
//
//	入口参数：	无
//
//	返回参数：	UTC秒值
//
//	说明：		可搜索NTP协议相关资料
//				NTP服务器：UDP协议，端口123
//
//				已测试可用的NTP服务器-2017-11-07
//				1.cn.pool.ntp.org		来源：网上抄的，不知道哪来的(注意“1.”不是序号，是域名的一部分)
//				cn.ntp.org.cn			来源：中国
//				edu.ntp.org.cn			来源：中国教育网
//				tw.ntp.org.cn			来源：中国台湾
//				us.ntp.org.cn			来源：美国
//				sgp.ntp.org.cn			来源：新加坡
//				kr.ntp.org.cn			来源：韩国
//				de.ntp.org.cn			来源：德国
//				jp.ntp.org.cn			来源：日本
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
		time_buffer[1] = 0;								//表示本地时钟的层次水平
		time_buffer[2] = 6;								//八位signed integer，表示连续信息之间的最大间隔
		time_buffer[3] = 0xEB;							//表示本地时钟精度，精确到秒的平方级
		
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
			
			if(((*dataPtr >> 6) & 0x03) == 3)			//bit6和bit7同为1表示当前不可对时（服务器处于闰秒状态）
				return second;
			
			second = *(dataPtr + 40) << 24 | *(dataPtr + 41) << 16 | *(dataPtr + 42) << 8 | *(dataPtr + 43);
			second -= 2208960000UL;						//时区修正
			
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
//	函数名称：	NET_DEVICE_GetLinkIP
//
//	函数功能：	获取使用协议的登录IP和PORT
//
//	入口参数：	protocol：协议号
//				ip：保存返回IP的缓存区
//				port：保存返回port的缓存区
//
//	返回参数：	0-成功		1-失败
//
//	说明：		1-edp	2-nwx	3-jtext		4-Hiscmd
//				5-jt808			6-modbus	7-mqtt
//				8-gr20			9-reg		10-HTTP(自定义)
//				获取IP本身不支持HTTP协议，这里自定义一个标志
//==========================================================
_Bool NET_DEVICE_GetLinkIP(unsigned char protocol, char *ip, char *port)
{
	
	char *dataPtr = NULL;
	char send_buf[128];
	unsigned char count = 0, timeOut = 200;
	
	if(protocol == 10)													//如果是HTTP协议
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
			dataPtr = strstr(netDeviceInfo.cmd_ipd, "no-cache");				//找到最后的关键词
			
			if(dataPtr != NULL)
				break;
			
			delay_ms(10);
		}
		
		if(dataPtr != NULL)
		{
			if(strstr(dataPtr, "unsupportted") != NULL)							//不支持的协议类型
			{
				printf("不支持该协议类型\r\n");
			}
			else
			{
				dataPtr += 12;													//跳过无用数据
				
				if(*dataPtr < '0' || *dataPtr > '9')
				{
					printf("can't find a available IP\r\n");
				}
				else
				{
					while(*dataPtr != ':')										//复制IP
						ip[count++] = *dataPtr++;
					
					dataPtr++;
					
					strcpy(port, dataPtr);										//复制PORT
					
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
//	函数名称：	NET_DEVICE_Exist
//
//	函数功能：	网络设备存在检查
//
//	入口参数：	无
//
//	返回参数：	返回结果
//
//	说明：		0-成功		1-失败
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
	
	while(timeOut--)												//等待
	{
		delay_ms(1000);												//挂起等待
		
//		if(KeyScan(GPIOC, KEY0) == KEYDOWN)							//长按进入配网模式
//			key_count++;
//		else
//			key_count = 0;
		
		wifi_status = NET_DEVICE_Check();							//获取状态
		if(wifi_status == 3)
			wait_count++;
		
		if(wifi_status || key_count >= 2)							//如果收到数据
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
				printf("请使用OneNET微信公众号配置SSID和PSWD\r\n");
				
				while(1)
				{
//					Led4_Set(LED_ON);Led5_Set(LED_ON);Led6_Set(LED_ON);Led7_Set(LED_ON);
					
					if(strstr((char *)netDeviceInfo.cmd_resp, "SMART SUCCESS"))
					{
						printf("收到:\r\n%s\r\n", strstr((char *)netDeviceInfo.cmd_resp, "SSID:"));
						status = 1;
//						Led4_Set(LED_OFF);Led5_Set(LED_OFF);Led6_Set(LED_OFF);Led7_Set(LED_OFF);
						break;
					}
					else
					{
						if(++cfgTimeOut >= 15)													//超时时间--15s
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
				
				if(cfgTimeOut != 0)																//如果为0，则是超时退出
				{
					cfgTimeOut = 0;
					while(NET_DEVICE_Check() != 2)												//等待WIFI接入
					{
						if(++cfgTimeOut >= 10)
						{
							printf("接入超时,请检查WIFI配置\r\n");
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
//	函数名称：	NET_DEVICE_Init
//
//	函数功能：	网络设备初始化
//
//	入口参数：	protocol：协议号(参考NET_DEVICE_GetLinkIP的说明)
//				ip：IP地址缓存指针
//				port：端口缓存指针
//
//	返回参数：	返回初始化结果
//
//	说明：		0-成功		1-失败
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
		
			if(NET_DEVICE_SendCmd(cfgBuffer, "CONNECT"))					//连接平台，检索“CONNECT”，如果失败会进入循环体
			{
//				Led5_Set(LED_ON);
//				delay_ms(500);
//				
//				Led5_Set(LED_OFF);
//				delay_ms(500);
				
				if(++netDeviceInfo.err >= 10)
				{
					netDeviceInfo.err = 0;
					printf("请检查IP地址和PORT是否正确\r\n");
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
//	函数名称：	NET_DEVICE_Reset
//
//	函数功能：	网络设备复位
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void NET_DEVICE_Reset(void)
{

	printf("Tips:	ESP8266_Reset\r\n");
	
	NET_DEVICE_RST_ON;		//复位
	delay_ms(250);
	
	NET_DEVICE_RST_OFF;		//结束复位
	delay_ms(1000);

}

//==========================================================
//	函数名称：	NET_DEVICE_Close
//
//	函数功能：	关闭网络连接
//
//	入口参数：	无
//
//	返回参数：	0-成功	1-失败
//
//	说明：		
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
//	函数名称：	NET_DEVICE_ReLink
//
//	函数功能：	重连平台
//
//	入口参数：	ip：IP地址缓存指针
//				port：端口缓存指针
//
//	返回参数：	返回连接结果
//
//	说明：		0-成功		1-失败
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

	status = NET_DEVICE_SendCmd(cfgBuffer, "CONNECT");		//重新连接
	
	return status;

}

//==========================================================
//	函数名称：	NET_DEVICE_CmdHandle
//
//	函数功能：	检查命令返回是否正确
//
//	入口参数：	cmd：需要发送的命令
//
//	返回参数：	无
//
//	说明：		命令处理成功则将指针置NULL
//==========================================================
void NET_DEVICE_CmdHandle(char *cmd)
{
	
	if(strstr(cmd, netDeviceInfo.cmd_hdl) != NULL)
		netDeviceInfo.cmd_hdl = NULL;
	
	netDeviceInfo.cmd_resp = cmd;

}

//==========================================================
//	函数名称：	NET_DEVICE_SendCmd
//
//	函数功能：	向网络设备发送一条命令，并等待正确的响应
//
//	入口参数：	cmd：需要发送的命令
//				res：需要检索的响应
//
//	返回参数：	返回连接结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;
	
	NET_IO_Send((unsigned char *)cmd, strlen(cmd));			//写命令到网络设备
	
	if(res == NULL)											//如果为空，则只是发送
		return 0;
	
	netDeviceInfo.cmd_hdl = res;							//需要所搜的关键词
	
	while((netDeviceInfo.cmd_hdl != NULL) && --timeOut)		//等待
		delay_ms(10);
	
	if(timeOut > 0)
		return 0;
	else
		return 1;

}

//==========================================================
//	函数名称：	NET_DEVICE_SendData
//
//	函数功能：	使网络设备发送数据到平台
//
//	入口参数：	data：需要发送的数据
//				len：数据长度
//
//	返回参数：	0-发送完成	1-发送失败
//
//	说明：		
//==========================================================
_Bool NET_DEVICE_SendData(unsigned char *data, unsigned short len)
{
	
	char cmdBuf[40];
	unsigned char time_out = 200;
	
	while(!netDeviceInfo.send_ok && --time_out)		//等待上一次的数据成功发送
		delay_ms(10);
	
	if(time_out == 0)
		return 1;
	
	netDeviceInfo.send_ok = 0;
	
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//发送命令
	if(!NET_DEVICE_SendCmd(cmdBuf, ">"))			//收到‘>’时可以发送数据
	{
		NET_IO_Send(data, len);  					//发送设备连接请求数据
	}
	
	delay_ms(50);									//等待一下
	
	return 0;

}

//==========================================================
//	函数名称：	NET_DEVICE_Read
//
//	函数功能：	读取一帧数据
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
unsigned char *NET_DEVICE_Read(void)
{

	return NET_IO_Read();

}

//==========================================================
//	函数名称：	NET_DEVICE_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	dataPtr：原始数据指针
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *NET_DEVICE_GetIPD(unsigned char *dataPtr)
{
	
	char *dataPtr_tp = (char *)dataPtr;
	
	if(dataPtr_tp != NULL)
	{
		dataPtr_tp = strstr(dataPtr_tp, "IPD,");					//搜索“IPD”头
		if(dataPtr_tp == NULL)										//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
		{
			//printf("\"IPD\" not found\r\n");
		}
		else
		{
			dataPtr_tp = strchr(dataPtr_tp, ':');					//找到':'
			if(dataPtr_tp != NULL)
			{
				dataPtr_tp++;
				return (unsigned char *)(dataPtr_tp);
			}
			else
				return NULL;
			
		}
	}

	return NULL;													//超时还未找到，返回空指针

}

//==========================================================
//	函数名称：	NET_DEVICE_Check
//
//	函数功能：	检查网络设备连接状态
//
//	入口参数：	无
//
//	返回参数：	返回状态
//
//	说明：		
//==========================================================
unsigned char NET_DEVICE_Check(void)
{

	if(NET_DEVICE_SendCmd("AT+CIPSTATUS\r\n", "OK") == 0)			//发送状态监测
	{
		if(strstr(netDeviceInfo.cmd_resp, "STATUS:2"))				//获得IP
		{
			printf("ESP8266 Got IP\r\n");
			return 2;
		}
		else if(strstr(netDeviceInfo.cmd_resp, "STATUS:3"))			//建立连接
		{
			printf("ESP8266 Connect OK\r\n");
			return 2;
		}
		else if(strstr(netDeviceInfo.cmd_resp, "STATUS:4"))			//失去连接
		{
			printf("ESP8266 Lost Connect\r\n");
			return 2;
		}
		else if(strstr(netDeviceInfo.cmd_resp, "STATUS:5"))			//物理掉线
		{
			//printf("ESP8266 Lost\r\n");			//设备丢失
			return 3;
		}
		else
			return 3;
	}
	else
		return 3;

}

//==========================================================
//	函数名称：	NET_DEVICE_ReConfig
//
//	函数功能：	设备网络设备初始化的步骤
//
//	入口参数：	步骤值
//
//	返回参数：	无
//
//	说明：		该函数设置的参数在网络设备初始化里边用到
//==========================================================
void NET_DEVICE_ReConfig(unsigned char step)
{

	netDeviceInfo.initStep = step;

}

/******************************************************************************************
										消息队列
******************************************************************************************/

//==========================================================
//	函数名称：	NET_DEVICE_CheckListHead
//
//	函数功能：	检查发送链表头是否为空
//
//	入口参数：	无
//
//	返回参数：	0-空	1-不为空
//
//	说明：		
//==========================================================
_Bool NET_DEVICE_CheckListHead(void)
{

	if(netDeviceInfo.head == NULL)
		return 0;
	else
		return 1;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetListHeadBuf
//
//	函数功能：	获取链表里需要发送的数据指针
//
//	入口参数：	无
//
//	返回参数：	获取链表里需要发送的数据指针
//
//	说明：		
//==========================================================
unsigned char *NET_DEVICE_GetListHeadBuf(void)
{

	return netDeviceInfo.head->buf;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetListHeadLen
//
//	函数功能：	获取链表里需要发送的数据长度
//
//	入口参数：	无
//
//	返回参数：	获取链表里需要发送的数据长度
//
//	说明：		
//==========================================================
unsigned short NET_DEVICE_GetListHeadLen(void)
{

	return netDeviceInfo.head->dataLen;

}

//==========================================================
//	函数名称：	NET_DEVICE_AddDataSendList
//
//	函数功能：	在发送链表尾新增一个发送链表
//
//	入口参数：	buf：需要发送的数据
//				dataLen：数据长度
//
//	返回参数：	0-成功	其他-失败
//
//	说明：		异步发送方式
//==========================================================
unsigned char NET_DEVICE_AddDataSendList(unsigned char *buf ,unsigned short dataLen)
{
	
	struct NET_SEND_LIST *current = (struct NET_SEND_LIST *)NET_MallocBuffer(sizeof(struct NET_SEND_LIST));
																//分配内存
	
	if(current == NULL)
		return 1;
	
	current->buf = (unsigned char *)NET_MallocBuffer(dataLen);	//分配内存
	if(current->buf == NULL)
	{
		NET_FreeBuffer(current);								//失败则释放
		return 2;
	}
	
	if(netDeviceInfo.head == NULL)								//如果head为NULL
		netDeviceInfo.head = current;							//head指向当前分配的内存区
	else														//如果head不为NULL
		netDeviceInfo.end->next = current;							//则end指向当前分配的内存区
	
	memcpy(current->buf, buf, dataLen);							//复制数据
	current->dataLen = dataLen;
	current->next = NULL;										//下一段为NULL
	
	netDeviceInfo.end = current;								//end指向当前分配的内存区
	
	return 0;

}

//==========================================================
//	函数名称：	NET_DEVICE_DeleteDataSendList
//
//	函数功能：	从链表头删除一个链表
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
_Bool NET_DEVICE_DeleteDataSendList(void)
{
	
	struct NET_SEND_LIST *next = netDeviceInfo.head->next;	//保存链表头的下一段数据地址
	
	netDeviceInfo.head->dataLen = 0;
	netDeviceInfo.head->next = NULL;
	NET_FreeBuffer(netDeviceInfo.head->buf);				//释放内存
	NET_FreeBuffer(netDeviceInfo.head);						//释放内存
	
	netDeviceInfo.head = next;								//链表头指向下一段数据
	
	return 0;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetWifiIP
//
//	函数功能：	获取本地IP地址和MAC
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
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
