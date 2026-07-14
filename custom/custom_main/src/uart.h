/*
 * @Author: tn
 * @Description: 
 * @Date: 2025-03-10 20:18:29
 * @LastEditors: tn
 * @LastEditTime: 2025-04-02 20:40:20
 */
#ifndef uart_c_
#define uart_c_ extern
#else
#define uart_c_
#endif

#ifndef uart_h_
#define uart_h_

#include "cm_sys.h"
#include "cm_os.h"
#include "cm_mem.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include <string.h>
#include "cm_gpio.h"
#include "cm_iomux.h"
#include "cm_uart.h"



#define UART_EVENT_RECV 0x01  //串口接收到数据事件标志
#define UART_EVENT_FULL 0x02  //串口接收缓存区满事件标志

typedef struct{
  char *data;
  uint32_t len;
}uartSendStruct;


#define uartRecvBuffLen 1460//串口缓存数据长度
// #define uartRecvBuffLen 2920//串口缓存数据长度
typedef struct{
    cm_uart_cfg_t config;
    osSemaphoreId_t osSemaphoreIdEvent;
    osMessageQueueId_t osMessageQueueIdUartSendData;//发送数据队列
    cm_uart_dev_e uartId;//串口号
    char uartRecvBuff[uartRecvBuffLen];//缓存串口接收的数据
    void (*uartRecvCb) (uint32_t flags,char *data, int len);//串口接收数据回调函数
    cm_uart_event_t event;
    char flag;
}uartStruct;


int uart_init(uartStruct *uart);
void uartSendData(uartStruct *uart,uint8_t *data,uint16_t len);
void uartClose(uartStruct *uart);

#endif


