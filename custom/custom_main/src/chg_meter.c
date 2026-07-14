/*
 * @Author: tn
 * @Description: 封装计量芯片函数
 * @Date: 2025-03-31 16:34:17
 * @LastEditors: tn
 * @LastEditTime: 2025-07-01 20:45:38
 */
#include "cm_gpio.h"
#include "cm_iomux.h"
#include "cm_os.h"
#include "chg_meter.h"
#include "chg_module.h"
#include "uart.h"
#include "chg_struct.h"
#include "custom_main.h"

uartStruct uart1={0};//定义串口结构体变量

uint8_t Rx_Buffer[37];//定义计量芯片全局变量
unsigned char Tx_Buffer[8];
unsigned char read_enable,receive_finished,reveive_number;
unsigned long Voltage_data,Current_data,Power_data,Energy_data,Pf_data,CO2_data;
float voltage, current, power, energy, pf, co2;
static char data_voltage[255];
static char data_current[255];
static char data_power[255];

/**
 * @author: tn
 * @description: 
 * @param {unsigned char} crcbuf
 * @param {unsigned int} crc
 * @return {*}
 */
unsigned int calccrc(unsigned char crcbuf,unsigned int crc) 
{ 
    unsigned char i; 
    unsigned char chk; 
    crc=crc ^ crcbuf; 
    for(i=0;i<8;i++) 
    { 
        chk=( unsigned char)(crc&1); 
        crc=crc>>1; 
        crc=crc&0x7fff; 
        if (chk==1)
        crc=crc^0xa001; 
        crc=crc&0xffff; 
    } 
        return crc; 
}

/**
 * @author: tn
 * @description: 
 * @param {unsigned char} *buf
 * @param {unsigned char} len
 * @return {*}
 */
unsigned int chkcrc(unsigned char *buf,unsigned char len)
{ 
    unsigned char hi,lo; 
    unsigned int i; 
    unsigned int crc; 
    crc=0xFFFF; //初始化CRC寄存器为0xFFFF
    for(i=0;i<len;i++) 
    {//遍历每个字节
        crc=calccrc(*buf,crc); 
        buf++; 
    }
    hi=( unsigned char)(crc%256);//获取低字节
    lo=( unsigned char)(crc/256);//获取高字节
    crc=(((unsigned int)(hi))<<8)|lo;//高低字节交换
    return crc; 
}


/**
 * @author: tn
 * @description: 
 * @return {*}
 */
void read_data(void)
{//用于更新数据
    union crcdata
    { 
        unsigned int word16;
        unsigned char byte[2];
    }crcnow; 
    if(read_enable==1) // 到时间抄读模块，抄读间隔 1 秒钟(或其他) 
    { 
        read_enable=0; 
        // Tx_Buffer[0]=Read_ID; //模块的 ID 号，默认 ID 为 0x01 
        Tx_Buffer[0]=0x01; //模块的 ID 号，默认 ID 为 0x01 
        Tx_Buffer[1]=0x03; 
        Tx_Buffer[2]=0x00; 
        Tx_Buffer[3]=0x48; 
        Tx_Buffer[4]=0x00; 
        // Tx_Buffer[5]=0x06;
        Tx_Buffer[5]=0x08; //数据长度
        crcnow.word16=chkcrc(Tx_Buffer,6); 
        Tx_Buffer[6]=crcnow.byte[1]; //CRC 效验低字节在前
        Tx_Buffer[7]=crcnow.byte[0]; 
        //Send_data(8); //发送 8 个数据，请根据单片机类型自己编程
        meterPrintf(Tx_Buffer,8);
    } 
}

