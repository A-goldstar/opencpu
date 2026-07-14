/*
 * @Author: TN
 * @Date: 2025-04-28 13:27:29
 * @LastEditTime: 2025-05-25 17:52:35
 * @FilePath: \opencpu\custom\custom_main\src\shell_task.c
 * @Description: 
 */
#include "cm_gpio.h"
#include "cm_iomux.h"
#include "cm_os.h"
#include "chg_ctrl.h"
#include "shell_task.h"
#include "chg_module.h"
#include "cm_pm.h"
#include "cm_mem.h"
#include "stdio.h"
#include "string.h"
#include "chg_struct.h"
#include "chg_dev.h"
#include <stdlib.h>

extern uint8_t Step;	// 数字电位器范围为0-127

/*****************命令函数*****************/
int RebootFun(void *cmdbuf);
int paramshow(void* cmdbuf);
int nfcshow(void* cmdbuf);
int metershow(void* cmdbuf);
int chgctrl(void* cmdbuf);
int chgdevctrl(void* cmdbuf);
int testctrl(void* cmdbuf);

static int shellpassed = 0;  /**< shell密码是否校验 0已经校验 1未校验  */
static Shell_List_t *m_cmd_list_p = (Shell_List_t *)0;
static char cmd_buf[255]="\r";

const shell_cmd_cfg shell_cmd_list[ ] =
{
    {"reboot",               RebootFun,            "重启"},
    {"ctrl" ,                chgctrl ,             "控制 ctrl help"},
    {"param" ,               paramshow ,           "打印温湿度 param help"},
    {"meter" ,               metershow ,           "打印电量 meter help"},
    {"nfc" ,                 nfcshow ,             "NFC复位"},
    {"chg" ,                 chgdevctrl ,          "开关控制"},
    {"test" ,                testctrl ,            "测试报警"},
    { 0,		0 },
};

/**************************命令实现*******************************/
/**
 * @author: tn
 * @description: 温湿度打印
 * @param {void*} cmdbuf
 * @return {*}
 */
int paramshow(void* cmdbuf)
{
    char valuestr1[20]={0};
    char valuestr2[20]={0};
    sscanf(cmdbuf,"%*s %s %s",valuestr1,valuestr2);//跳过第一个字符串提取后面两个
    if(strncmp("open", valuestr1, strlen(valuestr1)) == 0)
    {
        param_shell = 1;
        sysPrintf("开启温湿度打印\r\n");
    }
    else if(strncmp("close", valuestr1, strlen(valuestr1)) == 0)
    {
        param_shell = 0;
        sysPrintf("关闭温湿度打印\r\n");
    }
    else
    {
        sysPrintf("输入命令无效\r\n");
    }
  return 0;
}

/**
 * @author: tn
 * @description: 测试报警函数
 * @param {void*} cmdbuf
 * @return {*}
 */
int testctrl(void* cmdbuf)
{
    char valuestr1[20]={0};
    char valuestr2[20]={0};
    sscanf(cmdbuf,"%*s %s %s",valuestr1,valuestr2);//跳过第一个字符串提取后面两个
    if(strncmp("temp", valuestr1, strlen(valuestr1)) == 0)
    {
        temp_shell = 1;
        sysPrintf("开启温度测试\r\n");
    }
    else if(strncmp("humi", valuestr1, strlen(valuestr1)) == 0)
    {
        humi_shell = 1;
        sysPrintf("开启湿度测试\r\n");
    }
    else if(strncmp("close", valuestr1, strlen(valuestr1)) == 0)
    {
        temp_shell = 0;
        humi_shell = 0;
        sysPrintf("关闭所有测试\r\n");
    }
    else
    {
        sysPrintf("输入命令无效\r\n");
    }
  return 0;
}

int nfcshow(void* cmdbuf)
{
    char valuestr1[20]={0};
    char valuestr2[20]={0};
    sscanf(cmdbuf,"%*s %s %s",valuestr1,valuestr2);//跳过第一个字符串提取后面两个
    if(strncmp("reboot", valuestr1, strlen(valuestr1)) == 0)
    {
        nfc_shell = 1;
        // sysPrintf("NFC复位\r\n");
    }
    else
    {
        sysPrintf("输入命令无效\r\n");
    }
  return 0;
}

