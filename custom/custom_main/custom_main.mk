#定义 CUSTOM_MAIN_DIR  代表  custom/custom_main 路径名称
CUSTOM_MAIN_DIR := custom/custom_main

#追加源文件(.c文件)  (custom/custom_main/src/custom_main.c)
OC_FILES += $(CUSTOM_MAIN_DIR)/src/custom_main.c

#追加lib文件(.a文件)   (custom/custom_main/sys_package.a)
OC_LIBS  += $(CUSTOM_MAIN_DIR)/sys_package.a

#追加 .h文件路径 (custom/custom_main/inc)
INC      += -I'$(CUSTOM_MAIN_DIR)/inc'
#追加 .h文件路径  (custom/custom_main)
INC      += -I'$(CUSTOM_MAIN_DIR)'


#追加源文件(.c文件)
#因为OC_FILES 代表 custom/custom_main 路径名称 
#所以下面就是 OC_FILES +=custom/custom_main/src/test.c  (OC_FILES = OC_FILES + custom/custom_main/src/test.c)
#OC_FILES这个是固定的标识可以看成一个变量;  +=就是追加新的数据, 这里是追加新的文件路径
# OC_FILES += $(CUSTOM_MAIN_DIR)/src/test.c


#追加 .h文件路径 (custom/custom_main/src)
INC      += -I'$(CUSTOM_MAIN_DIR)/src'

OC_FILES += $(CUSTOM_MAIN_DIR)/src/uart.c
OC_FILES += $(CUSTOM_MAIN_DIR)/src/chg_module.c
OC_FILES += $(CUSTOM_MAIN_DIR)/src/shell_task.c
OC_FILES += $(CUSTOM_MAIN_DIR)/src/chg_nfc.c
OC_FILES += $(CUSTOM_MAIN_DIR)/src/chg_param.c
OC_FILES += $(CUSTOM_MAIN_DIR)/src/chg_ctrl.c
OC_FILES += $(CUSTOM_MAIN_DIR)/src/chg_meter.c
OC_FILES += $(CUSTOM_MAIN_DIR)/src/chg_audio.c
OC_FILES += $(CUSTOM_MAIN_DIR)/src/chg_dev.c
# OC_FILES += $(CUSTOM_MAIN_DIR)/src/cJSON.c