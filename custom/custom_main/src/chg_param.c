/*
 * @Author: tn
 * @Description: 封装sht30温湿度模块，使用iic进行通信，相当于hal库硬件iic，不需要自己写起始位，停止位等
 * 检测设备状态函数
 * @Date: 2025-03-25 15:18:38
 * @LastEditors: tn
 * @LastEditTime: 2025-05-26 14:57:49
 */
#include <stdint.h>
#include "cm_i2c.h"
#include "cm_os.h"
#include "chg_param.h"

#include "chg_module.h"
sht3x_struct sht3x;


/**
 * @brief 初始化 SHT3x 传感器
 *
 * @param [in] sht3x: SHT3x 传感器结构体
 * @param [in] addr: 传感器 I2C 地址
 * 
 * @return  
 *   = 0  - 成功.
 *   < 0  - 失败, 返回值为错误码.
 */
int32_t sht3x_init(sht3x_struct *sht3x, uint8_t addr) 
{
    sht3x->addr = addr;
    sht3x->temperature = -99.0f;
    sht3x->humidity = -99.0f;
    return 0;
}

/**
 * @brief 启动 SHT3x 传感器
 *
 * @param [in] sht3x: SHT3x 传感器结构体
 * 
 * @return  
 *   = 0  - 成功.
 *   < 0  - 失败, 返回值为错误码.
 * 1. 高重复性周期性采集模式

    0.5 Hz: {0x20, 0x32}
    1 Hz: {0x21, 0x30}
    2 Hz: {0x22, 0x36}
    4 Hz: {0x23, 0x34}
    10 Hz: {0x27, 0x37}

    2. 中重复性周期性采集模式

    0.5 Hz: {0x20, 0x24}
    1 Hz: {0x21, 0x26}
    2 Hz: {0x22, 0x20}
    4 Hz: {0x23, 0x22}
    10 Hz: {0x27, 0x21}

    3. 低重复性周期性采集模式

    0.5 Hz: {0x20, 0x2F}
    1 Hz: {0x21, 0x2D}
    2 Hz: {0x22, 0x2B}
    4 Hz: {0x23, 0x29}
    10 Hz: {0x27, 0x2A}

 */
int32_t sht3x_begin(sht3x_struct *sht3x) 
{
    uint8_t cmd[2] = { 0x27, 0x37 };  // 设置为周期性采集模式，高重复性
    int32_t ret = cm_i2c_write(SHT3X_I2C_ID, sht3x->addr, cmd, 2);
    if (ret < 0) {
        cm_log_printf(0,"SHT3x begin transmission failed: 0x%x addr:0X%x\n", ret, sht3x->addr);
        return ret;
    }
    return 0;
}

/**
 * @author: tn
 * @description: 初始化sht3x
 * @param {cm_i2c_dev_e} iic_id
 * @return {*}
 */
int32_t sht3x_open(cm_i2c_dev_e iic_id)
{
    cm_i2c_cfg_t config = {
        CM_I2C_ADDR_TYPE_7BIT,
        CM_I2C_MODE_MASTER,
        CM_I2C_CLK_100KHZ
    };

    sht3x_init(&sht3x, SHT3X_DEV_ADDR);

    cm_i2c_dev_e s_iic = iic_id;

    //启动i2c
    int32_t ret = cm_i2c_open(s_iic, &config);

    if (ret != 0) {
        osDelay(1000/5);//延时
    }
    else
    {
        osDelay(200);//延时
        sht3x_begin(&sht3x);
    }
    return 0;
}

/**
 * @brief 读取并计算 SHT3x 温湿度数据
 *
 * @param [in] sht3x: SHT3x 传感器结构体
 * 
 * @return  
 *   = 0  - 成功.
 *   < 0  - 失败, 返回值为错误码.
 */
