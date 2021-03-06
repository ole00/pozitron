/*

HSYNC & DMA pixel transfer test
=================================
This code was adapted from bitBox code (see vga_pal.c).
See: https://github.com/makapuf/bitbox-sdk.git

This code generates:
* H-sync pulses on Port B pin 7 via HW timer driven PWM
* Pixel output on Port D pins 0-16 via DMA1 Channel 5

Setup:
- Timer 4 channel 2 is used for PWM.
- Timer 4 channel 3 is used to trigger Timer 1.
- Timer 1 UP event is hardwired to trigger DMA1_Channel5 transactions
- DMA1_Channel5 is set up to copy pixels from 'dma_buffer' to Port D

The code assumes the CPU runs at 72MHz in order to generate 15.625KHz signal.

Test:
- Use oscilloscope CH1 connected to GPIO pin B7: view the Hsync pulses
- Use oscilloscope CH2 connected to resitor based DAC on GPIO pins D0-D2: view the RED pixels
 

*/

#pragma GCC push_options
#pragma GCC optimize ("O1")

#include <stddef.h>
#include "stm32f10x.h"

#define DAC_GPIO_RC        RCC_APB2Periph_GPIOD
#define DAC_GPIO_PORT      GPIOD
#define DAC_GPIO_PIN       ( GPIO_Pin_All) 

#define NOP __asm("nop")

#define APB1_DIV 1

#define VGA_MODE 244
#define PAL_MODE 244
#define VGA_BPP 16


#define VGA_V_PIXELS 224
#define VGA_V_FRONTPORCH 2
#define VGA_V_SYNC 2
#define VGA_V_BACKPORCH 25


//DMA clocks per pixel - CPU @ 186MHz
//#define VGA_PIXELCLOCK (8064/PAL_MODE)

//DMA clocks per pixel - CPU @ 72MHz
//#define VGA_PIXELCLOCK (3542/PAL_MODE)

// It looks like the minimal number of clocks the DMA needs for the transfer is 11
#define VGA_PIXELCLOCK 14

#define SYSCLK 72000000
#define SYSCLK_MHZ 72
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

uint16_t lineBuffer1[1024] __attribute__((aligned (1024)));
uint16_t lineBuffer2[1024] __attribute__((aligned (1024)));

//uint16_t lineBuffer1[512];
//uint16_t lineBuffer2[512];


uint16_t* displayBuffer = lineBuffer1; // will be sent to display
uint16_t* drawBuffer = lineBuffer2;    // will be drawn (bg already drawn)


volatile uint32_t videoLine;
volatile uint32_t videoFrame;

void drawScanLine(void);



static void setupGpioDac(void)
{	
    GPIO_InitTypeDef  GPIO_InitStructure;	
    RCC_APB2PeriphClockCmd(DAC_GPIO_RC,ENABLE);

    GPIO_InitStructure.GPIO_Pin = DAC_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DAC_GPIO_PORT, &GPIO_InitStructure);
}

