/*
 * @Author: tn
 * @Description: main.h
 * @Date: 2024-05-19 22:50:10
 * @LastEditors: tn
 * @LastEditTime: 2025-05-25 16:16:00
 */
/*********************************************************
*  @file    cm_main.h
*  @brief   ML302 OpenCPU main header file
*  Copyright (c) 2019 China Mobile IOT.
*  All rights reserved.
*  created by XieGangLiang 2019/10/08
********************************************************/
#ifndef __CM_MAIN_H__
#define __CM_MAIN_H__

#include "cm_os.h"

osMutexId_t xDataMutex;//定义互斥信号量
/*
 请在后面添加应用任务ID
 */
enum
{
	tWdt_id = 1,
	tModule_id,
    tShell_id,
    tnfc_id,
    tparam_id,
    tctrl_id,
    tmeter_id,
    taudio_id,
	tApp_id,
    
};

#define tWdt_pri            osPriorityNormal
#define tModule_pri         osPriorityNormal
#define tShell_pri          osPriorityNormal
#define tnfc_pri            osPriorityAboveNormal
#define tparam_pri          osPriorityNormal
#define tctrl_pri           osPriorityNormal
#define tmeter_pri          osPriorityNormal
#define taudio_pri          osPriorityNormal

/**
 * \struct tsk_cfg
 * 任务创建参数结构体定义.
 * 用于传入给任务创建函数.
 */
typedef struct
{
    unsigned int   id;         /**< 任务id         */
    unsigned int   pri;        /**< 任务优先级     */
    osThreadId_t   threadid;    /**< 任务堆栈地址   */
    char           *name;      /**< 任务名称   */
    uint32_t       tsksize;      /**< 任务堆栈大小   */
    void           *taskp;      /**< 任务入口函数指针*/
    osThreadAttr_t handle;     /**< 任务句柄   */
}TSK_CFG_t;

extern TSK_CFG_t gTaskList[tApp_id];//任务参数
unsigned char OsTaskGetName(char *name);
unsigned char OsTaskGetCurrentId(void);
unsigned char OsTaskGetId(char *name);

#endif