int chgdevctrl(void* cmdbuf)
{
    char valuestr1[20]={0};
    char valuestr2[20]={0};
    sscanf(cmdbuf,"%*s %s %s",valuestr1,valuestr2);//跳过第一个字符串提取后面两个
    if(strncmp("open", valuestr1, strlen(valuestr1)) == 0)
    {
        chg_ctrl=1;//启动充电
        audio_play=2;//设备启动成功
        eeprom_addr();
        sysPrintf("shell开启充电\r\n");
    }
    else if(strncmp("close", valuestr1, strlen(valuestr1)) == 0)
    {
        chg_ctrl = 0;
        audio_play=5;
        eeprom_del();
        sysPrintf("shell关闭充电\r\n");
    }
    else if(strncmp("read", valuestr1, strlen(valuestr1)) == 0)
    {
        int8_t r_data[5] = {};
        driver_eeprom_read(0x0000, r_data,1);
        osDelay(100);
        sysPrintf("i2c read e2prom: %d  ",r_data[0]);
    }
    else
    {
        sysPrintf("输入命令无效\r\n");
    }
  return 0;
}

/**
 * @author: tn
 * @description: 计量打印
 * @param {void*} cmdbuf
 * @return {*}
 */
int metershow(void* cmdbuf)
{
    char valuestr1[20]={0};
    char valuestr2[20]={0};
    sscanf(cmdbuf,"%*s %s %s",valuestr1,valuestr2);//跳过第一个字符串提取后面两个
    if(strncmp("open", valuestr1, strlen(valuestr1)) == 0)
    {
        meter_shell = 1;
        sysPrintf("开启计量打印\r\n");
    }
    else if(strncmp("close", valuestr1, strlen(valuestr1)) == 0)
    {
        meter_shell = 0;
        sysPrintf("关闭计量打印\r\n");
    }
    else
    {
        sysPrintf("输入命令无效\r\n");
    }
  return 0;
}

/**
 * @Author: TN
 * @description: 重启
 * @param {void} *cmdbuf
 * @return {*}
 */
int RebootFun(void *cmdbuf)
{
    char type[16];
    int len = sscanf((char const *)cmdbuf, "%*s %15s", type);
    if(len == 1)
    {
        sysPrintf("ok");
        if(memcmp(type, "sw", strlen("sw")) == 0)
        {           
            eeprom_del();
            cm_pm_reboot();
        }
        else if(memcmp(type, "hw", strlen("hw")) == 0)
        {
            sysPrintf("hw");
        }
        return 0;
    }
    else
    {
        sysPrintf("usage: \r\n\treboot [sw|hw]\r\n");
        return -1;
    }
}

/**
 * @author: tn
 * @description: 控制数字电位器的电阻值
 * 格式为 ctrl dev 127\r 必须打换行符号
 * @param {void*} cmdbuf
 * @return {*}
 */
int chgctrl(void* cmdbuf)
{
    char valuestr1[20]={0};
    char valuestr2[20]={0};
    sscanf(cmdbuf,"%*s %s %s",valuestr1,valuestr2);//跳过第一个字符串提取后面两个
    uint8_t shell_step = strtoul(valuestr2,NULL,10);//字符串转换
    if(strncmp("dev", valuestr1, strlen(valuestr1)) == 0)
    {
        Step=shell_step;
        sysPrintf("test=%d",shell_step);
        tpl0401a_ctrl(shell_step);//调控电阻
    }
    else if(strncmp("config", valuestr1, strlen(valuestr1)) == 0)
    {
        RWH = (128 - Step) *78.125;
        RWL = Step * 78.125;
        sysPrintf("RWH=%.3f",RWH);
        sysPrintf("RWL=%.3f",RWL);
    }
    else
    {
        sysPrintf("输入命令无效\r\n");
    }
  return 0;
}

/**
 * @Author: TN
 * @description: shell命令注册链表管理器，动态执行指令
 * @param {shell_cmd_cfg} *cmd_list
 * @return {*}
 */
