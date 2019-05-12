
/* Includes */
#include <stddef.h>
#include "stm32f10x.h"

//DAC connected to port D
#define DAC_GPIO_RC        RCC_APB2Periph_GPIOD
#define DAC_GPIO_PORT      GPIOD
#define DAC_GPIO_PIN       ( GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_15) 
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

void DACToggle(void)
{
	DAC_GPIO_PORT->ODR ^= DAC_GPIO_PIN;
}

void Delay_ticks( uint16_t ticks )
{
  uint16_t i;
  ticks >>= 1;
  for(i = 0; i < ticks; i++)
  {
  	NOP;
  }
}

int main(void)
{
	int i;

	DAC_GPIO_Config();

	while (1) {
	
		for (i = 0; i < 4096; i++) {
			DAC_GPIO_PORT->ODR = i; // | 0x8000;
			Delay_ticks(100);
		}
	}
}