static void setupGpioHsync(void)
{	
	//Timer 4 channel 2 is connected / hardwired to Port B pin 7 in Alternative function mode
    GPIO_InitTypeDef  GPIO_InitStructure;	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //Note: we use alternative function (AF) mode here  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}


static void setupTimers(void) {

	// enable timer 4
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

	TIM4->PSC = 0; // XXX debug Prescaler = 1 : PSC=0
	TIM4->CR1 = TIM_CR1_ARPE; // autoreload preload enable, no other function on
	TIM4->DIER = TIM_DIER_UIE; // Enable update interrupt.
	TIM4->CCER = 0; // Disable CC, so we can program it.
	TIM4->ARR = TIMER_CYCL - 1; // 96 MHz (OC) / 31.46875 kHz = 3 050.64548 (96 = 192MHz / 2 )

	// -- Channel 2  : Hsync pulse

	// On CNT==0: sync pulse start
	// set to PWM mode active level on reload, passive level after CC2-match.
	TIM4->CCMR1 = 6 * TIM_CCMR1_OC2M_0;
	// output enabled, reversed polarity (active low).
	TIM4->CCER = TIM_CCER_CC2E | TIM_CCER_CC2P;
	// 88 MHz * 3.813 microseconds = 335.544 - sync pulse end
	// TIM5->CCR2=336;
    // 96 MHz * 3.813 microseconds = 366.048 - sync pulse end

	TIM4->CCR2 = SYNC_END;

	// -- Channel 3 : Trigger signal for TIM1 - will start on ITR1

	// Master mode selection OC3REF signal is used as trigger output (TRGO)
	TIM4->CR2 = (0b110 * TIM_CR2_MMS_0);

	// Channel 3 set to passive level on reload, active level after CC3-match.
	TIM4->CCMR2= 7 * TIM_CCMR2_OC3M_0;

	// 96 MHz * (3.813 + 1.907) microseconds = 549.12 - back porch end, start pixel clock
    // -14 is a kludge to account for slow start of timer.
	TIM4->CCR3 = BACKPORCH_END - 14;

	// Enable HSync timer.

	// -- Channel 4 : software Hsync interrupt
	TIM4->CCR4 = BACKPORCH_END;

	// Enable HSync timer interrupt and set highest priority.
	//InstallInterruptHandler(TIM5_IRQn,TIM5_IRQHandler);
	NVIC_EnableIRQ(TIM4_IRQn);
	NVIC_SetPriority(TIM4_IRQn, 0);

	TIM4->CNT = -10; // Make sure it hits ARR.
	TIM4->CR1 |= TIM_CR1_CEN; // Go.


	// --- TIMER 1 : DMA1 pixel clock
	// enable it
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	// Prescaler = 1.
	TIM1->PSC = 0;
	// loop each "pixelclock"
	TIM1->ARR = VGA_PIXELCLOCK - 1;
	// autoreload preload enable, no other function on
	TIM1->CR1 = TIM_CR1_ARPE;
	// Enable update DMA request interrupt
	TIM1->DIER = TIM_DIER_UDE;
	// Only run TIM1 when TIM4 trigger-out is high

	// SMS=5 : set Gated mode (clk enabled on input),
	// TS: selection ITR3 : for timer 1 this is timer 4 (Table 82. TIMx Internal trigger connection)
	TIM1->SMCR = (5 * TIM_SMCR_SMS_0) | (3 * TIM_SMCR_TS_0);

	// --- DMA -----------------------------------------------------------------------------------------

	// TIM1_UP drives the DMA1 channel 5 (Table 78. Summary of DMA1 requests for each channel)
    RCC->AHBENR |= RCC_AHBENR_DMA1EN; // DMA1 enable

    // --- Interrupt priorities ------------------------------------------------------------------------

    NVIC_SetPriority(DMA1_Channel5_IRQn, 0);
    NVIC_SetPriority(TIM1_UP_IRQn, 4);
    NVIC_SetPriority(TIM1_TRG_COM_IRQn, 4);
    NVIC_SetPriority(TIM1_CC_IRQn, 4);
    NVIC_SetPriority(TIM1_BRK_IRQn, 4);
    NVIC_SetPriority(TIM4_IRQn, 1);

}

// Visible line. Configure and enable pixel DMA.
static void setupPixelDma()
{
    DMA1_Channel5->CCR =
        (3 * DMA_CCR1_PL_0) |    // Very high priority
        (1 * DMA_CCR1_PSIZE_0) | // PSIZE = 16 bit
        (1 * DMA_CCR1_MSIZE_0) | // MSIZE = 16 bit
        DMA_CCR1_MINC |          // Increase memory address
        (1 * DMA_CCR1_DIR) |     // read from memory
        DMA_CCR1_TCIE            // Transfer complete interrupt
        ;

    DMA1_Channel5->CNDTR = VGA_MODE + 1; // transfer N pixels + one last _black_ pixel
    DMA1_Channel5->CPAR = ((uint32_t)&(DAC_GPIO_PORT->ODR)); //transfer to DAC port (Port D)
    DMA1_Channel5->CMAR = (uint32_t)displayBuffer; // use this buffer source buffer

	// Enable pixel clock. Clock will only start once TIM1 allows it.
	TIM1->DIER = 0; // Update DMA request has to be disabled while zeroing the counter.
	TIM1->EGR = TIM_EGR_UG; // Force an update event to reset counter. Setting CNT is not reliable.
	TIM1->DIER = TIM_DIER_UDE; // Re-enable update DMA request.

	TIM1->CR1 |= TIM_CR1_CEN; // go .. when slave active

    DMA1_Channel5->CCR |= DMA_CCR1_EN; //Enable DMA1 channel 5
}

static inline void videoRaiseVSync()
{
	TIM4->CCR2 = SYNC_END; // use "short" sync signal
	GPIOB->BSRR |= GPIO_Pin_7; // raise VSync line
}

static inline void videoLowerVSync()
{
	TIM4->CCR2 = LONGSYNC_END; // use "long" sync signal
}

 // Hsync Handler
void __attribute__ ((used)) TIM4_IRQHandler() {

    TIM4->SR=0; // clear pending interrupts

    videoLine++;

    if (videoLine < VGA_V_PIXELS) {
        // swap display & draw buffers, effectively draws line-1
		uint16_t *t;
		t = displayBuffer;
		displayBuffer = drawBuffer;
		drawBuffer = t;

        setupPixelDma(); // DMA will be triggered

        drawScanLine(); //game callback
        
    } else {
        //drawVSync();
        GPIOD->ODR = 0; // send 0 to DAC

        if (videoLine == VGA_V_PIXELS) {
			videoFrame++; // new frame sync now.
		} else

        if (videoLine == VGA_V_PIXELS + VGA_V_FRONTPORCH + 1) {
			videoLowerVSync();
		} else if(videoLine == VGA_V_PIXELS + 1 + VGA_V_FRONTPORCH + VGA_V_SYNC)	{
			videoRaiseVSync();
		} else if(videoLine == VGA_V_PIXELS + VGA_V_FRONTPORCH + VGA_V_SYNC + VGA_V_BACKPORCH) {
			videoLine = 0;
            drawScanLine();  // first line next frame!
		}
    }

}


 // DMA handler of Transfer-Complete event
void __attribute__ ((used)) DMA1_Channel5_IRQHandler() {
    GPIOD->ODR = 0; // send 0 to DAC

    // Clear Transfer complete interrupt flag of DMA1_Channel5
	do {
		DMA1->IFCR |= DMA_IFCR_CTCIF5;
	} while (DMA1->ISR & DMA_ISR_TCIF5);    
   

    // stop DMA
	TIM1->CR1 &= ~TIM_CR1_CEN; // Stop pixel clock.
	DMA1_Channel5->CCR = 0; // Disable pixel DMA.

    // this will trigger a new interrupt. need to get rid of it !
	NVIC_ClearPendingIRQ (DMA1_Channel5_IRQn);    // clear pending DMA IRQ from the NVIC

}



static void setupDmaBuffer(void) {
    int i;

    for (i = 0; i <= VGA_MODE; i++) {
        lineBuffer1[i] = 0;
        lineBuffer2[i] = 0;
    }

    displayBuffer = lineBuffer1;
	drawBuffer = lineBuffer2;
    
    videoLine = 0;
	videoFrame = 0;
}

void initVideo(void) {

    setupDmaBuffer();
	
    setupGpioDac();
    setupGpioHsync();
    setupTimers();

    //enableDmaInterruptHandler();
	//enableTimerInterruptHandler();
}


