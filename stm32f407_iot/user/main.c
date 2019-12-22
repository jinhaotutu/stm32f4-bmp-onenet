/**
  ******************************************************************************
  * @file    main.c 
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

#include "net_device.h"
#include "onenet.h"
#include "fault.h"

#include "SI7021.h"
#include "infrared.h"
#include "ov7670.h"
#include "dcmi.h"
#include "ov7670test.h"
#include "rgb2bmp.h"

/* Defines --------------------------------------------------------------------*/


/* Variables ------------------------------------------------------------------*/
extern float Temp_buf;
extern float Humi_buf;

extern u16 Infrared_Count;

DATA_STREAM dataStream[] = {
								{"Home_Temp", &Temp_buf, TYPE_FLOAT, 1},
								{"Home_Humi", &Humi_buf, TYPE_FLOAT, 1},
								{"Home_Infrared", &Infrared_Count, TYPE_USHORT, 1},
							};
unsigned char dataStreamCnt = sizeof(dataStream) / sizeof(dataStream[0]);	
				
/* Functions ------------------------------------------------------------------*/			



/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	/* stm32系统配置 */
	Sys_Config();
	
	/* 红外HC-SR501相关初始化配置 */
	Infrared_Init();
    
    /* SI7021温度传感器IO初始化配置 */
    SI_IO_Init();
    
    /* WIFI模块IO初始化配置 */
    NET_DEVICE_IO_Init();
	
		/* 摄像头OV7670相关初始化配置 */
	if (OV7670_Init() != 0)
	{
		printf("Ov7670 Init Failed.\r\n");
	}
	else
	{
		printf("Ov7670 Init Succeed.\r\n");
#ifdef OV7670_DBG
		OV7670_USART_Init();
#endif
	}

	while(1)
	{
		/************** 平台命令处理**************/
        if(oneNetInfo.cmd_ptr)
		{
			OneNet_RevPro(oneNetInfo.cmd_ptr);
			
			oneNetInfo.cmd_ptr = NULL;
		}
        
        
        /************** 错误处理函数 **************/
        if(faultType != FAULT_NONE)												//如果错误标志被设置
		{
			printf("WARN:	Fault Process\r\n");
			Fault_Process();													//进入错误处理函数
		}
		
		/************** 打包数据发送平台 **************/
		if(oneNetInfo.netWork == 1)
        {
            switch(oneNetInfo.sendData)
            {
                case SEND_TYPE_DATA:	
                    if (!SI7021_measure())
                    {
                        printf("\r\nTemp Data Error!\r\n");
                    }
                    OneNet_SendData(FORMAT_TYPE3, NULL, NULL, dataStream, dataStreamCnt);//上传数据到平台
					printf("\r\nOnenet Data Ready.\r\n");
                    break;
                case SEND_TYPE_HEART:
                    OneNet_SendData_Heart();		//心跳检测
					printf("\r\nOnenet Heart Ready.\r\n");
                    break;
                case SEND_TYPE_PICTURE:
#ifdef OV7670_DBG
					ShanWai_SendCamera(camera_buffer+27, PIC_WIDTH, PIC_HEIGHT);
#endif
					rgb565tobmp((char *)camera_buffer, PIC_WIDTH, PIC_HEIGHT);
                    oneNetInfo.sendData = OneNet_SendData(FORMAT_TYPE2, NULL, NULL, NULL, 0);
					printf("\r\nOnenet Picture Ready.\r\n");
                    break;
                default:
                    break;
            }
            
            if (oneNetInfo.sendData != SEND_TYPE_OK)
            {
                if(NET_DEVICE_CheckListHead())
                {
                    //printf("Wifi Send Data Start.\r\n");
                    oneNetInfo.sendData = NET_DEVICE_SendData(NET_DEVICE_GetListHeadBuf(), NET_DEVICE_GetListHeadLen());
                    NET_DEVICE_DeleteDataSendList();
                    //printf("Wifi Send Data Ok.\r\n\r\n");
                }
            }
        }
		
        /************** 平台登录初始化相关 **************/
        if(oneNetInfo.netWork == 0)
		{
            /*********** 登录onenet平台 ***********/
            if(!oneNetInfo.netWork && (checkInfo.NET_DEVICE_OK == DEV_OK))
			{			
				if(!NET_DEVICE_Init(oneNetInfo.protocol, oneNetInfo.ip, oneNetInfo.port))
				{
					OneNet_DevLink(oneNetInfo.devID, oneNetInfo.apiKey);
					
					if(oneNetInfo.netWork)
					{
						printf("Login in Onenet Succeed.\r\n");
					}
					else
					{
                        printf("Login in Onenet Failed.\r\n");
					}
				}
			} 
        
            /*********** 检测WIFI模块状态 ***********/
            if(checkInfo.NET_DEVICE_OK == DEV_ERR)
            {   
                if(!NET_DEVICE_Exist())
                {
                    printf("NET Device :Ok\r\n");
                    checkInfo.NET_DEVICE_OK = DEV_OK;

					NET_DEVICE_GetWifiIP();
                }
                else
                {
                    printf("NET Device :Error\r\n");
                }
            }
        }
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
