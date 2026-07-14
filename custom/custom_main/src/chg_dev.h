/*
 * @Author: tn
 * @Description: 
 * @Date: 2025-03-29 18:46:43
 * @LastEditors: tn
 * @LastEditTime: 2025-06-01 16:44:08
 */
#ifndef _CHG_DEV_H__
#define _CHG_DEV_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "cm_i2c.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/


/*******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/

//  void ctrl_task_entry(void);
void PID_init(void);
uint16_t PID_calc(uint16_t temp_val,uint16_t cnt_val);
void tpl0401a_init(void);
int16_t tpl0401a_ctrl(uint8_t Step_ctrl);

#ifdef __cplusplus
}
#endif

#endif /*  */
