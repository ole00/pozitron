/*
 *  Uzebox Kernel
 *  Copyright (C) 2008-2009 Alec Bourque
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Uzebox is a reserved trade mark
*/

#include <stdbool.h>

#include "uzebox.h"

//Callbacks defined in each video modes module
extern void DisplayLogo(); 
extern void InitializeVideoMode();
extern void InitSoundPort();
extern void InitializeVideoCore(void);

extern unsigned char render_lines_count;
extern unsigned char first_render_line;
extern unsigned char sound_enabled;

volatile uint16_t joystickState;

/**
 * Dynamically sets the rasterizer parameters:
 * firstScanlineToRender = First scanline to render
 * scanlinesToRender     = Total number of vertical lines to render. 
 */
void SetRenderingParameters(u8 firstScanlineToRender, u8 scanlinesToRender){        
	render_lines_count=scanlinesToRender;
	first_render_line=firstScanlineToRender;
}

#if 1

// http://blog.tkjelectronics.dk/2010/02/stm32-overclocking/
static void setClock() {
	ErrorStatus HSEStartUpStatus;
	/* RCC system reset(for debug purpose) */
	RCC_DeInit();

	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);

	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if(HSEStartUpStatus == SUCCESS)
	{
		/* Enable Prefetch Buffer */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

		/* Flash 2 wait state */
		FLASH_SetLatency(FLASH_Latency_2);

		/* HCLK = SYSCLK */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);

		/* PCLK2 = HCLK */
		RCC_PCLK2Config(RCC_HCLK_Div1);

		/* PCLK1 = HCLK/2 */
		RCC_PCLK1Config(RCC_HCLK_Div1);

		/* PLLCLK = 8MHz * 9 = 72 MHz */
		//RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
		/* PLLCLK = 8MHz * 16 = 128 MHz */
		/* PLLCLK = 8MHz * 12 = 96 MHz */

		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_12);
		// The frequency has also been changed in system_stm32f10x

		/* Enable PLL */
		RCC_PLLCmd(ENABLE);

		/* Wait till PLL is ready */
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
			{;}

		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source */
		while(RCC_GetSYSCLKSource() != 0x08)
			{;}
	}

	/* Enable peripheral clocks --------------------------------------------------*/
	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
		| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG
		| RCC_APB2Periph_AFIO | RCC_APB2Periph_TIM1 , ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_DAC , ENABLE);
}
#endif


#ifdef CUSTOM_INITIALIZE
void Initialize(void) {

	setClock();

	//set rendering parameters
	render_lines_count=FRAME_LINES;
	first_render_line=FIRST_RENDER_LINE;

	#if SOUND_MIXER == MEXER_TYPE_VSYNC
		//Initialize the mixer buffer
		for(int j = 0; j < MIX_BANK_SIZE * 2; j++) {
			mix_buf[j]=0x80;
		}	
	
		mix_pos = mix_buf;
		mix_bank = 0;
	#endif

	#if MIXER_CHAN4_TYPE == 0
		{
			struct MixerNoiseChannelStruct* noise;		
			noise = &mixer.channels.type.noise;
			//initialise LSFR
			noise->barrel = 0x101;
			noise->params = 1;
		}
	#endif

	//silence all sound channels
	for(int i = 0; i < CHANNELS; i++){
		mixer.channels.all[i].volume = 0;
	}

	InitSoundPort();

    InitializeVideoCore();

    InitializeVideoMode();

    DisplayLogo();


}

#endif


/*
 * Reads the power button status. 
 *
 * Returns: true if pressed.
 */
bool IsPowerSwitchPressed(){
	
	return 	(joystickState & (BTN_START | BTN_SELECT)) == (BTN_START | BTN_SELECT) ;
	
}


#if USE_EEPROM==1
/*
 * Write the specified structure into the specified EEPROM block id. 
 * If the block does not exist, it is created.
 *
 * Returns:
 *  0x00 = Success
 * 	0x01 = EEPROM_ERROR_INVALID_BLOCK
 *	0x02 = EEPROM_ERROR_FULL
 */
char EepromWriteBlock(struct EepromBlockStruct *block){
	u16 eepromAddr=0;
	u8 *srcPtr=(unsigned char *)block,res,nextFreeBlock=0;

	res=EepromBlockExists(block->id,&eepromAddr,&nextFreeBlock);
	if(res!=0 && res!=EEPROM_ERROR_BLOCK_NOT_FOUND) return res;

	if(eepromAddr==0 && nextFreeBlock==0) return EEPROM_ERROR_FULL;
	if(eepromAddr==0 && nextFreeBlock!=0) eepromAddr=nextFreeBlock*EEPROM_BLOCK_SIZE;

	for(u8 i=0;i<EEPROM_BLOCK_SIZE;i++){
		WriteEeprom(eepromAddr++,*srcPtr);
		srcPtr++;	
	}
	
	return 0;
}

/*
 * Loads a data block from the in EEPROM into the specified structure.
 *
 * Returns: 
 *  0x00 = Success
 * 	0x01 = EEPROM_ERROR_INVALID_BLOCK
 *	0x03 = EEPROM_ERROR_BLOCK_NOT_FOUND
 */
