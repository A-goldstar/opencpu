/*
 * @Author: tn
 * @Description: 
 * @Date: 2025-03-10 19:54:39
 * @LastEditors: tn
 * @LastEditTime: 2025-05-17 16:18:24
 */

#ifndef _CHG_MODULE_H__
#define _CHG_MODULE_H__
//头文件保护

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
//C++兼容性处理
/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "cm_os.h"
#include "chg_struct.h"

void CreateQueueTask(void);

char SendmeterMessQueue(char *data);
char GetmeterMessQueue(char *data);

char SendUartMessQueue(char *data);
char GetUartMessQueue(char *data);

char SendMqttMessQueue(char *data);
char GetMqttMessQueue(char *data);

uint8_t sysPrintf(const char *data, ...);
void uart0RecvData(uint32_t flags,char *data, int len);
uint8_t uart0Recv_init(void);
void module_task_entry(void);

void eeprom_addr(void);
void eeprom_del(void);

#ifdef __cplusplus
}
#endif
//C++兼容性处理

#endif /*  */
//头文件保护