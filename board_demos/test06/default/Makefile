#no kernel code
NO_KERNEL=1

# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=test06

KERNEL_DIR=../../../kernel

# Put your STM32F1 library code directory here
STM_COMMON=$(KERNEL_DIR)/Libraries

# Put your source files here (or *.c, etc)
SRCS=../$(PROJ_NAME).c ../video_out.c

# Put your preprocesor defines here
DEFINES=-DSHOW_FRAME_COUNTER

# Include the common makefile
include $(KERNEL_DIR)/common.mk

