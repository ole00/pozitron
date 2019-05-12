GAME = MusicDemo

KERNEL_DIR = ../../../kernel
KERNEL_OPTIONS  = -DVIDEO_MODE=1 -DINTRO_LOGO=0 -DMIDI_IN=1 -DMODE1_FAST_VSYNC=0
KERNEL_OPTIONS += -DSOUND_MIXER=0 -DUART_RX_BUFFER_SIZE=256

# Put your STM32F1 library code directory here
STM_COMMON=$(KERNEL_DIR)/Libraries

# Put your source files here (or *.c, etc)
SRCS=../$(PROJ_NAME).c

# Put your preprocesor defines here
#DEFINES=-DSHOW_FRAME_COUNTER

# Include the common makefile
include $(KERNEL_DIR)/common.mk

CFLAGS += -fsigned-char
