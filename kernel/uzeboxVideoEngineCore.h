

/** 
 * ==============================================================================
 *
 * This file contains global definitions for video cores.
 *
 * ===============================================================================
 */
#pragma once


//SYNC connected to port C
#define SYNC_GPIO_RC       RCC_APB2Periph_GPIOC
#define SYNC_GPIO_PORT     GPIOC
#define SYNC_GPIO_PIN      GPIO_Pin_7

#define OUT_0		BRR
#define OUT_1		BSRR

//DAC connected to port D
#define DAC_GPIO_RC        RCC_APB2Periph_GPIOD
#define DAC_GPIO_PORT      GPIOD
#define DAC_GPIO_PIN       GPIO_Pin_All 

//JOYSTICK connected to port E
#define JOY_GPIO_RC        RCC_APB2Periph_GPIOE
#define JOY_GPIO_PORT      GPIOE
#define JOY_GPIO_PIN       GPIO_Pin_All 


//-----------------------------------------

#define APB1_DIV 1

#define VGA_MODE 256
#define PAL_MODE 256
#define VGA_BPP 16


#define VGA_V_PIXELS 224
#define VGA_V_FRONTPORCH 4
#define VGA_V_SYNC 9
#define VGA_V_BACKPORCH 25

// total 262 lines


//DMA clocks per pixel - CPU @ 186MHz
//#define VGA_PIXELCLOCK (8064/PAL_MODE)

//DMA clocks per pixel - CPU @ 72MHz
//#define VGA_PIXELCLOCK (3542/PAL_MODE)

// It looks like the minimal number of clocks the DMA needs for the transfer is 11
#define VGA_PIXELCLOCK 18

#define SYSCLK 96000000
#define SYSCLK_MHZ 96
#define VGA_VFREQ 15625

// total 64us (48 + 4 + 4 + 8) per scanline
#define VGA_H_PIXELS     48 * SYSCLK_MHZ / VGA_PIXELCLOCK
#define VGA_H_FRONTPORCH  4 * SYSCLK_MHZ / VGA_PIXELCLOCK
#define VGA_H_SYNC        4 * SYSCLK_MHZ / VGA_PIXELCLOCK
#define VGA_H_BACKPORCH   8 * SYSCLK_MHZ / VGA_PIXELCLOCK

#define TIMER_CYCL    (SYSCLK / VGA_VFREQ / APB1_DIV)
#define H_TOTAL       (VGA_H_PIXELS + VGA_H_SYNC + VGA_H_FRONTPORCH + VGA_H_BACKPORCH)
#define SYNC_END      (VGA_H_SYNC * TIMER_CYCL / H_TOTAL)
#define BACKPORCH_END ((VGA_H_SYNC + VGA_H_BACKPORCH) * TIMER_CYCL / H_TOTAL)
#define LONGSYNC_END  ((H_TOTAL - VGA_H_SYNC) * TIMER_CYCL / H_TOTAL)


