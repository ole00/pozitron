GAME = Arkanoid

KERNEL_DIR = ../../../kernel
KERNEL_OPTIONS  = -DVIDEO_MODE=3 -DINTRO_LOGO=0 -DSCROLLING=0
KERNEL_OPTIONS += -DSOUND_MIXER=1 -DSOUND_CHANNEL_5_ENABLE=0
KERNEL_OPTIONS += -DXXMIXER_WAVES=\"$(realpath ../data/arkanoid_sounds.inc)\" -DMAX_SPRITES=20 -DRAM_TILES_COUNT=36 -DVRAM_TILES_V=29
KERNEL_OPTIONS += -DOVERLAY_LINES=0

# Only necessary if scrolling is enabled
#KERNEL_OPTIONS += -DVRAM_TILES_V=32


# Put your STM32F1 library code directory here
STM_COMMON=$(KERNEL_DIR)/Libraries

# Put your source files here (or *.c, etc)
SRCS=../$(PROJ_NAME).c

# Put your preprocesor defines here
#DEFINES=-DSHOW_FRAME_COUNTER

# Include the common makefile
include $(KERNEL_DIR)/common.mk

CFLAGS += -fsigned-char
CFLAGS += -ffunction-sections -fno-toplevel-reorder
