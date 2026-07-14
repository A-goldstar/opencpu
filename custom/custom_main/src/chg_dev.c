/*
 * @Author: tn
 * @Description: 封装数字电位器,使用软件iic,gpio6 gpio7
 * @Date: 2025-05-02 20:12:17
 * @LastEditors: tn
 * @LastEditTime: 2025-06-01 16:51:37
 */
#include <stdint.h>
#include "cm_gpio.h"
#include "cm_iomux.h"
#include "cm_os.h"
#include "chg_dev.h"
#include "cm_i2c.h"
#include "chg_module.h"
#include "stdint.h"
#include "chg_struct.h"

#define TPL0401A_I2C_ID CM_I2C_DEV_2
#define TPL0401A_ADDRESS_W	0x5C	//写地址
#define TPL0401A_ADDRESS_R	0x5D	//读地址
#define DELAY_COUNT 20

#define IIC_SCL(x) do{ x ? \
    cm_gpio_set_level(CM_GPIO_NUM_6, CM_GPIO_LEVEL_HIGH) : \
    cm_gpio_set_level(CM_GPIO_NUM_6, CM_GPIO_LEVEL_LOW); \
}while(0) 

#define IIC_SDA(x)        do{ x ? \
    cm_gpio_set_level(CM_GPIO_NUM_7, CM_GPIO_LEVEL_HIGH) : \
    cm_gpio_set_level(CM_GPIO_NUM_7, CM_GPIO_LEVEL_LOW); \
}while(0) 

#define IIC_READ_SDA     get_level() /* 读取SDA */

uint8_t Step = 0;	// 数字电位器范围为0-127

/*PID控制参数*/
// uint16_t target_val;               //目标值
// uint16_t actual_val;        		//实际值
// uint16_t err;             			//定义偏差值
// uint16_t err_last;          		//定义上一个偏差值
// uint16_t err_prev;					//上上次误差
// uint16_t actual_val_last;
// uint16_t Kp,Ki,Kd;          		//定义比例、积分、微分系数
// uint16_t integral;          		//定义积分值
struct
{
    uint16_t target_val;               //目标值
    uint16_t actual_val;        		//实际值
    uint16_t err;             			//定义偏差值
    uint16_t err_last;          		//定义上一个偏差值
    uint16_t Kp,Ki,Kd;          		//定义比例、积分、微分系数
    uint16_t integral;          		//定义积分值
}pid;

/**
 * @author: tn
 * @description: PID控制参数
 * @return {*}
 */
void PID_init()
{
	pid.Kp=0.3;
	pid.Ki=0;
	pid.Kd=0;
}

/**
 * @author: tn
 * @description: PID控制算法，增量式
 * @param {int16_t} target_val 目标值
 * @param {int16_t} actual_val 实际值
 * 
 * 在该代码中不适用这样形式的算法，后续需要进行改进，调控的是电位器的值
 * 这个PID的输出作为控制电位器算法的输入
 * @return {*}
 */
// int PID_value(int16_t target_val,int16_t actual_val)//预估值，实际值
// {
// 	err=target_val-actual_val;//目标值和实际值的误差
	
// 	if(err<=0.5)
// 	{
// 		actual_val=actual_val_last;
// 	}//死区控制
// 	else
// 	{
// 	    actual_val+=Kp*(err-err_last)+Ki*err+Kd*(err-2*err_last+err_prev);//进行算法计算
// 	}
// 	err_prev=err_last;
// 	err_last=err;//将误差进行传递,有两次误差

// 	return actual_val;
// }

/**
 * @author: tn
 * @description: 位置式PID算法，返回的直接就是阶数，用于调控电位器，没有小车那种精准调控实时
 * @param {uint16_t} temp_val
 * @param {uint16_t} cnt_val
 * @return {*}
 */
uint16_t PID_calc(uint16_t temp_val,uint16_t cnt_val)
{//位置式PID算法，temp_val为预估值 cnt_val为理论值
	pid.target_val=cnt_val;//理论值
	pid.err=temp_val-pid.target_val;//误差比例，预估值减去实际值
	pid.integral+=pid.err;//积分
	//积分限幅
	pid.actual_val=pid.Kp*pid.err+pid.Ki*pid.integral+pid.Kd*(pid.err-pid.err_last);//进行算法计算
	pid.err_last=pid.err;//将误差进行传递
	
	return pid.actual_val;
}

