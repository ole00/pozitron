
/* Includes */
#include <stddef.h>
#include "stm32f10x.h"

//On-board LED is defined as Port C13
#define LED3_GPIO_RCC       RCC_APB2Periph_GPIOC
#define LED3_GPIO_PORT      GPIOC
#define LED3_GPIO_PIN      	GPIO_Pin_13
#define LED3_ONOFF(x)      	GPIO_WriteBit(LED3_GPIO_PORT,LED3_GPIO_PIN,x);

#define NOP __asm("nop")

void LED_GPIO_Config(void)
{	
	
    GPIO_InitTypeDef  GPIO_InitStructure;	
    RCC_APB2PeriphClockCmd(LED3_GPIO_RCC,ENABLE);


    GPIO_InitStructure.GPIO_Pin =LED3_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 			 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStructure);
}

void LEDToggle(void)
{
	LED3_GPIO_PORT->ODR ^= LED3_GPIO_PIN;
}

void Delay_ms( uint16_t time_ms )
{
  uint16_t i,j;
  for( i=0;i<time_ms;i++ )
  {
		for( j=0;j<8309;j++ ) NOP;
  }
}

int main(void)
{

	LED_GPIO_Config();

	while (1) {
		LEDToggle();
    	Delay_ms(1000);

		LEDToggle();
    	Delay_ms(1000);
	}
}




