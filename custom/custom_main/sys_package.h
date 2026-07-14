#ifndef sys_package_h_
#define sys_package_h_

#ifndef sys_package_c_//如果没有定义
#define sys_package_cx_ extern
#else
#define sys_package_cx_
#endif

#include <stdio.h>
#include "stdio.h"
#include "stdlib.h"

void sys_package_test(void);

#endif
