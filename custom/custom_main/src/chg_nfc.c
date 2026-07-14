/*
 * @Author: tn
 * @Description: 
 * @Date: 2025-05-07 17:06:54
 * @LastEditors: tn
 * @LastEditTime: 2025-07-13 21:53:42
 */
/*
 * @Author: tn
 * @Description: 封装NFC刷卡代码，使用spi进行通信，spi底层通信代码相当于hal库的硬件spi
 * @Date: 2025-03-13 19:24:43
 * @LastEditors: tn
 * @LastEditTime: 2025-05-25 15:48:17
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "chg_nfc.h"
#include "cm_spi.h"
#include "chg_struct.h"
#include "cm_iomux.h"
#include "cm_gpio.h"
#include "cm_iomux.h"
#include "chg_module.h"
#include "cm_os.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/
/*ci522的复位引脚，需要修改*/
#define APP_DEMO_CI522_SPI       CM_SPI_DEV_1
#define APP_DEMO_CI522_GPI0_RST   CM_GPIO_NUM_8//复位引脚
#define APP_DEMO_CI522_GPI0_IRQ CM_GPIO_NUM_1  //CI522的中断引脚

/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**----------------------------------------------------------------------------*/
/*spi编号*/
static cm_spi_dev_e s_ci522_spi = CM_SPI_DEV_1;             // CI522 SPI编号
static cm_gpio_num_e s_ci522_gpio_rst = CM_GPIO_NUM_8;      // CI522 复位引脚
//选ci522
//static uint8_t setlect_ci522_flag=1;
/*----------------------------------------------------------------------------*
**                             Local Func                                     *
**----------------------------------------------------------------------------*/
bool app_ci522_write(uint8_t reg_addr,uint8_t value);
uint8_t app_ci522_read(uint8_t reg_addr);

/*定义一个变量存储uid*/
uint8_t readUid[12]; 
uint8_t uuid[12] = {0};

uint8_t ctrl_nfc;//NFC刷卡鉴权标志位

#define list_count 30
/*白名单*/
unsigned char white_list[list_count][15] = {
    "04512bf2a61c91",
    "04041ff2a61c91",
    "ff0fd182df0000",//合法卡

};

/*
使用顺序查找 注意大小写
返回1表示遍历到白名单
返回-1未遍历到白名单
*/
int white_list_search(unsigned char *read_list) 
{
    char read_hex[15] = {0}; // 存储转换后的字符串
    // 将 7字节 UID 转换为 14字符的十六进制字符串
    snprintf(read_hex, sizeof(read_hex), 
             "%02x%02x%02x%02x%02x%02x%02x",
             read_list[0], read_list[1], read_list[2],
             read_list[3], read_list[4], read_list[5], read_list[6]);

    for (int i = 0; i < list_count; i++) 
    {
        if (strcmp(white_list[i], read_hex) == 0) 
        {
            return 1; // 匹配成功
        }
    }
    return -1; // 未匹配
}
/**
 *  @brief 16进制转变为16进制字符串
 *
 *  @param [out] hexstr 16进制字符串数据缓冲区
 *  @param [in] hexstr_size 16进制字符串数据缓冲区大小
 *  @param [in] hex 16进制数据
 *  @param [in] hexlen 16进制数据长度
 *  @param [in] type 0:小写 1:大写
 *  
 *  @return 转换后的16进制字符串大小
 *
 *  @details 
 */
unsigned short app_get_hex2hexstr(unsigned char *hexstr,unsigned short hexstr_size,unsigned char *hex,unsigned short hexlen,unsigned char type)
{
    if(hex == NULL || hexstr == NULL || hexlen == 0 || hexstr_size < (2 * hexlen))
    {
        return 0;
    }
    unsigned char temp[5] = {0};
    unsigned short i = 0;
    for (i = 0; i < hexlen; i++)
    {
        memset(temp, 0, sizeof(temp));
        if(type == 0)
        {
            sprintf((char *)temp, "%02x", *(hex + i));
        }
        else
        {
            sprintf((char *)temp, "%02X", *(hex + i));
        }
        strcat((char *)hexstr, (char *)temp);
        //memcpy((hexstr + i * 2),temp,2);
    }
    return (2 * hexlen);
}


/**
 *  @brief ci522 外设初始化
 *
 *  @param [in] spi_id  spi序号
 *  @param [in] rst_gpio  ci522复位引脚
 *  @param [in] irq_pgio  ci522中断引脚
 *
 *  @return
 *
 *  @details 
 */
