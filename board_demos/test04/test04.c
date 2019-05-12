
/* Includes */
#include <stddef.h>
#include "stm32f10x.h"

//DAC connected to port D
#define DAC_GPIO_RC        RCC_APB2Periph_GPIOD
#define DAC_GPIO_PORT      GPIOD
#define DAC_GPIO_PIN       ( GPIO_Pin_15) 
#define DAT_GPIO_ONOFF(x)  GPIO_WriteBit(DAC_PORT, DAC_GPIO_PIN,x);

#define NOP __asm("nop")

void DAC_GPIO_Config(void)
{	
	
    GPIO_InitTypeDef  GPIO_InitStructure;	
    RCC_APB2PeriphClockCmd(DAC_GPIO_RC,ENABLE);


    GPIO_InitStructure.GPIO_Pin = DAC_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DAC_GPIO_PORT, &GPIO_InitStructure);
}

static void setupSyncTimer(int prescale) {
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	//value 144 -> triggers the ISR every 2 uSec

	// timer 2 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseStructure.TIM_Period = 144;
	TIM_TimeBaseStructure.TIM_Prescaler = prescale - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_SetCounter(TIM2, 0);
	TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void enableTimerInterrupts()
{
    NVIC_InitTypeDef nvicStructure;
    nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}

void DACToggle(void)
{
	DAC_GPIO_PORT->ODR ^= DAC_GPIO_PIN;
}


void hw_init(void) {
	DAC_GPIO_Config();
	setupSyncTimer(1);
	enableTimerInterrupts();
}

// this function is called from within the interrupt handler when timer is triggered
void TIM2_IRQHandler(void) {

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

	//toggle GPIO, Port D, bit 15 (D15) 
	DACToggle();
}

int main(void)
{
	//init hardware and timer
	hw_init();

	// Keep running, the timer should trigger ISR (TIM2_IRQHandler function) periodically
	// every 2 micro seconds
	while (1) {
		NOP;
	}
}