int shell_register_cmdlist(const shell_cmd_cfg *cmd_list)
{

	Shell_List_t *new_p = cm_malloc(sizeof(Shell_List_t));//申请链表内存
	int realse = -1;
	if (new_p)
	{
		new_p->next = 0;
		new_p->shell_tab = cmd_list;
		//OsLock();//互斥锁保护
		if (m_cmd_list_p)
		{//当链表为空时，直接设置为链表头
			Shell_List_t *next = m_cmd_list_p;
			while (next->next)
			{
				next = next->next;
			}
			next->next = new_p;
		}
		else
		{//当全局链表m_cmd_list_p存在时，遍历到末尾追加新节点
			m_cmd_list_p = new_p;
		}
		//OsUnLock();
		realse = 0;
	}
	return realse;
}

/**
 * @Author: TN
 * @description: 获取命令符长度 以空格或\0结束
 * @param {char} *cmd 字符串
 * @return {*}
 */
static unsigned int shell_cmd_len(char *cmd)
{
    char *p = cmd;
    unsigned int len = 0;
    while((*p != ' ') && (*p != 0))
    {
        p++;
        len++;
    }
    return len;
}

/**
 * @Author: TN
 * @description: 匹配命令字符
 * @param {char} *cmd 字符串
 * @param {char} *str 匹配字符
 * @return {*}0 匹配成功 1 匹配失败
 */
static int shell_cmd_check(char *cmd, char *str)
{
    char *p1 = NULL;
    char *p2 = NULL;    
    unsigned int len1 = 0;
    unsigned int len2 = shell_cmd_len(str);
    p1=cmd;
    if(p1!=NULL)
      p2 = strstr(p1, "\r");
    if(p2!=NULL)
      cmd[p2-p1]=0;
    len1 = shell_cmd_len(cmd);
    if(len1 != len2)
    {
        return 1;
    }
    return memcmp(cmd, str, len1);
}

/**
 * @Author: TN
 * @description: 这个函数的功能主要是搜索命令并执行
 * cmd == 0 遍历链表 进行信息输出
 * cmd != 0 匹配命令字符串，执行命令回调函数
 * @param {char*} cmd
 * @return {*}
 */
int shell_exec_cmdlist(char* cmd)
{
    int j = 0;
    if(m_cmd_list_p)
    {/** 一般需要保护 但是动态运行修改链表应该不会太大*/
        Shell_List_t *next;
        //__disable_interrupt();
        next = m_cmd_list_p;
        //__enable_interrupt();
        do{
        for (int i = 0; next->shell_tab[i].name; i++)
        {
            if (cmd == 0)
            {
                ++j;
                sysPrintf("%02d.",j);
                sysPrintf("%-16s", next->shell_tab[i].name);
                sysPrintf("%s\r\n", next->shell_tab[i].helpstr);
            }
            else
            {
                if(shell_cmd_check(cmd,  next->shell_tab[i].name) == 0)
                {
                return  next->shell_tab[i].func(cmd);
                }
            }
        }
        next = next->next;
        }while (next);

    }
    if (cmd != 0)
    {
        sysPrintf("unkown command\r\n");
    }
    return -1;
}

void shell_exec_shellcmd(void)
{
    if (!shellpassed)
    {//校验shellpassed密码，暂时屏蔽
        //  if (shell_check_passwd() == 0)
        {
            shellpassed = 1;
            //passedtime = driver_time_getruntime();
        }
    }
    if (shellpassed == 1)
    {
        if(GetUartMessQueue(cmd_buf)==0)
        {//使用消息队列  
            if(cmd_buf[0]=='\r')
            {
              sysPrintf("sh>\r\n");
              memset(cmd_buf,0x00,sizeof(cmd_buf));
            }
            else  
            {//执行对应的命令回调函数           
              shell_exec_cmdlist(cmd_buf);
              sysPrintf("\r\n");
            }
        }
    }
}

/**
 * @Author: TN
 * @description: shell任务入口函数
 * @return {*}
 */
void shell_task_entry(void)
{
    shell_register_cmdlist(shell_cmd_list);
    while(1)
    {
        shell_exec_shellcmd();//shell处理
        osDelay(100/5);//延时
    }
}

