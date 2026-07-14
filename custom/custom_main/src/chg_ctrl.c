/*
 * @Author: tn
 * @Description: 该文件用于封装继电器控制io，电平转换芯片使能引脚，pwm控制io口，pwm控制暂时无法使用，计量模块无法测量pwm电机电流，频率过快
 * @Date: 2025-03-29 18:46:31
 * @LastEditors: tn
 * @LastEditTime: 2025-06-01 15:55:56
 */

#include "cm_gpio.h"
#include "cm_iomux.h"
#include "cm_os.h"
#include "chg_ctrl.h"
#include "cm_pwm.h"
#include "chg_module.h"
#include "chg_dev.h"

uint32_t period = 10000;//周期为100khz
uint32_t period_h=5000;//修改占空比，高于3000
uint8_t dev = CM_PWM_DEV_0;//使用PWM0，74引脚
#define OPENCPU_TEST_PWM0_IOMUX CM_IOMUX_PIN_74, CM_IOMUX_FUNC_FUNCTION1

/**
 * @author: tn
 * @description: 用于配置pwm控制引脚,及L298n驱动板控制引脚
 * @return {*}
 */
void pwm_init(void)
{
    // cm_iomux_set_pin_func(CM_IOMUX_PIN_87, CM_IOMUX_FUNC_FUNCTION2);
    // cm_gpio_cfg_t cfg_pwm_1 = {0};
    // cfg_pwm_1.direction = CM_GPIO_DIRECTION_OUTPUT;//输出
    // cfg_pwm_1.pull = CM_GPIO_PULL_UP;//上拉
    // cm_gpio_init(CM_GPIO_NUM_21, &cfg_pwm_1);//配置GPIO21

    // cm_gpio_set_level(CM_GPIO_NUM_21, CM_GPIO_LEVEL_LOW);


    uint8_t ret;
    cm_iomux_set_pin_func(OPENCPU_TEST_PWM0_IOMUX);
    ret = cm_pwm_open_ns(dev, period,period_h);
    if(ret == 0)
    {
        sysPrintf("PWM_open\r\n");
    }
    else
    {
        sysPrintf("PWM_fail\r\n");
    }
}

/**
 * @author: tn
 * @description: 配置急停按键
 * @return {*}
 */
void E_stop_init(void)
{
    cm_iomux_set_pin_func(CM_IOMUX_PIN_86, CM_IOMUX_FUNC_FUNCTION2);
    cm_gpio_cfg_t cfg_pwm_0 = {0};
    cfg_pwm_0.direction = CM_GPIO_DIRECTION_INPUT;//输入
    cfg_pwm_0.pull = CM_GPIO_PULL_UP;//上拉
    cm_gpio_init(CM_GPIO_NUM_20, &cfg_pwm_0);//配置GPIO20
}

/**
 * @author: tn
 * @description: 急停按键，按键读取到低电平，关闭继电器，并语音播报滴
 * @return {*}
 */
void E_stop_read(void)
{
    cm_gpio_level_e level;//枚举类型
    cm_gpio_get_level(CM_GPIO_NUM_20,&level);
    if(level==CM_GPIO_LEVEL_LOW)
    {
        chg_ctrl = 0;
        audio_play=5;
        eeprom_del();
    }
}

/**
 * @author: tn
 * @description: 该函数用于封装串口用于修改占空比
 * @return {*}
 */
void pwn_ctrl(void)
{

}

/**
 * @author: tn
 * @description: 配置继电器的控制io口
 * @return {*}
 */
void ctrl_init(void)
{
    cm_iomux_set_pin_func(CM_IOMUX_PIN_54, CM_IOMUX_FUNC_FUNCTION2);
    cm_gpio_cfg_t cfg = {0};
    cfg.direction = CM_GPIO_DIRECTION_OUTPUT;//输出
    cfg.pull = CM_GPIO_PULL_UP;//上拉
    cm_gpio_init(CM_GPIO_NUM_5, &cfg);//配置GPIO5

    cm_iomux_set_pin_func(CM_IOMUX_PIN_76, CM_IOMUX_FUNC_FUNCTION2);
    cm_gpio_cfg_t cfg1 = {0};
    cfg1.direction = CM_GPIO_DIRECTION_OUTPUT;//输出
    cfg1.pull = CM_GPIO_PULL_UP;//上拉
    cm_gpio_init(CM_GPIO_NUM_12, &cfg1);//配置GPIO12
}

