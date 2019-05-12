
/* Includes */
#include <stddef.h>
#include "stm32f10x.h"

//Timer 4 channel 1 is connected / hardwired to Port B pin 6 
#define DAC_GPIO_RC        RCC_APB2Periph_GPIOB
#define DAC_GPIO_PORT      GPIOB
#define DAC_GPIO_PIN       ( GPIO_Pin_6) 

#define NOP __asm("nop")

void DAC_GPIO_Config(void)
{	
	
    GPIO_InitTypeDef  GPIO_InitStructure;	
    RCC_APB2PeriphClockCmd(DAC_GPIO_RC,ENABLE);


    GPIO_InitStructure.GPIO_Pin = DAC_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //Note: we use alternative function (AF) mode here  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DAC_GPIO_PORT, &GPIO_InitStructure);
}

static void setupInterrupt(int prescale) {
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	//value 71 (0..71 is 72 values) -> triggers the interrup every (1 uSec * prescale)
    //when the processor clock is 72MHz.

	// timer 4
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseStructure.TIM_Period = 72 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = prescale - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_SetCounter(TIM4, 0);
	TIM_Cmd(TIM4, ENABLE);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
}

static void enableTimerInterrupts()
{
    NVIC_InitTypeDef nvicStructure;
    nvicStructure.NVIC_IRQChannel = TIM4_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}

static void setupPwm(void) {
    TIM_OCInitTypeDef TIM_OCInitStructure;

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_Pulse = 12; //after 12 pulses toggle the pin
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //Start low then rise high
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
}

static void hw_init(void) {
	DAC_GPIO_Config();
	setupInterrupt(8);
    setupPwm();
	enableTimerInterrupts();
}


int main(void)
{
	//init hardware and timer
	hw_init();

	// Keep running, the timer should atomatically and periodically toggle the D6 pin.
	while (1) {
		NOP;
	}
}

