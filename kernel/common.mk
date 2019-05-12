
# global user defined settings
include ../../../kernel/user.mk


ifndef KERNEL_DIR
KERNEL_DIR=../../../kernel
endif

# Put your STM32F1 library code directory here
STM_COMMON=$(KERNEL_DIR)/Libraries

# Put your source files here (or *.c, etc)
SRCS += $(STM_COMMON)/system_stm32f10x.c

ifndef PROJ_NAME
PROJ_NAME=$(GAME)
endif

# Normally you shouldn't need to change anything below this line!
#######################################################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump

CFLAGS  = -Os -Wall -T$(STM_COMMON)/stm32_flash.ld
#CFLAGS += -g
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m3 -mthumb-interwork
CFLAGS += -mfloat-abi=soft
CFLAGS += -DSTM32F10X_HD -DUSE_STDPERIPH_DRIVER
CFLAGS += -I.
CFLAGS += -std=c99
CFLAGS += -DSTM_CPU


# DEFINES specified in project makefile
CFLAGS += $(KERNEL_OPTIONS)
CFLAGS += $(DEFINES)

# Include files from STM libraries
CFLAGS += -I$(STM_COMMON)/CMSIS/Include -I$(STM_COMMON)/CMSIS/Device/ST/STM32F10x/Include
CFLAGS += -I$(STM_COMMON)/STM32F10x_StdPeriph_Driver/inc
CFLAGS += -I$(STM_COMMON)/.
CFLAGS += -I$(KERNEL_DIR)/.

# add startup file to build
SRCS += $(STM_COMMON)/startup_stm32f10x_hd.s
# library code
SRCS += stm32f10x_gpio.c stm32f10x_rcc.c stm32f10x_tim.c stm32f10x_flash.c misc.c

#uzebox kernel code
ifndef NO_KERNEL
CFLAGS += -DCUSTOM_INITIALIZE
SRCS += $(KERNEL_DIR)/uzeboxCore.c
SRCS += $(KERNEL_DIR)/uzeboxVideoEngine.c
SRCS += $(KERNEL_DIR)/uzeboxSoundEngine.c
SRCS += $(KERNEL_DIR)/uzeboxVideoEngineCore.c
SRCS += $(KERNEL_DIR)/uzeboxSoundEngineCore.c

endif


vpath %.c \
$(STM_COMMON)/STM32F10x_StdPeriph_Driver/src \

#$(STM_COMMON)/Libraries/STM32_USB_OTG_Driver/src \
#$(STM_COMMON)/Libraries/STM32_USB_Device_Library/Core/src \
#$(STM_COMMON)/Libraries/STM32_USB_Device_Library/Class/cdc/src


OBJS = $(SRCS:.c=.o)

BUILD_DIR := .

.PHONY: proj
all: proj

proj: setup $(BUILD_DIR)/$(PROJ_NAME).elf

.PHONY: setup
setup:
	@echo "Builing $(PROJ_NAME) ..."

$(BUILD_DIR)/$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ 
	$(OBJCOPY) -O ihex $(BUILD_DIR)/$(PROJ_NAME).elf $(BUILD_DIR)/$(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(BUILD_DIR)/$(PROJ_NAME).elf $(BUILD_DIR)/$(PROJ_NAME).bin

clean:
	@rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.elf $(BUILD_DIR)/*.hex

# Flash the STM32F4
burn: proj
	$(STLINK)/st-flash write $(BUILD_DIR)/$(PROJ_NAME).bin 0x8000000

objdump: proj
	$(OBJDUMP) -D $(BUILD_DIR)/$(PROJ_NAME).elf > $(BUILD_DIR)/$(PROJ_NAME).dump
	
