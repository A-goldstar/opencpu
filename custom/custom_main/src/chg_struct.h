/*
 * @Author: tn
 * @Description: 
 * @Date: 2025-03-10 19:54:42
 * @LastEditors: tn
 * @LastEditTime: 2025-05-17 19:43:53
 */

#ifndef _CHG_STRUCT_H__
#define _CHG_STRUCT_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{//当使用C+编译的时候，确保内部C语言代码以未修饰的形式生成
#endif
/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <string.h>
#include <strings.h>
#include <stdbool.h>

#define SHELL_CMD_LENTH  255            // shell命令缓冲区长度 

#define buffer_len  37

uint8_t audio_play;//语音播放标志
uint16_t chg_ctrl;//启动充电标志 1启动充电 0停止充电
int8_t chg_mqtt;//设备上线标志位，当有PING回调表示设备已经正常上线.1,设备上线
extern uint8_t Step;	// 数字电位器范围为0-127
float RWH, RWL;		// 理论值

bool param_shell;
bool meter_shell;
bool nfc_shell;
bool temp_shell;
bool humi_shell;

#ifdef __cplusplus
}
#endif

#endif /*  */

/*
枚举类型通过 语义化命名、类型安全 和 资源优化
*/
