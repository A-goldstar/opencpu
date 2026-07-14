#define uart_c_

#include "uart.h"

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



//串口事件回调函数
static void uart0Event000000(void *param, uint32_t type)
{
    uartStruct *uart = ( uartStruct *)param;
    if (CM_UART_EVENT_TYPE_RX_ARRIVED & type)//收到接收事件
    {
		if (uart->osSemaphoreIdEvent != NULL)
		{
            osSemaphoreRelease(uart->osSemaphoreIdEvent);

            // osEventFlagsSet(uart->osEventFlagsIdUart, UART_EVENT_RECV);//发送事件
		}
    }
    if (CM_UART_EVENT_TYPE_RX_OVERFLOW & type)//收到溢出事件
	{
        if (uart->osSemaphoreIdEvent != NULL)
		{
            osSemaphoreRelease(uart->osSemaphoreIdEvent);

            // osEventFlagsSet(uart->osEventFlagsIdUart, UART_EVENT_RECV);//发送事件
		}
	}
}


//串口事件回调函数
static void uart1Event000000(void *param, uint32_t type)
{
    uartStruct *uart = ( uartStruct *)param;
    if (CM_UART_EVENT_TYPE_RX_ARRIVED & type)//收到接收事件
    {
		if (uart->osSemaphoreIdEvent != NULL)
		{
            osSemaphoreRelease(uart->osSemaphoreIdEvent);

            // osEventFlagsSet(uart->osEventFlagsIdUart, UART_EVENT_RECV);//发送事件
		}
    }
    if (CM_UART_EVENT_TYPE_RX_OVERFLOW & type)//收到溢出事件
	{
        if (uart->osSemaphoreIdEvent != NULL)
		{
            osSemaphoreRelease(uart->osSemaphoreIdEvent);

            // osEventFlagsSet(uart->osEventFlagsIdUart, UART_EVENT_RECV);//发送事件
		}
	}
}


//串口事件回调函数
static void uart2Event000000(void *param, uint32_t type)
{
    uartStruct *uart = ( uartStruct *)param;
    if (CM_UART_EVENT_TYPE_RX_ARRIVED & type)//收到接收事件
    {
		if (uart->osSemaphoreIdEvent != NULL)
		{
            osSemaphoreRelease(uart->osSemaphoreIdEvent);

            // osEventFlagsSet(uart->osEventFlagsIdUart, UART_EVENT_RECV);//发送事件
		}
    }
    if (CM_UART_EVENT_TYPE_RX_OVERFLOW & type)//收到溢出事件
	{
        if (uart->osSemaphoreIdEvent != NULL)
		{
            osSemaphoreRelease(uart->osSemaphoreIdEvent);

            // osEventFlagsSet(uart->osEventFlagsIdUart, UART_EVENT_RECV);//发送事件
		}
	}
}



//串口发送数据任务
static void uart0SendTask000000(void *param)
{
    uartStruct *uart = ( uartStruct *)param;

    osStatus_t status;
    uartSendStruct* uartSend;
    while(1)
    {
        status = osMessageQueueGet(uart->osMessageQueueIdUartSendData, &uartSend, NULL,osWaitForever);

        if (status == osOK)
        {
            cm_uart_write(uart->uartId, uartSend->data, uartSend->len, 1000);
            
            cm_free(uartSend->data);
            cm_free(uartSend);
        }
    }
}


//串口发送数据任务
static void uart1SendTask000000(void *param)
{
    uartStruct *uart = ( uartStruct *)param;

    osStatus_t status;
    uartSendStruct* uartSend;
    while(1)
    {
        status = osMessageQueueGet(uart->osMessageQueueIdUartSendData, &uartSend, NULL,osWaitForever);

        if (status == osOK)
        {
            cm_uart_write(uart->uartId, uartSend->data, uartSend->len, 1000);
            
            cm_free(uartSend->data);
            cm_free(uartSend);
        }
    }
}