void  app_ci522_dev_init(cm_spi_dev_e spi_id,cm_gpio_num_e rst_gpio,cm_gpio_num_e irq_pgio)
{   
    cm_gpio_cfg_t cfg = {0};
    cfg.direction = CM_GPIO_DIRECTION_OUTPUT;//输出
    cfg.pull = CM_GPIO_PULL_UP;//上拉
    cm_iomux_set_pin_func(CM_IOMUX_PIN_57, CM_IOMUX_FUNC_FUNCTION2);
    cm_gpio_init(CM_GPIO_NUM_8, &cfg);//配置GPIO8，复位脚

    s_ci522_spi = spi_id;
    s_ci522_gpio_rst = rst_gpio;
    cm_gpio_set_level(rst_gpio, CM_GPIO_LEVEL_LOW);//设置GPIO8输出高电平
    osDelay(100 / 5);
    cm_gpio_set_level(rst_gpio,CM_GPIO_LEVEL_HIGH);
    osDelay(100 / 5);

    cm_spi_cfg_t config = {CM_SPI_MODE_MASTER,CM_SPI_WOKR_MODE_0, CM_SPI_DATA_WIDTH_8BIT,CM_SPI_NSS_HARD,CM_SPI_CLK_3_25MHZ};//时钟=2.448MHZ(16分频),mode0
    
    cm_iomux_set_pin_func(CM_IOMUX_PIN_30, CM_IOMUX_FUNC_FUNCTION2); //SPI1_CLK
	cm_iomux_set_pin_func(CM_IOMUX_PIN_31, CM_IOMUX_FUNC_FUNCTION2); //SPI1_CS
	cm_iomux_set_pin_func(CM_IOMUX_PIN_32, CM_IOMUX_FUNC_FUNCTION2); //SPI1_MISO
	cm_iomux_set_pin_func(CM_IOMUX_PIN_33, CM_IOMUX_FUNC_FUNCTION2); //SPI1_MOSI

    if(cm_spi_open(s_ci522_spi, &config) < 0)
    {
        sysPrintf("cm_spi_open fail");
    }
    else
    {
        // sysPrintf("RFID_cm_spi_open");//初始化已经成功
    }
}

/**
 *  @brief 写ci522寄存器
 *
 *  @param [in] reg_addr 寄存器地址
 *  @param [in] value   需要写入的值
 *
 *  @return
 *
 *  @details 
 */
bool app_ci522_write(uint8_t reg_addr,uint8_t value)
{
    uint8_t cmd[4] = {0};//数组，用于存储发送的命令
    cmd[0] = (reg_addr & 0x3f) << 1;
    cmd[1] = value;
    // osDelay(100 / portTICK_PERIOD_MS);
    // cm_gpio_set_level(CM_GPIO_NUM_40,CM_GPIO_LEVEL_LOW);//拉低开始传输信号，默认拉高
    if(cm_spi_write(s_ci522_spi,cmd,2) < 0)
    {
        return false;
    }
    // cm_gpio_set_level(CM_GPIO_NUM_40,CM_GPIO_LEVEL_HIGH);
    return true;
}


/**
 *  @brief 读ci522寄存器
 *
 *  @param [in] reg_addr 寄存器地址
 *
 *  @return 寄存器地址的数值
 *
 *  @details 
 */
/*
reg_addr表示寄存器地址
先写后读，spi没有读写之分
*/
uint8_t app_ci522_read(uint8_t reg_addr) 
{   
    uint8_t value[5] = {0},cmd[5] = {0};//用于存储发送命令和读取的数据
    cmd[0] = ((reg_addr << 1)&0x7E) | 0x80;                 //code the first byte

    // osDelay(100 / portTICK_PERIOD_MS);
    // cm_gpio_set_level(CM_GPIO_NUM_40,CM_GPIO_LEVEL_LOW);//拉低开始传输信号，默认拉高

    if(cm_spi_write(s_ci522_spi,cmd,1) < 0)
    {//将命令字节发送到ci522，忽略高二位左移一位，最高位为1表示读操作
        //hal_error_log("cm_spi_write fail");
        return 0;
    }

    if(cm_spi_read(s_ci522_spi,value,1) < 0)
    {
        //hal_error_log("cm_spi_read fail");
        return 0;
    }
    // cm_gpio_set_level(CM_GPIO_NUM_40,CM_GPIO_LEVEL_HIGH);//拉高
    return value[0];
}

/**
 *  @brief ci522设置某个寄存器的bit位
 *
 *  @param [in] reg_addr 寄存器地址
 *  @param [in] mask   设置的bit位
 *
 *  @return
 *
 *  @details 
 */
void app_ci522_set_bitmask(uint8_t reg_addr,uint8_t mask)  
{
	uint8_t tmp = 0x00;
	tmp = app_ci522_read(reg_addr);
	app_ci522_write(reg_addr,tmp | mask);  // set bit mask
}


/**
 *  @brief ci522清除某个寄存器的bit位
 *
 *  @param [in] reg_addr 寄存器地址
 *  @param [in] mask   清除的bit位
 *
 *  @return
 *
 *  @details 
 */
void app_ci522_clear_bitmask(uint8_t reg_addr,uint8_t mask)  
{
	uint8_t tmp = 0x00;
    static uint8_t rd_cnt=0;
	tmp = app_ci522_read(reg_addr);   
	app_ci522_write(reg_addr, tmp & ~mask);  // clear bit mask
    if(tmp==0)
    {
        if(++rd_cnt>=10)
        {
            rd_cnt=0;
            // task_flag.flag.nfc_com_error=1;
        }
    }
    else
    {
        rd_cnt=0;
        // task_flag.flag.nfc_com_error=0;
    }    
} 


