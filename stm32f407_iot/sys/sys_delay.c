/**
  ******************************************************************************
  * @file    sys_delay.c 
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

/* Defines --------------------------------------------------------------------*/


/* Variables ------------------------------------------------------------------*/

				
/* Functions ------------------------------------------------------------------*/			


void Delay_Configuration(void)  
{  
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;     //时钟源为系统时钟168MHz  
    SysTick->LOAD = 167;                             //重载值为168-1，每1us溢出一次  
}  

void delay_ms(vu32 nTime)  
{  
    nTime *= 1000;  
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;        //使能SysTick，开始计数  
    while(nTime--){  
        while((SysTick->CTRL&0X010000) == 0);        //等待COUNTFLAG标志位置1  
    }  
    SysTick->CTRL &= (~SysTick_CTRL_ENABLE_Msk);     //失能SysTick，停止计数  
}  
  
void delay_us(vu32 nTime)  
{  
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;  
    while(nTime--){  
        while((SysTick->CTRL&0X010000) == 0);  
    }  
    SysTick->CTRL &= (~SysTick_CTRL_ENABLE_Msk);  
} 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
