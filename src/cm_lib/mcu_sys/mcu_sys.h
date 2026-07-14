#ifndef mcu_sys_h_
#define mcu_sys_h_

#ifndef mcu_sys_c_//如果没有定义
#define mcu_sys_cx_ extern
#else
#define mcu_sys_cx_
#endif

#include <stdio.h>
#include "stdio.h"
#include "stdlib.h"
#include "cm_sys.h"

char mcu_sys_init(uint64_t val, uint64_t* val1,uint64_t* val2,uint64_t* val3);

#endif
