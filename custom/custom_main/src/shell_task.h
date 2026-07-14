/*
 * @Author: TN
 * @Date: 2025-04-28 13:27:44
 * @LastEditTime: 2025-04-28 13:49:54
 * @FilePath: \opencpu\custom\custom_main\src\shell_task.h
 * @Description: 
 */
#ifndef __SHELL_TASK_H__
#define __SHELL_TASK_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
/*******************************************************************************
 * Include files
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
typedef int ( * CommandFunc )( void *);   /**< shell命令回调函数 */

 /*链表单个数据的结构体，链表当中的每个元素都存储着这些数据*/
typedef struct
{
    char* name;           /**< shell命令名 */
    CommandFunc func;           /**< shell命令回调函数 */
  //  char const* helpstr;       /**< 帮助字符串 */
    char * helpstr;
}shell_cmd_cfg;

/*配置一个链表 用于进行shell打印*/
typedef struct Shell_List{
	struct Shell_List *next; /** 下一个数据结构*/
	const shell_cmd_cfg *shell_tab; /**shell 表必须是全局数组*/
}Shell_List_t;

/*******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/

 void shell_task_entry(void);

#ifdef __cplusplus
}
#endif

#endif /*  */