/**
 * @author: tn
 * @description: 获取SDA电平
 * @return {*}
 */
int get_level(void)
{
    cm_gpio_level_e level;//枚举类型
    cm_gpio_get_level(CM_GPIO_NUM_7,&level);
    if(level==CM_GPIO_LEVEL_LOW)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/**
 * @author: tn
 * @description: 阻塞延时，2us
 * @return {*}
 */
static void iic_delay(void)
{
    volatile uint32_t i;
    for (i = 0; i < DELAY_COUNT; i++); // 调整DELAY_COUNT值
}


/**
 * @author: tn
 * @description: IIC起始信号
 * @return {*}
 */
void iic_start(void)
{
    IIC_SDA(1);
    IIC_SCL(1);
    iic_delay();
    IIC_SDA(0);     
    iic_delay();
    IIC_SCL(0);    
    iic_delay();
}


/**
 * @author: tn
 * @description: IIC停止信号
 * @return {*}
 */
void iic_stop(void)
{
    IIC_SDA(0);   
    iic_delay();
    IIC_SCL(1);
    iic_delay();
    IIC_SDA(1);    
    iic_delay();
}


/**
 * @author: tn
 * @description: 等待应答信号
 * @return {*}
 */
uint8_t iic_wait_ack(void)
{
    uint8_t waittime = 0;
    uint8_t rack = 0;

    IIC_SDA(1);     
    iic_delay();
    IIC_SCL(1);     
    iic_delay();

    while (IIC_READ_SDA)    
    {
        waittime++;

        if (waittime > 250)
        {
            iic_stop();
            rack = 1;
            break;
        }
    }

    IIC_SCL(0);     
    iic_delay();
    return rack;
}


/**
 * @author: tn
 * @description: 产生ACK应答
 * @return {*}
 */
void iic_ack(void)
{
    IIC_SDA(0);     
    iic_delay();
    IIC_SCL(1);     
    iic_delay();
    IIC_SCL(0);
    iic_delay();
    IIC_SDA(1);     
    iic_delay();
}


/**
 * @author: tn
 * @description: 不产生ack应答
 * @return {*}
 */
void iic_nack(void)
{
    IIC_SDA(1);     
    iic_delay();
    IIC_SCL(1);     
    iic_delay();
    IIC_SCL(0);
    iic_delay();
}

/**
 * @author: tn
 * @description: 发送一个字节
 * @param {uint8_t} data
 * @return {*}
 */
void iic_send_byte(uint8_t data)
{
    uint8_t t;
    
    for (t = 0; t < 8; t++)
    {
        IIC_SDA((data & 0x80) >> 7);    
        iic_delay();
        IIC_SCL(1);
        iic_delay();
        IIC_SCL(0);
        data <<= 1;     
    }
    IIC_SDA(1);        
}

/**
 * @author: tn
 * @description: 读取一个字节
 * @param {uint8_t} ack
 * @return {*}
 */
uint8_t iic_read_byte(uint8_t ack)
{
    uint8_t i, receive = 0;

    for (i = 0; i < 8; i++ )   
    {
        receive <<= 1; 
        IIC_SCL(1);
        iic_delay();

        if (IIC_READ_SDA)
        {
            receive++;
        }
        
        IIC_SCL(0);
        iic_delay();
    }

    if (!ack)
    {
        iic_nack();     
    }
    else
    {
        iic_ack();      
    }

    return receive;
}




/**
 * @author: tn
 * @description: 初始化tpl0401a iic，使用CM_I2C_DEV_2
 * @param {cm_i2c_dev_e} iic_id 入参iic2
 * @return {*}0 iic初始化正确  -1 iic初始化失败
 */
void iic_init(void)
{
    cm_iomux_set_pin_func(CM_IOMUX_PIN_55, CM_IOMUX_FUNC_FUNCTION2);
    cm_gpio_cfg_t cfg_1 = {0};
    cfg_1.direction = CM_GPIO_DIRECTION_OUTPUT;//输出
    cfg_1.pull = CM_GPIO_PULL_UP;//上拉
    cm_gpio_init(CM_GPIO_NUM_6, &cfg_1);//配置GPIO6 SCL

    cm_iomux_set_pin_func(CM_IOMUX_PIN_56, CM_IOMUX_FUNC_FUNCTION2);
    cm_gpio_cfg_t cfg_2 = {0};
    cfg_2.direction = CM_GPIO_DIRECTION_OUTPUT;//输出
    cfg_2.pull = CM_GPIO_PULL_UP;//上拉
    cm_gpio_init(CM_GPIO_NUM_7, &cfg_2);//配置GPIO7 SDA
    // cm_i2c_cfg_t config = 
	// {
	// 	CM_I2C_ADDR_TYPE_7BIT,
	// 	CM_I2C_MODE_MASTER, //目前仅支持模式
	// 	CM_I2C_CLK_100KHZ
	// };//master模式,(100KHZ)

    // int32_t ret;

    // //开启之前一定要先设置引脚复用
	// if(iic_id == CM_I2C_DEV_0)
	// {
	// 	cm_iomux_set_pin_func(CM_IOMUX_PIN_66, CM_IOMUX_FUNC_FUNCTION1); //IIC0_SDA
	// 	cm_iomux_set_pin_func(CM_IOMUX_PIN_67, CM_IOMUX_FUNC_FUNCTION1); //IIC0_SCL	
	// }
	// else if(iic_id == CM_I2C_DEV_1)
	// {
	// 	cm_iomux_set_pin_func(CM_IOMUX_PIN_52, CM_IOMUX_FUNC_FUNCTION2); //IIC1_SDA
	// 	cm_iomux_set_pin_func(CM_IOMUX_PIN_53, CM_IOMUX_FUNC_FUNCTION2); //IIC1_SCL
	// }
	// else
	// {
	// 	cm_iomux_set_pin_func(CM_IOMUX_PIN_74, CM_IOMUX_FUNC_FUNCTION2); //IIC2_SDA
	// 	cm_iomux_set_pin_func(CM_IOMUX_PIN_75, CM_IOMUX_FUNC_FUNCTION2); //IIC2_SCL
	// }
    // ret = cm_i2c_open(iic_id, &config);

    // if(ret != 0)
	// {
	// 	sysPrintf("i2c init err,ret=%d\n", ret);
	// 	return -1;
	// }
    // sysPrintf("tpl0401a i2c init ok\n");

	// return 0;
}


/**
 * @author: tn
 * @description: 写寄存器
 * @param {uint8_t} Data
 * @return {*}
 */
void tpl0401a_write_reg(uint8_t Data)
{	
	iic_start();
	iic_send_byte(TPL0401A_ADDRESS_W);
	iic_wait_ack();
	iic_send_byte(0x00);
	iic_wait_ack();
	iic_send_byte(Data);
	iic_wait_ack();
	iic_stop();
}

/**
 * @author: tn
 * @description: 读寄存器
 * @return {*}
 */
uint8_t tpl0401a_read_reg(void)
{
	uint8_t data = 0;
	
	iic_start();
	iic_send_byte(TPL0401A_ADDRESS_W);
	iic_wait_ack();
	iic_send_byte(0x00);
	iic_wait_ack();
	
	iic_start();
	iic_send_byte(TPL0401A_ADDRESS_R);
	iic_wait_ack();
	data = iic_read_byte(0);
	iic_stop();

	return data;
}

/**
 * @author: tn
 * @description: TPL0401A初始化
 * @return {*}
 */
void tpl0401a_init(void)
{ 
	iic_init();
	osDelay(10/5);

	tpl0401a_write_reg(0x40);

    tpl0401a_write_reg(Step);
    RWH = (128 - Step) *78.125;
	RWL = Step * 78.125;
    sysPrintf("RWH=%.3f",RWH);
    sysPrintf("RWL=%.3f",RWL);
}   

/**
 * @author: tn
 * @description: 调整tpl0401a电阻值     10k
 * @param {uint8_t} Step_ctrl  入参 范围为0-127 电阻最大为10k
 * @return {*}0 电阻调控成功    -1入参错误
 */
int16_t tpl0401a_ctrl(uint8_t Step_ctrl)
{
    /*调整电阻值*/
    if(Step_ctrl<128)
    {
        Step=Step_ctrl;
        tpl0401a_write_reg(Step_ctrl);
        sysPrintf("step test");
    }
    else
    {
        sysPrintf("STEP ERR");
        return -1;
    }
    
    /*打印理论值*/
    RWH = (128 - Step) *78.125;
	RWL = Step * 78.125;
    sysPrintf("RWH=%.3f",RWH);
    sysPrintf("RWL=%.3f",RWL);
    return 0;
}
