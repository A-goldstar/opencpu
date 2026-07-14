
CM_CYCLEQUEUE_DIR :=src/cm_lib/mcu_sys


# OC_FILES +=  $(CM_CYCLEQUEUE_DIR)/mcu_sys.c
OC_LIBS+= $(CM_CYCLEQUEUE_DIR)/mcu_sys.a
INC      += -I'$(CM_CYCLEQUEUE_DIR)'