/**
 *  @brief ci522复位
 *
 *  @param [in] mode 0:硬件引脚复位 1:操作寄存器复位
 *
 *  @return
 *
 *  @details 
 */
// void app_ci522_reset(uint8_t mode)
// {
//     //硬件复位
//     if(mode == 0)
//     {
//         cm_gpio_set_level(s_ci522_gpio_rst,CM_GPIO_LEVEL_LOW);
//         osDelay(100 / portTICK_PERIOD_MS);
//         cm_gpio_set_level(s_ci522_gpio_rst,CM_GPIO_LEVEL_HIGH);
//         osDelay(100 / portTICK_PERIOD_MS);
//     }
//     else
//     {
//         //寄存器复位
//         app_ci522_write(CommandReg, 0x0f);
//         while (app_ci522_read(CommandReg) & 0x10);
//         osDelay(100 / portTICK_PERIOD_MS);
//     }
// }

/**
 *  @brief 开启ci522天线
 *
 *  @param [in] type 1:打开TX1 2:打开TX2 3:打开TX1,TX2
 *
 *  @return
 *
 *  @details 
 */
void app_ci522_antenna_open(uint8_t type)
{
    if(type == 0 || type > 3)
    {
        return;
    }
    uint8_t reg_value = 0;
    reg_value = app_ci522_read(TxControlReg);
    if (!(reg_value & type))
    {
        app_ci522_set_bitmask(TxControlReg, type);
    }
}

/**
 *  @brief 关闭ci522天线
 *
 *  @param [in] type: 1:关闭TX1 2:关闭TX2 3:关闭TX1,TX2
 *
 *  @return
 *
 *  @details 
 */
void app_ci522_antenna_close(uint8_t type)
{
    if(type == 0 || type > 3)
    {
        return;
    }
    app_ci522_clear_bitmask(TxControlReg,type);
}

/**
 *  @brief ci522功能初始化
 *
 *  @param [in] 
 *
 *  @return
 *
 *  @details 
 */
void app_ci522_func_init()
{
    //app_ci522_reset(1);

    app_ci522_clear_bitmask(Status2Reg, 0x08);  

    // Reset baud rates
	app_ci522_write(TxModeReg, TxModeReg_Val);//写寄存器操作，定义发送时的速率
	app_ci522_write(RxModeReg, RxModeReg_Val);
	// Reset ModWidthReg
	app_ci522_write(ModWidthReg, ModWidthReg_Val);

	// RxGain:110,43dB by default
	app_ci522_write(RFCfgReg, RFCfgReg_Val);//增益
	
	// When communicating with a PICC we need a timeout if something goes wrong.
	// f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
	// TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
	app_ci522_write(TModeReg, TModeReg_Val);			// TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
	app_ci522_write(TPrescalerReg, TPrescalerReg_Val);	// TPreScaler = TModeReg[3..0]:TPrescalerReg
	app_ci522_write(TReloadRegH, TReloadRegH_Val);		// Reload timer 
	app_ci522_write(TReloadRegL, TReloadRegL_Val);
	
	app_ci522_write(TxASKReg, TxASKReg_Val);			// Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
	app_ci522_write(ModeReg, ModeReg_Val);				// Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
	
	// Turn on the analog part of receiver 
	app_ci522_write(CommandReg, PCD_IDLE);
    /*之前的工程在选卡阶段再进行开启天线，将两个天线都打开并进行初始化*/
    app_ci522_antenna_open(3);    //开启天线
}

/**
 *  @brief ci522计算CRC16校验值
 *
 *  @param [in] data 数据
 *  @param [in] datalen 数据长度
 *
 *  @return 校验值
 *
 *  @details 
 */
uint16_t app_ci522_cal_crc16_value(uint8_t *data,uint16_t datalen)
{
    app_ci522_clear_bitmask(DivIrqReg,0x04);
    app_ci522_write(CommandReg,PCD_IDLE);
    app_ci522_set_bitmask(FIFOLevelReg,0x80);

    uint8_t i = 0,n = 0;
    for (i = 0; i < datalen; i++)
    {
        app_ci522_write(FIFODataReg, *(data + i));
    }
    app_ci522_write(CommandReg, PCD_CALCCRC);
    i = 20;
    do 
    {
        n = app_ci522_read(DivIrqReg);
        i--;
        osDelay(1);
    }while ((i!=0) && !(n&0x04));

    uint16_t crc_value = app_ci522_read(CRCResultRegH) * 256 + app_ci522_read(CRCResultRegL);
    return crc_value;
}

/**
 *  @brief ci522与card通讯
 *
 *  @param [out] out_buf 接收到卡片返回的数据
 *  @param [out] out_buflen 接收到卡片返回的数据长度
 *  @param [in] command CI522命令字
 *  @param [in] data 发送的数据
 *  @param [in] datalen 发送的数据长度
 *
 *  @return CI522错误码
 *
 *  @details 
 */
