GAME = Maze

KERNEL_DIR = ../../../kernel
KERNEL_OPTIONS  = -DVIDEO_MODE=1 -DINTRO_LOGO=0 -DCENTER_ADJUSTMENT=-44 -DMODE1_FAST_VSYNC=0 -DTRUE_RANDOM_GEN=1

# Put your STM32F1 library code directory here
STM_COMMON=$(KERNEL_DIR)/Libraries

# Put your source files here (or *.c, etc)
SRCS=../$(PROJ_NAME).c

# Put your preprocesor defines here
#DEFINES=-DSHOW_FRAME_COUNTER

# Include the common makefile
include $(KERNEL_DIR)/common.mk

CFLAGS += -fsigned-char