/**
 * @author: tn
 * @description: LED初始化
 * @return {*}
 */
void LED_init(void)
{
    cm_iomux_set_pin_func(CM_IOMUX_PIN_16, CM_IOMUX_FUNC_FUNCTION1);
    cm_gpio_cfg_t cfg = {0};
    cfg.direction = CM_GPIO_DIRECTION_OUTPUT;//输出
    cfg.pull = CM_GPIO_PULL_UP;//上拉
    cm_gpio_init(CM_GPIO_NUM_0, &cfg);//配置GPIO0
}

void LED_run(void)
{
    /*LED运行灯功能 使用gpio22*/
    if(chg_mqtt==0)
    {  
        cm_gpio_set_level(CM_GPIO_NUM_0, CM_GPIO_LEVEL_LOW);//设置GPIO0输出高电平，灭灯
    }
    else if(chg_mqtt==1)
    {  
        cm_gpio_set_level(CM_GPIO_NUM_0, CM_GPIO_LEVEL_LOW);//设置GPIO0输出低电平
        osDelay(100); 
        cm_gpio_set_level(CM_GPIO_NUM_0, CM_GPIO_LEVEL_HIGH);//设置GPIO0输出高电平
    }

}

/**
 * @author: tn
 * @description: 配置并使能txs0108e电平转换芯片
 * 使能0E引脚
 * 电流驱动弱，接继电器需要接上拉电阻，增强电流驱动能力（电阻不能过小）
 * 
 * 此电平转换芯片的电流驱动能力小，建议只接继电器以及rc522
 * @return {*}
 */
void vcc_turn(void)
{
    /*使能电平转换芯片*/
    cm_gpio_cfg_t cfg_1 = {0};
    cfg_1.direction = CM_GPIO_DIRECTION_OUTPUT;//输出
    cfg_1.pull = CM_GPIO_PULL_UP;//上拉
    cm_iomux_set_pin_func(CM_IOMUX_PIN_81, CM_IOMUX_FUNC_FUNCTION2);
    cm_gpio_init(CM_GPIO_NUM_16, &cfg_1);//配置GPIO16，复位脚
    cm_gpio_set_level(CM_GPIO_NUM_16, CM_GPIO_LEVEL_HIGH);//设置GPIO16输出高电平

    /*使能电平转换芯片*/
    cm_gpio_cfg_t cfg_2 = {0};
    cfg_2.direction = CM_GPIO_DIRECTION_OUTPUT;//输出
    cfg_2.pull = CM_GPIO_PULL_UP;//上拉
    cm_iomux_set_pin_func(CM_IOMUX_PIN_78, CM_IOMUX_FUNC_FUNCTION2);
    cm_gpio_init(CM_GPIO_NUM_14, &cfg_2);//配置GPIO16，复位脚
    cm_gpio_set_level(CM_GPIO_NUM_14, CM_GPIO_LEVEL_HIGH);//设置GPIO16输出高电平
}


/**
 * @author: tn
 * @description: 继电器控制端口，当充电标志为1时，闭合继电器开始充电
 * @return {*}
 */
void ctrl_chg(void)
{
    if(chg_ctrl == 1)
    {//打开继电器
        cm_gpio_set_level(CM_GPIO_NUM_12, CM_GPIO_LEVEL_HIGH);
    }
    if(chg_ctrl == 0)
    {//关闭继电器
        cm_gpio_set_level(CM_GPIO_NUM_12, CM_GPIO_LEVEL_LOW);
    }
}

 void ctrl_task_entry(void)
 {
    ctrl_init();
    vcc_turn();
    LED_init();
    // pwm_init();
    E_stop_init();
    tpl0401a_init();
     while (1)
     { 
        // cm_gpio_set_level(CM_GPIO_NUM_5, CM_GPIO_LEVEL_LOW);
        LED_run();
        ctrl_chg();
        E_stop_read();//保证该函数的优先性
        osDelay(1000/5);
     }
 }