//串口发送数据任务
static void uart2SendTask000000(void *param)
{
    uartStruct *uart = ( uartStruct *)param;

    osStatus_t status;
    uartSendStruct* uartSend;
    while(1)
    {
        status = osMessageQueueGet(uart->osMessageQueueIdUartSendData, &uartSend, NULL,osWaitForever);

        if (status == osOK)
        {
            cm_uart_write(uart->uartId, uartSend->data, uartSend->len, 1000);
            
            cm_free(uartSend->data);
            cm_free(uartSend);
        }
    }
}



/* 串口接收 */
static void uart0RecvTask000000(void *param)
{
	uartStruct *uart = ( uartStruct *)param;
	int  uartRecvLen=0;
	int len = 0;
    while (1)
    {
        uartRecvLen=0;
        //等待事件,                   


        if (uart->osSemaphoreIdEvent != NULL)
        {
            osSemaphoreAcquire(uart->osSemaphoreIdEvent, osWaitForever);//阻塞
        }

        // uint32_t flag = osEventFlagsWait(uart->osEventFlagsIdUart, UART_EVENT_RECV | UART_EVENT_FULL, osFlagsWaitAll, osWaitForever);
        uint32_t flag=1;
        if (flag>0)
        {
            len = cm_uart_read(uart->uartId, (void*)&uart->uartRecvBuff[uartRecvLen], uartRecvBuffLen - uartRecvLen, 1000);
            uartRecvLen += len;
            while (len!=0)//直到读取完
            {
                if (uartRecvLen < uartRecvBuffLen)
                {
                    osDelay(1);//延时
                    len = cm_uart_read(uart->uartId, (void*)&uart->uartRecvBuff[uartRecvLen], uartRecvBuffLen - uartRecvLen, 1000);
                    uartRecvLen += len;
                }
                else
                {
                    break;
                }
            }

            if (uart->uartRecvCb)
            {
                uart->uartRecvCb(flag, uart->uartRecvBuff, uartRecvLen );
            }
        }
    }
}



/* 串口接收 */
static void uart1RecvTask000000(void *param)
{
	uartStruct *uart = ( uartStruct *)param;
	int  uartRecvLen=0;
	int len = 0;
    while (1)
    {
        uartRecvLen=0;
        //等待事件,                                     
        if (uart->osSemaphoreIdEvent != NULL)
        {
            osSemaphoreAcquire(uart->osSemaphoreIdEvent, osWaitForever);//阻塞
        }

        // uint32_t flag = osEventFlagsWait(uart->osEventFlagsIdUart, UART_EVENT_RECV | UART_EVENT_FULL, osFlagsWaitAll, osWaitForever);
        uint32_t flag=1;
        if (flag>0)
        {
            len = cm_uart_read(uart->uartId, (void*)&uart->uartRecvBuff[uartRecvLen], uartRecvBuffLen - uartRecvLen, 1000);
            uartRecvLen += len;
            while (len!=0)//直到读取完
            {
                if (uartRecvLen < uartRecvBuffLen)
                {
                    osDelay(1);//延时
                    len = cm_uart_read(uart->uartId, (void*)&uart->uartRecvBuff[uartRecvLen], uartRecvBuffLen - uartRecvLen, 1000);
                    uartRecvLen += len;
                }
                else
                {
                    break;
                }
            }

            if (uart->uartRecvCb)
            {
                uart->uartRecvCb(flag, uart->uartRecvBuff, uartRecvLen );
            }
        }
    }
}