void Analysis_data(void)
{ 
    //unsigned char i;
    union crcdata
    { 
        unsigned int word16;
        unsigned char byte[2];
    }crcnow; 
    if(receive_finished==1) //接收完成
    { 
        receive_finished=0; 
        // if(data[0]==Read_ID) //确认 ID 正确
        if(Rx_Buffer[0]==0x01) //确认 ID 正确
        { 
            crcnow.word16=chkcrc(Rx_Buffer,reveive_number-2); //reveive_numbe 是接收数据总长度
            // if((crcnow.byte[0]==data[reveive_number-1])&&(crcnow.byte[1]==data[reveive_number-2]))
            // //确认 CRC 校验正确，校验失败
            // { 
                // Voltage_data=(((unsigned long)(data[3]))<<24)|(((unsigned long)(data[4]))<<16)|(((unsigned long)(data[5]))<<8)|data[6];
                // Current_data=(((unsigned long)(data[7]))<<24)|(((unsigned long)(data[8]))<<16)|(((unsigned long)(data[9]))<<8)|data[10]; 
                // Power_data=(((unsigned long)(data[11]))<<24)|(((unsigned long)(data[12]))<<16)|(((unsigned long)(data[13]))<<8)|data[14]; 
                // Energy_data=(((unsigned long)(data[15]))<<24)|(((unsigned long)(data[16]))<<16)|(((unsigned long)(data[17]))<<8)|data[18]; 
                // Pf_data=(((unsigned long)(data[19]))<<24)|(((unsigned long)(data[20]))<<16)|(((unsigned long)(data[21]))<<8)|data[22]; 
                // CO2_data=(((unsigned long)(data[23]))<<24)|(((unsigned long)(data[24]))<<16)|(((unsigned long)(data[25]))<<8)|data[26]; 
            // } 
        } 
    } 
}

/**
 * @author: tn
 * @description: 封装处理接收到的数据
 * @return {*}
 */
void rev_data(void)
{
    voltage = Voltage_data / 10000.0f;      // 假设电压单位为毫伏，原始数据为毫伏
    current = Current_data / 10.0f;      // 假设电流单位为毫安，原始数据为毫安
    // power = Power_data / 10.0f;            // 假设功率单位为瓦，原始数据为0.1瓦
    power = voltage*current;                //单位为毫瓦
    energy = Energy_data / 10000.0f;         // 假设能量单位为千瓦时，原始数据为0.01kWh
    // Analysis_data();
    if(chg_mqtt == 1)
    {
        if(chg_ctrl == 1)
        {
            // sprintf(data_voltage,"{\"params\":{\"voltage\":%.2f}}",voltage);
            sprintf(data_voltage,"{\"method\":\"thing.service.property.set\",\"id\":\"\",\"params\":{\"voltage\":%.1f},\"version\":\"1.0.0\"}",voltage);
            SendmeterMessQueue(data_voltage);//推送电压数据到消息队列
            osDelay(25/5);
            // sprintf(data_current,"{\"params\":{\"current\":%.2f}}",current);
            sprintf(data_current,"{\"method\":\"thing.service.property.set\",\"id\":\"\",\"params\":{\"current\":%.1f},\"version\":\"1.0.0\"}",current);
            SendmeterMessQueue(data_current);//推送电流数据到消息队列
            osDelay(25/5);
            // sprintf(data_power,"{\"params\":{\"power\":%.2f}}",power);
            sprintf(data_power,"{\"method\":\"thing.service.property.set\",\"id\":\"\",\"params\":{\"power\":%.1f},\"version\":\"1.0.0\"}",power);
            SendmeterMessQueue(data_power);//推送电压数据到消息队列
            osDelay(25/5);
            if(meter_shell == 1)
            {
                sysPrintf("V=%.1fV i=%.1fmA w=%.1fmW p=%.3fKWh\r\n",voltage,current,power,energy);
            }
        }
        else
        {
            sprintf(data_voltage,"{\"method\":\"thing.service.property.set\",\"id\":\"\",\"params\":{\"voltage\":%.0f},\"version\":\"1.0.0\"}",0.0f);
            SendmeterMessQueue(data_voltage);//推送电压数据到消息队列
            osDelay(25/5);
            // sprintf(data_current,"{\"params\":{\"current\":%.2f}}",current);
            sprintf(data_current,"{\"method\":\"thing.service.property.set\",\"id\":\"\",\"params\":{\"current\":%.0f},\"version\":\"1.0.0\"}",0.0f);
            SendmeterMessQueue(data_current);//推送电流数据到消息队列
            osDelay(25/5);
            // sprintf(data_power,"{\"params\":{\"power\":%.2f}}",power);
            sprintf(data_power,"{\"method\":\"thing.service.property.set\",\"id\":\"\",\"params\":{\"power\":%.0f},\"version\":\"1.0.0\"}",0.0f);
            SendmeterMessQueue(data_power);//推送电压数据到消息队列
            osDelay(25/5);
        }
    }
}

