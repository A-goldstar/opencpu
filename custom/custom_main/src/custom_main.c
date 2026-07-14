/*
 * @Author: tn
 * @Description: 
 * @Date: 2024-05-19 22:50:10
 * @LastEditors: tn
 * @LastEditTime: 2025-05-27 11:18:38
 */
#include "stdio.h"
#include "stdlib.h"
#include "custom_main.h"
#include "cm_os.h"
#include "cm_fs.h"
#include "string.h"
#include "chg_struct.h"
#include "chg_module.h"
#include "chg_nfc.h"
#include "chg_param.h"
#include "chg_ctrl.h"
#include "chg_meter.h"
#include "chg_audio.h"
#include "shell_task.h"

/*请在此设定任务参数*/
TSK_CFG_t gTaskList[tApp_id] =
{
    {tWdt_id,		tWdt_pri,		0,		"wdt",		1024 * 2,  0,  {0}},//这个任务并没有创建，用于看门狗，以后优化
    {tModule_id,	tModule_pri,	0,		"module",   1024 * 6,  module_task_entry,       {0}},
    {tShell_id,     tShell_pri,     0,      "shell",    1024 * 4,  shell_task_entry,        {0}},
    {tnfc_id,       tnfc_pri,       0,      "nfc",      1024 * 2,  nfc_task_entry,          {0}},
    {tparam_id,     tparam_pri,     0,      "param",    1024 * 2,  param_task_entry,        {0}},
    {tctrl_id,      tctrl_pri,      0,      "ctrl",     1024 * 2,  ctrl_task_entry,         {0}},
    {tmeter_id,     tmeter_pri,     0,      "meter",    1024 * 2,  meter_task_entry,        {0}},
    {taudio_id,     taudio_pri,     0,      "audio",    1024 * 4,  audio_task_entry,        {0}},
    
    {0,0,0,0,0,0,{0}}
};

/**
 * @author: tn
 * @description: 根据当前线程ID查找任务名称
 * @param {char} *name
 * @return {*}
 */
unsigned char OsTaskGetName(char *name)
{
    for(int i=0; i<tApp_id; i++) {
        if(gTaskList[i].threadid == osThreadGetId())
        {
            memcpy(name,gTaskList[i].name,strlen(gTaskList[i].name));
            return gTaskList[i].id;
        }
    }
    return 0;
}

/**
 * @author: tn
 * @description: 通过任务查找ID
 * @param {char} *name
 * @return {*}
 */
unsigned char OsTaskGetId(char *name)
{
    for(int i=1; i<tApp_id; i++)
    {
        if (memcmp(gTaskList[i].name, name, strlen(name)) == 0)
        {
            return gTaskList[i].id;
        }
    }
    return 0;
}

/**
 * @author: tn
 * @description: 获取当前运行任务的ID
 * @return {*}
 */
unsigned char OsTaskGetCurrentId(void)
{
    for(int i=0; i<tApp_id; i++) {
        if(gTaskList[i].threadid == osThreadGetId())
            return gTaskList[i].id;
    }
    return 0;
}

/**
 * @author: tn
 * @description: RTOS 的核心原则：任务必须主动释放 CPU
 * 空闲任务是在所有任务进入阻塞的时候才运行，不需要创建
 * 高优先级任务抢占：收到信号量和消息队列等会抢占，或者等待低优先级释放
 * osdelay的意思是从阻塞态进入就绪态，进入就绪态之后还有进行时间片轮转，或者高优先级之间抢占，osdelay只是表示进入就绪态的时间
 * 请注意osDelay挂起时间，以及不同任务的优先级
 * @return {*}
 */
/*主任务*/
static osThreadId_t OC_APP_TaskHandle = NULL;
static void my_appimg_enter(char *param)
{
    CreateQueueTask();//创建消息队列
    uart0Recv_init();//初始化串口
    
    char buf[65] = {0};
    cm_fs_system_info_t info = {0, 0};
    cm_heap_stats_t stats = {0};
    sysPrintf("\n\n\n\n\n\n\n\n\n\n");
    sysPrintf("CM OpenCPU Starts\n");
    cm_sys_get_cm_ver(buf, CM_VER_LEN);
    sysPrintf("SDK VERSION:%s\n", buf);
    cm_fs_getinfo(&info);
    cm_mem_get_heap_stats(&stats);
    sysPrintf("fs total:%d,remain:%d\n", info.total_size, info.free_size);
    sysPrintf("heap total:%d,remain:%d\n",stats.total_size,stats.free);

    sysPrintf("waiting for device open...\n");

    // while(1)
    // {//申请pdp驻网，未完成后续优化
    //     if(pdp_time_out>10)
    //     {
    //         sysPrintf("network timeout\n");
    //         break;
    //     }
    //     if(cm_modem_get_pdp_state(1) == 1)
    //     {
    //         sysPrintf("network ready\n");        
    //         break;
    //     }
    //     osDelay(200);   
    //     pdp_time_out++;
    // }

    for(int i=0;i<tApp_id;i++)
    {//遍历，创建所有任务
        if(gTaskList[i].taskp!=0)
        {
            gTaskList[i].handle.name  = gTaskList[i].name;//任务名字
            gTaskList[i].handle.stack_size = gTaskList[i].tsksize;//任务使用栈大小-写这个就可以
            gTaskList[i].handle.priority = gTaskList[i].pri;//任务优先级
            gTaskList[i].threadid = osThreadNew(gTaskList[i].taskp,0,&gTaskList[i].handle);
        }
    }
    while (1)
    {
		osDelay(100);//主任务完全让出CPU，如果不加延时会导致主任务一直运行，其他任务卡死
    }
}

/**
 * @author: tn
 * @description: 创建一个主任务mian_task，用于初始化其他所有任务，可以保证资源隔离，确保所有的子任务共享资源
	通过osThreadNew创建一个正式的任务（main_task），用于执行my_appimg_enter，之后执行while(1),里面让出了CPU资源
    通过使用互斥锁增加同步机制，确保任何时候都只有一个任务能访问共享资源
        osStatus_t status = osMutexAcquire(xDataMutex,osWaitForever);//获取锁
        if(status == osOK)
        {

        }
        status = osMutexRelease(xDataMutex);//释放锁
    * @return {*}
 */
//相当于程序的main函数
int cm_opencpu_entry(char * param)
{
    // xDataMutex = osMutexNew(NULL);//创建互斥锁
    // if(xDataMutex==NULL)
    // {
    //     return -1;
    // }

	osThreadAttr_t app_task_attr = {0};
    app_task_attr.name  = "main_task";
    app_task_attr.stack_size = 1024 * 4;
    app_task_attr.priority = osPriorityAboveNormal;
    OC_APP_TaskHandle = osThreadNew((osThreadFunc_t)my_appimg_enter, 0, &app_task_attr);
	return 0;
} 