/* 串口接收 */
static void uart2RecvTask000000(void *param)
{
	uartStruct *uart = ( uartStruct *)param;
	int  uartRecvLen=0;
	int len = 0;
    while (1)
    {
        uartRecvLen=0;
        //等待事件,                                     
        if (uart->osSemaphoreIdEvent != NULL)
        {
            osSemaphoreAcquire(uart->osSemaphoreIdEvent, osWaitForever);//阻塞
        }

        // uint32_t flag = osEventFlagsWait(uart->osEventFlagsIdUart, UART_EVENT_RECV | UART_EVENT_FULL, osFlagsWaitAll, osWaitForever);
        uint32_t flag=1;
        if (flag>0)
        {
            len = cm_uart_read(uart->uartId, (void*)&uart->uartRecvBuff[uartRecvLen], uartRecvBuffLen - uartRecvLen, 1000);
            uartRecvLen += len;
            while (len!=0)//直到读取完
            {
                if (uartRecvLen < uartRecvBuffLen)
                {
                    osDelay(1);//延时
                    len = cm_uart_read(uart->uartId, (void*)&uart->uartRecvBuff[uartRecvLen], uartRecvBuffLen - uartRecvLen, 1000);
                    uartRecvLen += len;
                }
                else
                {
                    break;
                }
            }

            if (uart->uartRecvCb)
            {
                uart->uartRecvCb(flag, uart->uartRecvBuff, uartRecvLen );
            }
        }
    }
}