char EepromReadBlock(unsigned int blockId,struct EepromBlockStruct *block){	
	u16 eepromAddr;
	u8 *blockPtr=(unsigned char *)block;

	u8 res=EepromBlockExists(blockId,&eepromAddr,NULL);
	if(res!=0) return res;

	for(u8 i=0;i<EEPROM_BLOCK_SIZE;i++){
		*blockPtr=ReadEeprom(eepromAddr++);
		blockPtr++;	
	}
	
	return EEPROM_OK;
}

/*
 * Scan is the specified EEPROM if block id exists. 
 * @param eepromAddr Set with it adress in EEPROM memory if block exists or zero if doesn't exist
 * @param nextFreeBlockId Set with id of next unnalocated block avaliable or zero (0) if all are allocated (i.e: eeprom is full)
 * 
 * @return
 *  0x00 = Success, Block exists.
 * 	0x01 = EEPROM_ERROR_INVALID_BLOCK.
 *	0x03 = EEPROM_ERROR_BLOCK_NOT_FOUND.
 */
char EepromBlockExists(unsigned int blockId, u16* eepromAddr, u8* nextFreeBlockId){
	u8 nextFreeBlock=0;
	u8 result=EEPROM_ERROR_BLOCK_NOT_FOUND;
	u16 id;
	*eepromAddr=0;

	if(blockId==EEPROM_FREE_BLOCK) return EEPROM_ERROR_INVALID_BLOCK;
		
	//scan all blocks and get the memory adress of that block and the next free block
	for(u8 i=0;i<EEPROM_MAX_BLOCKS;i++){
		id=ReadEeprom(i*EEPROM_BLOCK_SIZE)+(ReadEeprom((i*EEPROM_BLOCK_SIZE)+1)<<8);
		
		if(id==blockId){
			*eepromAddr=(i*EEPROM_BLOCK_SIZE);
			result=EEPROM_OK;
		}
		
		if(id==0xffff && nextFreeBlock==0){
			nextFreeBlock=i;
			if(nextFreeBlockId!=NULL) *nextFreeBlockId=nextFreeBlock;					
		}
	}

	return result;
}

unsigned char ReadEeprom(unsigned int addr) {
	//TODO - implement
	return 0;
}

void WriteEeprom(unsigned int addr, unsigned char value) {
	//TODO - implement 
}

#endif /* USE_EEPROM==1*/

/**
 * Generate a random number based on a LFSR. This function is *much* faster than avr-libc rand();
 * taps: 16 14 13 11; feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1
 *
 * Input: Zero=return the next random value. Non-zero=Sets the seed value.
 */
u16 GetPrngNumber(u16 seed){
	static u16 prng_state;
  	
	if(seed!=0) prng_state=seed;
	
	u16 bit  = ((prng_state >> 0) ^ (prng_state >> 2) ^ (prng_state >> 3) ^ (prng_state >> 5) ) & 1;
	prng_state =  (prng_state >> 1) | (bit << 15);
	return prng_state;   
}


#if UART == 1
	/*
	 * UART RX/TX buffer functions
	 */

	volatile u8 uart_rx_tail;
	volatile u8 uart_rx_head;
	volatile u8 uart_rx_buf[UART_RX_BUFFER_SIZE];

	//obsolete
	void UartGoBack(u8 count){
		uart_rx_tail-=count;
		uart_rx_tail&=(UART_RX_BUFFER_SIZE-1);		//wrap pointer to buffer size
	}

	//obsolete
	u8 UartUnreadCount(){
		return uart_rx_head-uart_rx_tail;
	}

	bool IsUartRxBufferEmpty(){
		return (uart_rx_tail==uart_rx_head);
	}

	s16 UartReadChar(){

		if(uart_rx_head != uart_rx_tail){

			u8 data=uart_rx_buf[uart_rx_tail];
			uart_rx_tail=((uart_rx_tail+1) & (UART_RX_BUFFER_SIZE-1));	//wrap pointer to buffer size			
			return (data&0xff);

		}else{
			return -1;	//no data in buffer
		}
	}

	void InitUartRxBuffer(){
		uart_rx_tail=0;
		uart_rx_head=0;
	}

	/*
	 * UART Transmit buffer function
	 */
	volatile u8 uart_tx_tail;
	volatile u8 uart_tx_head;
	volatile u8 uart_tx_buf[UART_TX_BUFFER_SIZE];


	bool IsUartTxBufferEmpty(){
		return (uart_tx_tail==uart_tx_head);
	}

	bool IsUartTxBufferFull(){
		u8 next_head = ((uart_tx_head + 1) & (UART_TX_BUFFER_SIZE-1));
		return (next_head == uart_tx_tail);
	}

	s8 UartSendChar(u8 data){

 		u8 next_head = ((uart_tx_head + 1) & (UART_TX_BUFFER_SIZE-1));

		if (next_head != uart_tx_tail) {
			uart_tx_buf[uart_tx_head]=data;
			uart_tx_head=next_head;		
			return 0;
		}else{
			return -1; //buffer full
		}
	}

	void InitUartTxBuffer(){
		uart_tx_tail=0;
		uart_tx_head=0;
	}


#endif