/**
 * @author: tn
 * @description: 只支持二进制发送
 * @param {uint8_t} *data 数据指针
 * @param {uint16_t} length 数据长度
 * @return {*}1=成功, 0=失败
 */
uint8_t meterPrintf(uint8_t *data, uint16_t length) 
{
  if (data == NULL || length == 0) 
  {
      return 0; // 无效参数
  }
    uartSendData(&uart1, (uint8_t *)data, length); // 直接发送原始数据
  return 1;
}

/**
 * @author: tn
 * @description: 数据接收函数，用于接收计量模块的回传数据
 * @param {uint32_t} flags
 * @param {char} *data
 * @param {int} len
 * @return {*}
 * 波特率为4800会导致37位数据被拆封成32位和5位，需要修改成115200
 * const 定义一个常量，不能使用const会导致数据无法修改
 * 37个字节过多，无法全部传递，不能使用互斥锁加全局变量的形式
 * 
 * 
 * 
 * 无法解决转运函数37位失败的问题，只能转运32位，只取前面27位
 * 使用115200，可以转运成功，目前认为是任务切换导致失败
 * 
 * 这个接收函数的data不能传输到这个函数外，否则会问题
 */
void uart1RecvData(uint32_t flags, char *data, int len)
{
    if(data[0]==0x01) //确认 ID 正确
    { 
        /*将其放入其中为无奈之举，接收到的数据无法转运出去，未发现具体原因*/
        Voltage_data=(((unsigned long)(data[3]))<<24)|(((unsigned long)(data[4]))<<16)|(((unsigned long)(data[5]))<<8)|data[6];
        Current_data=(((unsigned long)(data[7]))<<24)|(((unsigned long)(data[8]))<<16)|(((unsigned long)(data[9]))<<8)|data[10]; 
        Power_data=(((unsigned long)(data[11]))<<24)|(((unsigned long)(data[12]))<<16)|(((unsigned long)(data[13]))<<8)|data[14]; 
        Energy_data=(((unsigned long)(data[15]))<<24)|(((unsigned long)(data[16]))<<16)|(((unsigned long)(data[17]))<<8)|data[18]; 
    }

    // uartSendData(&uart1, (uint8_t *)data, len);//测试代码
    // Pf_data=(((unsigned long)(data[19]))<<24)|(((unsigned long)(data[20]))<<16)|(((unsigned long)(data[21]))<<8)|data[22]; 
    // CO2_data=(((unsigned long)(data[23]))<<24)|(((unsigned long)(data[24]))<<16)|(((unsigned long)(data[25]))<<8)|data[26]; 
    // memcpy(data,(uint8_t *) data, 27);//这个数据转运会失败，具体原因未知
    // memset(data, 0, sizeof(* data));//清空data
}

/**
 * @author: tn
 * @description: 配置串口1，用于与电能计量模块通信，波特率为4800
 * @return {*}
 */
uint8_t uart1Recv_init()
{  
  //配置串口
  uart1.uartId = 1;//配置串口号
  uart1.uartRecvCb = uart1RecvData;//设置接收数据函数
  uart1.config.baudrate = 9600;//波特率
  uart1.config.byte_size = CM_UART_BYTE_SIZE_8;//数据位数
  uart1.config.flow_ctrl = CM_UART_FLOW_CTRL_NONE;//硬件流控
  uart1.config.parity = CM_UART_PARITY_NONE;//奇偶校验
  uart1.config.stop_bit = CM_UART_STOP_BIT_ONE;//停止位
  uart1.config.is_lpuart = 0;//若要配置为低功耗模式可改为1
  if (uart_init(&uart1) !=0)//初始化串口
  {
      return -1;
  }
  return 0;
}

 /**
  * @author: tn
  * @description: 计量芯片入口函数，使用串口通信，波特率4800
  * @return {*}
  */
 void meter_task_entry(void)
 {
    uart1Recv_init();
    osDelay(500);
    sysPrintf("meter_uart open");
    while (1)
    {
        read_enable=1; //到时间抄读
        read_data();//发送定时开始
        osDelay(200);
        rev_data();

        /*测试代码*/
        // meterPrintf("hello_tn\r");
        osDelay(1000/5); 
    }
 }