/*
 * @Author: tn
 * @Description: 
 * @Date: 2025-03-31 16:34:28
 * @LastEditors: tn
 * @LastEditTime: 2025-04-02 19:48:05
 */

 #ifndef _CHG_METER_H__
 #define _CHG_METER_H__
 
 /* C binding of definitions if building with C++ compiler */
 #ifdef __cplusplus
 extern "C"
 {
 #endif
 /*******************************************************************************
  * Include files
  ******************************************************************************/
 #include "cm_os.h"
 #include "chg_struct.h"

uint8_t meterPrintf(uint8_t *data, uint16_t length);
void uart1RecvData(uint32_t flags,char *data, int len);
uint8_t uart1Recv_init(void);
 void meter_task_entry(void);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /*  */