int32_t sht3x_measure(sht3x_struct *sht3x) 
{
    uint8_t cmd[2] = {0xE0, 0x00};
    uint8_t rx_bytes[6] = {0};
    int32_t ret = cm_i2c_write(SHT3X_I2C_ID, sht3x->addr, cmd, 2);
    if (ret < 0) {
        cm_log_printf(0,"SHT3x measure command failed: 0x%x\n", ret);
        return ret;
    }

    osDelay(100/5);//等待数据准备好
    ret = cm_i2c_read(SHT3X_I2C_ID, sht3x->addr, rx_bytes, 6);
    if (ret < 0) {
        cm_log_printf(0,"SHT3x read data failed: 0x%x\n", ret);
        return ret;
    }

    uint16_t t_ticks = ((uint16_t)rx_bytes[0] << 8) | rx_bytes[1];
    uint16_t rh_ticks = ((uint16_t)rx_bytes[3] << 8) | rx_bytes[4];
    sht3x->temperature = -45.0f + (175.0f * (t_ticks / 65535.0f));
    sht3x->humidity = 100.0f * (rh_ticks / 65535.0f);

    return 0;
}

/**
 * @author: tn
 * @description: 温湿度读取入口函数
 * 
 * 继电器的反向电流，会影响iic通信
 * @return {*}
 */
void param_task_entry(void)
{
    static uint err = 0;
    static char data_temp[255];
    static char data_humi[255];
    int32_t ret = sht3x_open(CM_I2C_DEV_1);
    if(ret != 0)
    {
        sysPrintf("i2c init err, ret = %d\n", ret);
    }
    else
    {
        sysPrintf("Temp_humi_i2c open", ret);
    }
    osDelay(500); 
    while (1)
    {
        ret = sht3x_measure(&sht3x);// 读取并打印传感器数据
        if (ret != 0) 
        {//异常处理
            sysPrintf("Failed to read from SHT3x\n");
            if(++err>1)
            {
                cm_i2c_close(CM_I2C_DEV_1);
                int32_t ret = sht3x_open(CM_I2C_DEV_1);
                if(ret != 0)
                {
                    sysPrintf("i2c init err, ret = %d\n", ret);
                }
                else
                {
                    sysPrintf("Temp_humi_i2c open", ret);
                }
                err = 0;
                continue;//跳过本次循环
            }
            
        }
        else
        {
            if(chg_mqtt==1)
            {//充电上线后，推送进消息队列
                if(temp_shell == 0)
                {
                    // sprintf(data_temp,"{\"method\":\"thing.service.property.set\",\"id\":\"\",\"params\":{\"temperature\":%.2f},\"version\":\"1.0.0"}",sht3x.temperature);
                    sprintf(data_temp,"{\"params\":{\"temperature\":%.2f}}",sht3x.temperature);
                    SendMqttMessQueue(data_temp);//推送数据到消息队列
                    osDelay(100/5);
                }
                else
                {
                    sprintf(data_temp,"{\"params\":{\"temperature\":60.00}}");
                    SendMqttMessQueue(data_temp);//推送数据到消息队列
                    osDelay(100/5);
                }
                if(humi_shell == 0)
                {
                    // sprintf(data_humi,"{\"method\":\"thing.service.property.set\",\"id\":\"\",\"params\":{\"Humidity\":%.2f},\"version\":\"1.0.0"}",sht3x.humidity);
                    sprintf(data_humi,"{\"params\":{\"Humidity\":%.2f}}",sht3x.humidity);
                    SendMqttMessQueue(data_humi);//推送数据到消息队列
                    osDelay(100/5);
                }
                else
                {
                    sprintf(data_humi,"{\"params\":{\"Humidity\":90}}");
                    SendMqttMessQueue(data_humi);//推送数据到消息队列
                    osDelay(100/5);
                }
            }
            /*测试区*/
            // sysPrintf(data_temp,"{\"params\":{\"temperature\":%.2f}}",sht3x.temperature);
            // sysPrintf(data_humi,"{\"params\":{\"Humidity\":%.2f}}",sht3x.humidity);
            if(param_shell == 1)
            {
                sysPrintf("Temperature: %.2f C, Humidity: %.2f %%\r\n",sht3x.temperature, sht3x.humidity);
            }
        }
        osDelay(1000/5); 
    }
}