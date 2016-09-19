/*
 * File	: at_port.c
 * This file is part of Espressif's AT+ command set program.
 * Copyright (C) 2013 - 2016, Espressif Systems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "user_interface.h"
#include "osapi.h"
#include "driver/uart.h"
#include "mem.h"


#define at_recvTaskPrio        0
#define at_recvTaskQueueLen    10

os_event_t    at_recvTaskQueue[at_recvTaskQueueLen];
//os_event_t    at_busyTaskQueue[at_busyTaskQueueLen];
//os_event_t    at_procTaskQueue[at_procTaskQueueLen];

#if 0

/** @defgroup AT_PORT_Defines
  * @{
  */
#define at_cmdLenMax 128
#define at_dataLenMax 2048
/**
  * @}
  */

/** @defgroup AT_PORT_Extern_Variables
  * @{
  */
extern uint16_t at_sendLen;
extern uint16_t at_tranLen;
//extern UartDevice UartDev;
//extern bool IPMODE;
extern os_timer_t at_delayCheck;
extern uint8_t ipDataSendFlag;
/**
  * @}
  */

/** @defgroup AT_PORT_Extern_Functions
  * @{
  */
extern void at_ipDataSending(uint8_t *pAtRcvData);
extern void at_ipDataSendNow(void);
/**
  * @}
  */



BOOL specialAtState = TRUE;
at_stateType  at_state;
uint8_t *pDataLine;
BOOL echoFlag = TRUE;

static uint8_t at_cmdLine[at_cmdLenMax];
uint8_t at_dataLine[at_dataLenMax];/////
//uint8_t *at_dataLine;

/** @defgroup AT_PORT_Functions
  * @{
  */

static void at_procTask(os_event_t *events);
//static void at_busyTask(os_event_t *events);
static void at_recvTask(os_event_t *events);

/**
  * @brief  Uart receive task.
  * @param  events: contain the uart receive data
  * @retval None
  */

#endif
#define MAX_LEN	20
enum
{
	RX_IDLE=0,
	RX_S1,
	RX_S2,
	RX_S3
};



volatile uint8 len = 0;
u8 buf[MAX_LEN];

volatile u8 rx_stats = RX_IDLE;
static u8 cnt=0;

extern void tri_key(void);
extern void tri_key_long(void);
extern void tri_rf_recv(uint8* buf);

static void ICACHE_FLASH_ATTR ///////
at_recvTask(os_event_t *events)
{
	uint8_t data;
	while(READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S))
	{
		//    temp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
		WRITE_PERI_REG(0X60000914, 0x73); //WTD

		//if(at_state != at_statIpTraning)
		{
			data = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
		  //if((temp != '\n') && (echoFlag))
		  {
			//uart_tx_one_char(temp); //display back
			//	os_printf("%02x ",data);
			//uart_tx_one_char(UART0, temp);
		  }
		}

		switch (rx_stats)
		{
			case RX_IDLE:
				if(data == 0x55)
					rx_stats = RX_S1;
			break;
			case RX_S1:
				if(data > MAX_LEN)
				{
					rx_stats = RX_IDLE;
					len = 0;
				}
				else
				{
					rx_stats = RX_S2;
					len = data;
					cnt = 0;
				}
			break;
			case RX_S2:
			buf[cnt++] = data;
			if(cnt == len)
			{
				int i;
				//yy_start(len,buf);
				if(len == 9)
				{
					//rf
					uint8 *p = (uint8 *)os_zalloc(8);
					os_memcpy(p,&buf[1],8);
					tri_rf_recv(p);
				}
				else if(len == 1)
				{
					//key
					if(buf[0] == 0x01)
						tri_key_long();
					else if(buf[0] == 0x02)
						tri_key();
					else if(buf[0] == 0x03)
						tri_reset_key();

				}
			#if 0
				//os_printf("get ");
				for(i=0;i<len;i++)
					os_printf("%02x ",buf[i]);
				os_printf("\n");
			#endif
				rx_stats = RX_IDLE;
				len = 0;
				cnt = 0;
			}
			break;
		}
	}

	  if(UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST))
	  {
	    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
	  }
	  else if(UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_TOUT_INT_ST))
	  {
	    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
	  }
	  ETS_UART_INTR_ENABLE();

}

void tri_at_recv(void)
{
	system_os_post(at_recvTaskPrio, 0, 0);
}
/**
  * @brief  Initializes build two tasks.
  * @param  None
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_init(void)
{
  system_os_task(at_recvTask, at_recvTaskPrio, at_recvTaskQueue, at_recvTaskQueueLen);
//  system_os_task(at_busyTask, at_busyTaskPrio, at_busyTaskQueue, at_busyTaskQueueLen);
//  system_os_task(at_procTask, at_procTaskPrio, at_procTaskQueue, at_procTaskQueueLen);
}

/**
  * @}
  */