int8_t app_ci522_comm_with_card(uint8_t *out_buf,uint16_t *out_buflen,uint8_t command,uint8_t *data,uint16_t datalen)
{
    uint8_t irqEn = 0x00,waitFor = 0x00,temp = 0,lastBits = 0;
    uint16_t timeout = 0;
    int8_t status = MI_ERR;
    switch (command)
    {
        /* 验证密钥 */
        case PCD_AUTHENT:
        {
            irqEn   = 0x12;
			waitFor = 0x10;
			break;
        }
        /* 发送并接收数据 */
		case PCD_TRANSCEIVE:
        {
            irqEn   = 0x77;
			waitFor = 0x30;
			break;
        }
		default:
			break;
    }
    // app_ci522_write(ComIEnReg,irqEn|0x80);
    app_ci522_clear_bitmask(ComIrqReg,0x80);
    app_ci522_write(CommandReg,PCD_IDLE);
    app_ci522_set_bitmask(FIFOLevelReg,0x80);

    for (uint8_t i = 0; i < datalen; i++)
    {
        app_ci522_write(FIFODataReg, data[i]);
    }

    app_ci522_write(CommandReg, command);
   
    if (command == PCD_TRANSCEIVE)
    { 
        app_ci522_set_bitmask(BitFramingReg,0x80);   
	}
    do 
    {
        timeout ++;
        if(timeout >= 50)
        {
            break;
        }
        temp = app_ci522_read(ComIrqReg);
        osDelay(1);
    }while (!(temp&0x01) && !(temp&waitFor));

    app_ci522_clear_bitmask(BitFramingReg,0x80);
   
    if(timeout < 50)
    {
        uint8_t error_value =  app_ci522_read(ErrorReg);
        if(!(error_value & 0x1B))//if(!(app_ci522_read(ErrorReg) & 0x1B))
        {
            status = MI_OK;
            if(temp & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }
            if (command == PCD_TRANSCEIVE)
            {
                temp = app_ci522_read(FIFOLevelReg);
                lastBits = app_ci522_read(ControlReg) & 0x07;
                if(lastBits)
                {
                    *out_buflen = (temp - 1) * 8 + lastBits;
                }
                else
                {
                    *out_buflen = temp * 8;
                }
                if(temp == 0)
                {
                    temp = 1;
                }
                if(temp > MAXRLEN)
                {
                    temp = MAXRLEN;
                }
                for(uint8_t i = 0;i < temp;i ++)
                {
                    *(out_buf + i) = app_ci522_read(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_NOCARD;
        }
    }
    //清楚标志位
    app_ci522_set_bitmask(ControlReg,0x80); 
    app_ci522_write(CommandReg,PCD_IDLE);
    return status;
}

/**
 *  @brief ci522计算CRC16校验值
 *
 *  @param [in] type 哪个区域搜索   1:TX1区域搜索 2:TX2区域搜索 3:TX1、TX2区域搜索
 *  @param [in] req_code 寻卡方式   0x52:感应区内所有符合14443A标准的卡; 0x26:寻未进入休眠状态的卡;
 *  @param [out] card_type 卡片类型代码   0x4400:Mifare_UltraLight 0x0400:Mifare_One(S50) 0x0200:Mifare_One(S70) 0x0800:Mifare_Pro(X) 0x4403:Mifare_DESFire
 *
 *  @return CI522错误码
 *
 *  @details 
 */
/*
type用于指定搜索的天线区域，并与卡进行通信
*/
int8_t app_ci522_search_card(uint8_t type,uint8_t req_code,uint16_t *card_type)
{
    int8_t status = MI_ERR;
    uint8_t data[MAXRLEN] = {0};
    uint16_t datalen = 0;

    app_ci522_clear_bitmask(Status2Reg,0x08);
    app_ci522_write(BitFramingReg,0x07);
    //选择打开天线的区域
    app_ci522_antenna_close(0x03);
    osDelay(50 / 5);
    app_ci522_antenna_open(type);
    //app_ci522_set_bitmask(TxControlReg,0x03);

    status = app_ci522_comm_with_card(data,&datalen,PCD_TRANSCEIVE,&req_code,1);
    
    if(status == MI_OK && datalen == 0x10)
    {
        *card_type     = data[0];
		*(card_type+1) = data[1];
    }
    else
    {
        status = MI_ERR;
    }
    return status;
}

/**
 *  @brief 防冲撞读取4位UID
 *
 *  @param [out] out_buf 卡片序列号
 *  @param [in] anticollision_level 防冲撞等级(0x93 防冲撞等级1 0x95 防冲撞等级2  0x97 防冲撞等级3
 *
 *  @return CI522错误码
 *
 *  @details 
 */
int8_t app_ci522_anticollision(uint8_t *out_buf,uint8_t anticollision_level)
{
    uint8_t data[MAXRLEN] = {0},snr_check = 0;
    uint16_t datalen = 0;
    int8_t  status = MI_ERR;

    app_ci522_clear_bitmask(Status2Reg,0x08);
    app_ci522_write(BitFramingReg,0x00);
    app_ci522_set_bitmask(CollReg,0x80);
    
    uint8_t cmd[4] = {0};
    cmd[0] = anticollision_level;
    cmd[1] = 0x20;

    status = app_ci522_comm_with_card(data,&datalen,PCD_TRANSCEIVE,cmd,2);

    if(status == MI_OK)
    {
        uint8_t i = 0;
        for( i = 0;i < 4; i++)
        {
            *(out_buf + i) = data[i];
            snr_check ^= data[i];
        }
        if(snr_check != data[i])
        {
            status = MI_ERR;
        }
    }
    app_ci522_set_bitmask(CollReg,0x80);
    return status;
}

/**
 *  @brief 防冲撞
 *
 *  @param [out] uuid uuid码
 *  @param [out] sak sak标志
 *  @param [in] anticollision_level 防冲撞水平(0x93 防冲撞水平1 0x95 防冲撞水平2  0x97 防冲撞水平3
 *
 *  @return CI522错误码
 *
 *  @details 
 */
int8_t app_ci522_select_card(uint8_t *uuid, uint8_t *sak,uint8_t anticollision_level)
{
    int8_t status = MI_ERR;
    uint8_t i = 0,data[MAXRLEN] = {0};
    uint16_t datalen = 0;
    
    data[0] = anticollision_level;
    data[1] = 0x70;
    data[6] = 0;
    for (i=0; i<4; i++)
    {
    	data[i+2] = *(uuid+i);
    	data[6]  ^= *(uuid+i);
    }
    uint16_t crc_value = app_ci522_cal_crc16_value(data,7); 
    data[7] =  crc_value % 256;
    data[8] =  crc_value / 256;                                                            
  
    app_ci522_clear_bitmask(Status2Reg,0x08);

    status = app_ci522_comm_with_card(data,&datalen,PCD_TRANSCEIVE,data,9);
    //这里的返回值为-1，直接忽略
    status = 0;   
    if ((status == MI_OK) && (datalen == 0x18))//长度为24
    {   
		*sak = data[0];
		status = MI_OK;  
	}
    else
    {   
		status = MI_ERR;    
	}
    return status;
}


/**
 *  @brief 进入休眠状态
 *
 *  @param [in]
 *
 *  @return CI522错误码
 *
 *  @details 
 */
int8_t app_ci522_halt()
{
    int8_t status = MI_ERR;
    uint8_t data[MAXRLEN] = {0}; 
    uint16_t datalen_bit = 0;

    data[0] = PICC_HALT;
    data[1] = 0;
    uint16_t crc_value =  app_ci522_cal_crc16_value(data,2);
    data[2] =  crc_value % 256;
    data[3] =  crc_value / 256;  
 
    status = app_ci522_comm_with_card(data,&datalen_bit,PCD_TRANSCEIVE,data,4);
    return status;
}

/**
 *  @brief  
 *
 *  @param [in] auth_mode 密码验证模式 0x60 = 验证A密钥 0x61 = 验证B密钥
 *  @param [in] addr 块地址(块号，M1-S50容量为8K 位EEPROM分为16个扇区，每个扇区为4块，每块16个字节,以块为存取单位，即地址范围0~63)
 *  @param [in] pKey 密码
 *  @param [in] pSnr 卡片序列号，4字节
 *
 *  @return CI522错误码
 *
 *  @details 
 */
int8_t app_ci522_authstate(uint8_t auth_mode,uint8_t addr,uint8_t *pKey,uint8_t *pSnr)
{
    int8_t status = MI_ERR;
    uint8_t data[MAXRLEN] = {0}; 
    uint16_t datalen_bit = 0;

    data[0] = auth_mode;
    data[1] = addr;
	memcpy(&data[2], pKey, 6); 
	memcpy(&data[8], pSnr, 6); 
    
    status = app_ci522_comm_with_card(data,&datalen_bit,PCD_AUTHENT,data,12);
    if ((status != MI_OK) || (!(app_ci522_read(Status2Reg) & 0x08)))
    {
		status = MI_ERR;   
	}
    return status;
}

/**
 *  @brief 读取M1卡一块数据
 *
 *  @param [in] addr 块地址(块号，M1-S50容量为8K 位EEPROM分为16个扇区，每个扇区为4块，每块16个字节,以块为存取单位，即地址范围0~63)
 *  @param [in] pKey 密码
 *  @param [in] pSnr 卡片序列号，4字节
 *
 *  @return CI522错误码
 *
 *  @details 
 */
/*
读取卡特定块数据的函数
存储结构
扇区：4个块为一个扇区
块：块是存储和访问的基本单位
扇区控制区：每个扇区的最后一个块被称为扇区控制块
*/
int8_t app_ci522_block_read(uint8_t addr,uint8_t *pData)
{
    int8_t status = MI_ERR;
    uint8_t data[MAXRLEN] = {0}; 
    uint16_t datalen_bit = 0;

    data[0] = PICC_READ;
    data[1] = addr;
    uint16_t crc_value =  app_ci522_cal_crc16_value(data,2);
    data[2] =  crc_value % 256;
    data[3] =  crc_value / 256; 
   
    status = app_ci522_comm_with_card(data,&datalen_bit,PCD_TRANSCEIVE,data,4);
    if ((status == MI_OK) && (datalen_bit == 0x90))
   	{   
		memcpy(pData, data, 16);   
	}
    else
    {   
		status = MI_ERR;   
	}
    return status;
}

/**
 *  @brief 写数据到M1卡一块
 *
 *  @param [in] addr 块地址(块号，M1-S50容量为8K 位EEPROM分为16个扇区，每个扇区为4块，每块16个字节,以块为存取单位，即地址范围0~63)
 *  @param [in] pData 写入的数据，16字节
 *
 *  @return CI522错误码
 *
 *  @details 
 */
int8_t app_ci522_block_write(uint8_t addr,uint8_t *pData)      
{
    int8_t status = MI_ERR;
    uint8_t data[MAXRLEN] = {0}; 
    uint16_t datalen_bit = 0;

    data[0] = PICC_WRITE ;
    data[1] = addr;
    uint16_t crc_value =  app_ci522_cal_crc16_value(data,2);
    data[2] =  crc_value % 256;
    data[3] =  crc_value / 256; 
    status = app_ci522_comm_with_card(data,&datalen_bit,PCD_TRANSCEIVE,data,4);
    if ((status != MI_OK) || (datalen_bit != 4) || ((data[0] & 0x0F) != 0x0A))
    {   
		status = MI_ERR;   
	}
        
    if (status == MI_OK)
    {
        memcpy(data, pData, 16);
        crc_value = app_ci522_cal_crc16_value(data,16);
        data[16] =  crc_value % 256;
        data[17] =  crc_value / 256; 

        status = app_ci522_comm_with_card(data,&datalen_bit,PCD_TRANSCEIVE,data,18);
        if ((status != MI_OK) || (datalen_bit != 4) || ((data[0] & 0x0F) != 0x0A))
        {   
			status = MI_ERR;   
		}
    }
    return status;
}
/**
 *  @brief 获取卡的uuid
 *
 *  @param [in] uuid 卡的uuid(4 byte)
 *
 *  @return CI522错误码
 *
 *  @details 
 */
int8_t app_ci522_read_uuid(uint8_t *uuid)
{
    uint16_t l_card_type = 0;
    //寻卡
    app_ci522_write(RFCfgReg, 0x68);             //复位接收增益
	if(app_ci522_search_card(0x03,PICC_REQIDL, &l_card_type) != MI_OK )  //寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
	{
		app_ci522_write(RFCfgReg, 0x48);
		if(app_ci522_search_card(0x03,PICC_REQIDL, &l_card_type) != MI_OK)
		{
			app_ci522_write(RFCfgReg, 0x58);
			if(app_ci522_search_card(0x03,PICC_REQIDL, &l_card_type) != MI_OK)
			{	
				return MI_NOTAGERR;
			}
		}		
	}
	//hal_info_log("search card success,card type is 0x%04x",l_card_type);

    //冲突检测 level1
    uint8_t sak = 0;
	if(app_ci522_anticollision(uuid, PICC_ANTICOLL1) != MI_OK) 
	{
		hal_info_log("Anticoll1:fail");
		return MI_NOTAGERR;		
	}

    if (app_ci522_select_card(uuid, &sak, PICC_ANTICOLL1) != MI_OK)
    {
        hal_info_log("Select1:fail");
        return 1;
    }
    hal_info_log("Select1:ok  SAK1:%02x",sak);

    if (sak & 0x04) //判断UUID是否完整
    {
        //Anticoll 冲突检测 level2
        if (app_ci522_anticollision(uuid, PICC_ANTICOLL2) != MI_OK)
        {
            hal_info_log("Anticoll2:fail");
            return MI_ERR;
        }
        if (app_ci522_select_card(uuid, &sak, PICC_ANTICOLL2) != MI_OK)
        {
            hal_info_log("Select2:fail");
            return MI_ERR;
        }
        hal_info_log("Select2:ok  SAK2:%02x", sak);

        if (sak & 0x04) //判断UUID是否完整
        {
            //Anticoll 冲突检测 level3
            if (app_ci522_anticollision(uuid, PICC_ANTICOLL3) != MI_OK)
            {
                hal_info_log("Anticoll3:fail");
                return MI_ERR;
            }
            if (app_ci522_select_card(uuid, &sak, PICC_ANTICOLL3) != MI_OK)
            {
                hal_info_log("Select3:fail");
                return MI_ERR;
            }
            else
            {
                hal_info_log("Select3:ok  SAK3:%02x", sak);
                if (sak & 0x04)
                {
                    return MI_ERR;
                }
            }
        }
    }
    hal_info_log("get uuid success,uuid is:%02x %02x %02x %02x",uuid[0],uuid[1],uuid[2],uuid[3]);	
    return MI_OK;	
}

/**
 * @author: tn
 * @description: 
 * @param {uint8_t} *indata
 * @param {uint8_t} *outdata
 * @param {uint8_t} dlen
 * @return {*}
 * 这个是转字符串的函数
 */
void HexToString(uint8_t *indata,uint8_t *outdata,uint8_t dlen)
{
    char c1=0,c2=0;
    uint8_t datalen=dlen;
    if(datalen>=8)
    {
        datalen=8;
    }
    for (int i = 0; i < datalen; i++)
    {
        c1 = indata[i] & 0xFu;
        c2 = (indata[i] >> 4) & 0xFu;
        sprintf((char *)outdata + i * 2, "%x%x", c2, c1);
    }
}

/**
 *  @brief 读取卡ID号
 *
 *  @param [in] type 哪个区域搜索0x01:TX1区域搜索 0x02:TX2区域搜索 0x03:TX1、TX2区域搜索
 *  @param [out] card_id  卡号缓冲区(32 byte卡号)
 *  @param [out] card_id_size  缓冲区大小
 *  @param [in] auth_key  鉴权码(固定为6字节)
 *  @param [in] auth_key_len  鉴权码长度
 *
 *  @return CI522错误码
 *
 *  @details 
 */
int8_t app_ci522_cardid_read(uint8_t type,uint8_t *card_id,uint16_t card_id_size,uint8_t *auth_key,uint8_t auth_key_len)
{
    if(card_id_size < 32 || auth_key_len < 6)
    {
        //hal_info_log("ci522 buf is small");
        return MI_ERR;
    }
    uint8_t uuid[4] = {0},sak = 0;
    uint16_t card_type = 0;
    //寻卡
	if(app_ci522_search_card(type,PICC_REQIDL, &card_type) != MI_OK)  //寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
	{
		return MI_NOTAGERR;		
	}
	hal_info_log("search card success,card type is 0x%04x",card_type);
	
    //Anticoll 冲突检测
    if (app_ci522_anticollision(uuid, PICC_ANTICOLL1) != MI_OK)
    {
        hal_info_log("anticoll fail");
        return MI_ERR;
    }
	hal_info_log("anticoll success,uuid:%02x %02x %02x %02x",uuid[0],uuid[1],uuid[2],uuid[3]);
	
    // Select 选卡
	if(app_ci522_select_card(uuid,&sak,PICC_ANTICOLL1) != MI_OK)
	{
		hal_info_log("Select card fail");
		return MI_ERR;		
	}
	hal_info_log("select card ok,SAK:0x%02x",sak);
	
    //Authenticate 验证密码
    uint8_t keybuf[6] = {0};
    memcpy(keybuf,auth_key,6);
    if(app_ci522_authstate(PICC_AUTHENT1A, 1, keybuf, uuid) != MI_OK)   //card id存放在0扇区1块区
    {
        hal_info_log("authenticate card key fail");
        return MI_ERR;
    }
    hal_info_log("authenticate card key success");

	//读BLOCK原始数据
    uint8_t card_read_buf[32] = {0};
	if( app_ci522_block_read(1, card_read_buf) != MI_OK )
	{
		hal_info_log("read card id fail");
		return MI_ERR;		
	}

    app_get_hex2hexstr(card_id,card_id_size,card_read_buf,16,1);
    hal_info_log("read card id success,card id:%s",card_id);

	return MI_OK;
}

/**
 * @author: tn
 * @description: 返回0读卡失败，返回1读卡成功
 * @param {uint8_t} type
 * @param {uint8_t} *readUid
 * @return {*}
 */
uint8_t readCard(uint8_t type,uint8_t *readUid,void(*funCallBack)(void))
{
	uint8_t Temp[5];
    if (app_ci522_search_card(type,PICC_REQALL, Temp) != 0)
	{//进行选卡范围判断
        app_ci522_write(RFCfgReg, 0x48);
        // if (app_ci522_search_card(type,PICC_REQALL, Temp) != 0)
        // {
        //     app_ci522_write(RFCfgReg, 0x58);
        //     if (app_ci522_search_card(type,PICC_REQALL, Temp) != 0)
        //     {
                // sysPrintf("RFID_find_fail\r\n");
        //     }
        // }
    }
    if (app_ci522_search_card(type,PICC_REQALL, Temp) == 0)
	{//两个天线都打开
		if (app_ci522_anticollision(readUid, PICC_ANTICOLL1) == 0)
		{
            // sysPrintf("RFID2=%02x-%02x-%02x-%02x",readUid[0],readUid[1],readUid[2],readUid[3]);
            if(funCallBack != NULL)
            {//后续进行系统升级编写
                funCallBack();
            }	
			return 1;
		}	
	}
	return 0;
}


/**
 * @author: tn
 * @description: 回调函数
 * 密码为HNCU
 * 验证密码成功会返回0x55 0x55
    H: 0x48
    N: 0x4E
    C: 0x43
    U: 0x55
 * @return {*}
 */
void funCallBack(void)
{
    int8_t sys_value = 0;
    uint8_t sak = 0;
    uuid[0]=readUid[1];
    uuid[1]=readUid[2];
    uuid[2]=readUid[3];

        app_ci522_select_card(readUid,&sak,PICC_ANTICOLL1);
        uint16_t datalen_bit = 0;
        uint8_t mess[10] = {0};
        uint8_t cmd[10] = {0};
            /*read操作  验证成功*/
        // cmd[0] = 0x30;
        // cmd[1] =  0x2B;
        // uint16_t crc_value =  app_ci522_cal_crc16_value(cmd,2);
        // cmd[2] =  crc_value % 256;
        // cmd[3] =  crc_value / 256;
            /*pwd_auth操作*/
        cmd[0] = 0x1B;
        cmd[1] = 0x48;
        cmd[2] = 0x4E;
        cmd[3] = 0x43;
        cmd[4] = 0x55;
        uint16_t crc_value =  app_ci522_cal_crc16_value(cmd,5);
        osDelay(50);
        cmd[5] =  crc_value % 256;
        cmd[6] =  crc_value / 256; 
            /*write密码操作*/
        // cmd[0] = 0xA2;
        // cmd[1] = 0x2B;
        // cmd[2] = 0x48;
        // cmd[3] = 0x4E;
        // cmd[4] = 0x43;
        // cmd[5] = 0x55;
        // uint16_t crc_value =  app_ci522_cal_crc16_value(cmd,6);
        // cmd[6] = crc_value % 256;
        // cmd[7] = crc_value / 256;
            /*writePACK操作*/
        // cmd[0] = 0xA2;
        // cmd[1] = 0x2C;
        // cmd[2] = 0x55;
        // cmd[3] = 0x55;
        // cmd[4] = 0x00;
        // cmd[5] = 0x00;
        // uint16_t crc_value =  app_ci522_cal_crc16_value(cmd,6);
        // cmd[6] = crc_value % 256;
        // cmd[7] = crc_value / 256;
    // if(sak&0x04)
    // {}//进入这个判断表示为7位
        if (app_ci522_anticollision(readUid+4, PICC_ANTICOLL2) == 0)
        {//二次防冲撞
            uuid[3]=readUid[4];
            uuid[4]=readUid[5];
            uuid[5]=readUid[6];
            uuid[6]=readUid[7];
            /*在二次防冲撞的基础上，加上二次选卡，进入状态二*/
            sys_value = app_ci522_select_card(readUid+4,&sak,PICC_ANTICOLL2);//进行选卡
            app_ci522_comm_with_card(mess,&datalen_bit,PCD_TRANSCEIVE,cmd,7);
            // sysPrintf("mess=%02x-%02x-%02x-%02x\r\n",mess[0],mess[1],mess[4],mess[3]);
            if(mess[0]==0x55&&mess[1]==0x55)
            {//合法卡
                /*检索鉴权卡号*/
                sysPrintf("合法卡=%02x-%02x-%02x-%02x-%02x-%02x-%02x\r\n",uuid[0],uuid[1],uuid[2],uuid[3],uuid[4],uuid[5],uuid[6]);
                ctrl_nfc = 1;       
            }
            else
            {
                sysPrintf("非法卡"); 
                ctrl_nfc = 2;  
            }
        }
}

/**
 * @author: tn
 * @description: 获取系统运行时间s
 * @return {*}
 */
uint32_t driver_time_getruntime()
{
    return (osKernelGetTickCount() * 5)/1000;
}

/**
 *  @brief ci522读卡线程
 *
 *  @param [in]
 *
 *  @return
 *
 *  @details 
 */
/*
使用spi1进行通信，复位引脚使能85引脚，中断悬空

RC522模块供电有问题，当设备的供电不足时，无法使用

不要随便接电源线   ///////
验证通过
*/
void nfc_task_entry(void)
{
    app_ci522_dev_init(APP_DEMO_CI522_SPI,APP_DEMO_CI522_GPI0_RST,APP_DEMO_CI522_GPI0_IRQ);   //初始化外设引脚
    app_ci522_func_init();      //初始化功能
    osDelay(2000 / 5);
    static unsigned int oldreboottime=0;
    while(1)
    {
        unsigned int nowstrlocaltime=driver_time_getruntime();
        if(nowstrlocaltime - oldreboottime >= 10||nfc_shell == 1)
        {//间隔10s或者手动重启
            cm_spi_close(APP_DEMO_CI522_SPI);
            app_ci522_dev_init(APP_DEMO_CI522_SPI,APP_DEMO_CI522_GPI0_RST,APP_DEMO_CI522_GPI0_IRQ);   //初始化外设引脚
            osDelay(2500 / 5);
            app_ci522_func_init();      //初始化功能
            nfc_shell = 0;
            // sysPrintf("NFC复位\r\n");
            osDelay(100 / 5);
            oldreboottime=nowstrlocaltime;
        }
        readCard(3,readUid,funCallBack);
        if(ctrl_nfc==1)
        {//表示检测到合法卡，鉴权成功
            chg_ctrl=1;//启动充电
            audio_play=2;//设备启动成功
            eeprom_addr();
        }
        if(ctrl_nfc==2)
        {//检测到非法卡
            audio_play=7;//非法卡
        }
        ctrl_nfc=0;
        osDelay(1000/5);    
    }
}