int uart_init(uartStruct *uart)
{
    int32_t ret = -1;

    /* 配置参数 */
    cm_uart_cfg_t config = 
    {
        uart->config.byte_size, 
        uart->config.parity,
        uart->config.stop_bit, 
        uart->config.flow_ctrl, 
        uart->config.baudrate,
        uart->config.is_lpuart
    };

    /* 事件参数 */
    uart->event.event_type = CM_UART_EVENT_TYPE_RX_ARRIVED|CM_UART_EVENT_TYPE_RX_OVERFLOW;   //注册需要上报的事件类型
    uart->event.event_param = uart;
    if (uart->uartId==CM_UART_DEV_0)
    {
        uart->event.event_entry = uart0Event000000;
    }
    else if (uart->uartId==CM_UART_DEV_1)
    {
        uart->event.event_entry = uart1Event000000;
    }
    else if (uart->uartId==CM_UART_DEV_2)
    {
        uart->event.event_entry = uart2Event000000;
    }
    else
    {
        return -1;
    }
    
    //创建信号量
    if (uart->osSemaphoreIdEvent==NULL)
    {
        uart->osSemaphoreIdEvent = osSemaphoreNew(1024000, 0, NULL);
        if (uart->osSemaphoreIdEvent==NULL) 
        {
            return -1;
        }
        cm_log_printf(0, "uart_init osSemaphoreIdEvent\n");
    }

    //发送消息队列
    if (uart->osMessageQueueIdUartSendData==NULL)
    {
        uart->osMessageQueueIdUartSendData = osMessageQueueNew(50,sizeof(uartSendStruct *),NULL);
        if (uart->osMessageQueueIdUartSendData == NULL) 
        {
            return -1;
        }
        cm_log_printf(0, "uart_init osMessageQueueIdUartSendData\n");
    }

    osThreadAttr_t uart_send_task_attr = {0};
    uart_send_task_attr.name  = "UartSendTask";
    uart_send_task_attr.stack_size = 4096 * 2;
    uart_send_task_attr.priority = osPriorityNormal;//增加其优先级


    if (uart->uartId==CM_UART_DEV_0 && (uart->flag&0x01)!=0x01)
    {
        osThreadNew(uart0SendTask000000, (void *)uart, &uart_send_task_attr);
        uart->flag=0x01;
        cm_log_printf(0, "uart_init uart0SendTask000000\n");
    }
    else if (uart->uartId==CM_UART_DEV_1 && (uart->flag&0x01)!=0x01)
    {
        osThreadNew(uart1SendTask000000, (void *)uart, &uart_send_task_attr);
        uart->flag=0x01;
        cm_log_printf(0, "uart_init uart1SendTask000000\n");
    }
    else if (uart->uartId==CM_UART_DEV_2 && (uart->flag&0x01)!=0x01)
    {
        osThreadNew(uart2SendTask000000, (void *)uart, &uart_send_task_attr);
        uart->flag=0x01;
        cm_log_printf(0, "uart_init uart2SendTask000000\n");
    }

    /* 配置引脚复用 */
	if (uart->uartId==CM_UART_DEV_0)
	{
		cm_iomux_set_pin_func(17,1);
		cm_iomux_set_pin_func(18,1);
	}
	else if (uart->uartId==CM_UART_DEV_1)
	{
		cm_iomux_set_pin_func(28,1);
		cm_iomux_set_pin_func(29,1);
	}
	else if (uart->uartId==CM_UART_DEV_2)
	{
		cm_iomux_set_pin_func(50,3);
		cm_iomux_set_pin_func(51,3);
	}
	

    /* 注册事件和回调函数 */
    ret = cm_uart_register_event(uart->uartId, &uart->event);
    if (ret != 0)
    {
        cm_log_printf(0, "uart register event err,ret=%d\n", ret);
        return -1;
    }
    /* 开启串口 */
    ret = cm_uart_open(uart->uartId, &config);
    
    // if (ret != 0)
    // {
    //     cm_log_printf(0, "uart init err,ret=%d\n", ret);
    //     return -1;
    // }

    /* 配置串口唤醒 */
    /* 只有UART0具有串口唤醒功能 */
    if (uart->uartId == CM_UART_DEV_0)
    {
        /* 配置uart唤醒功能，使能边沿检测才具备唤醒功能，仅主串口具有唤醒功能，用于唤醒的数据并不能被uart接收，请在唤醒后再进行uart数传 */
        cm_iomux_set_pin_cmd(CM_IOMUX_PIN_17, CM_IOMUX_PINCMD1_LPMEDEG, CM_IOMUX_PINCMD1_FUNC1_LPM_EDGE_RISE);
    }

    /* 串口接收*/
    osThreadAttr_t uart_task_attr = {0};
    uart_task_attr.name = "uart_task";
    uart_task_attr.stack_size = 4096 * 2;
    uart_task_attr.priority= osPriorityAboveNormal;

    if (uart->uartId==CM_UART_DEV_0 && (uart->flag&0x02)!=0x02)
    {
        osThreadNew(uart0RecvTask000000, (void *)uart, &uart_task_attr);
        uart->flag|=0x02;
        cm_log_printf(0, "uart_init uart0RecvTask000000\n");
    }
    else if (uart->uartId==CM_UART_DEV_1 && (uart->flag&0x02)!=0x02)
    {
        osThreadNew(uart1RecvTask000000, (void *)uart, &uart_task_attr);
        uart->flag|=0x02;
        cm_log_printf(0, "uart_init uart1RecvTask000000\n");
    }
    else if (uart->uartId==CM_UART_DEV_2 && (uart->flag&0x02)!=0x02)
    {
        osThreadNew(uart2RecvTask000000, (void *)uart, &uart_task_attr);
        uart->flag|=0x02;
        cm_log_printf(0, "uart_init uart2RecvTask000000\n");
    }

    return 0;
}


void uartSendData(uartStruct *uart,uint8_t *data,uint16_t len)
{
    uartSendStruct* uartSend = (uartSendStruct*)cm_malloc(sizeof(uartSendStruct));
    if (uartSend!=NULL)
    {
        char *data1 = (char*)cm_malloc(len+1);
        if (data1!=NULL)
        {
            memcpy(data1,data,len);
            uartSend->data = data1;
            uartSend->len = len;

            if (osMessageQueuePut(uart->osMessageQueueIdUartSendData ,&uartSend,0U,0U)!=osOK)
            {
                cm_free(uartSend);
            }
        }
        else
        {
            cm_free(uartSend);
        }
    }
}



void uartClose(uartStruct *uart)
{
    if(0 == cm_uart_close(uart->uartId))
    {
        cm_log_printf(0,"uart%d close is ok\n", uart->uartId);
    }
    else
    {
        cm_log_printf(0,"uart%d close is error\n", uart->uartId);
    }
}


