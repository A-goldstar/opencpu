/*
 * @Author: tn
 * @Description: 
 * @Date: 2025-03-25 15:18:53
 * @LastEditors: tn
 * @LastEditTime: 2025-03-28 20:19:54
 */
#ifndef _CHG_PARAM_H__
#define _CHG_PARAM_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

#define sht3x_cx_
/*******************************************************************************
 * Include files
 ******************************************************************************/

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
#include "cm_i2c.h"


#define SHT3X_DEV_ADDR      0x44  // SHT3x 传感器 I2C 地址
#define SHT3X_I2C_ID        CM_I2C_DEV_1

typedef struct {
    uint8_t addr;
    float temperature;
    float humidity;
} sht3x_struct;

sht3x_cx_ sht3x_struct sht3x;

int32_t sht3x_init(sht3x_struct *sht3x, uint8_t addr) ;
int32_t sht3x_begin(sht3x_struct *sht3x);
int32_t sht3x_measure(sht3x_struct *sht3x);
void param_task_entry(void);

#ifdef __cplusplus
}
#endif

#endif /*  */