/*

Copyright (C) 2019, 2020 Evgeny "mrf" Chernykh
This file is part of neutr_cube project.

neutr_cube is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

neutr_cube is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
neutr_cube. If not, see <http://www.gnu.org/licenses/>.

 *
 * hal_init.c
 *
 *  Created on: Dec 30, 2018
 */
#include "neutron.h"
#include "menu.h"
#include "neutr_lfo_common.h"

extern uint32_t ser_input;
extern uint32_t ser_output;
extern uint16_t adc_input[POT_TOTAL];

//extern uint16_t o1freq[];
//extern uint16_t o2freq[];
extern uint16_t ft_freq[];





////////////////////////////////////////////////////////////
/*
 * 10 cr start
 * 1F cr end
 *
 * 20 sr start
 * 2F sr end
 *
 */

volatile uint32_t debug_sr_maxtime = 0;
volatile uint32_t debug_cr_maxtime = 0;
volatile uint32_t hal_debug[64]; //TODO delete this debug
volatile uint8_t hal_deb_pos;
void global_debug(uint16_t source)
{
	hal_debug[hal_deb_pos] = TIM7->CNT | (source<<16);
	hal_deb_pos = (hal_deb_pos +1)&63;
}
////////////////////////////////////////////////////////////






void peripherial_init()
{
	// ********************************************************************************************************
	// ********************************************************************************************************
	// ********************************************************************************************************
	// INITIALIZATION

	RCC->AHBENR |= 0x000E0001; //enable GPIOA,B,C, DMA
	RCC->APB1ENR |= (RCC_APB1ENR_TIM6EN | RCC_APB1ENR_TIM7EN | RCC_APB1ENR_SPI2EN | RCC_APB1ENR_DACEN | RCC_APB1ENR_USBEN | RCC_APB1ENR_TIM2EN);
	RCC->APB2ENR |= (RCC_APB2ENR_ADCEN | RCC_APB2ENR_SPI1EN | RCC_APB2ENR_TIM1EN | RCC_APB2ENR_USART1EN | RCC_APB2ENR_TIM17EN);
	RCC->CR2 |= RCC_CR2_HSI14ON; //ADC CLK
	while ((RCC->CR2 & RCC_CR2_HSI14RDY) == 0)
	{ //wait
	}

	// ********************************************************************************************************
	// ********************************************************************************************************
	// ********************************************************************************************************
	// GPIO

	GPIOA->MODER |= 0x402A4FFF; // @TODO: replace |= to = for non-debug
	//GPIOA->AFR[0] = 0x00000000; //
	GPIOA->AFR[1] = 0x00000112; //tim1ch1 + usart1
	GPIOB->MODER = 0x95695685;
	//GPIOB->AFR[0] = 0x00000000;
	GPIOB->AFR[1] = 0x00000520; //tim17ch1 + spi1 + spi2
	GPIOC->MODER = 0x54000000;

	// ********************************************************************************************************
	// ********************************************************************************************************
	// ********************************************************************************************************
	// SAMPLERATE TIMER, DAC

	//dac ch 1 - 8cv; ch2 - lfo
	DAC1->CR = DAC_CR_TEN2 | DAC_CR_EN1 | DAC_CR_EN2 | DAC_CR_BOFF1 | DAC_CR_BOFF2;

	TIM6->CR2 |= TIM_CR2_MMS_1; //update to trig output
	TIM6->DIER |= TIM_DIER_UIE;
	//TIM6->PSC = 0xFF;          //256
	//TIM6->CR1 |= TIM_CR1_URS;
	//TIM6->ARR = 0x3;           //48000 Hz
	TIM6->ARR = 0x3E7;           //div 1000 with zero psc

	TIM7->DIER |= TIM_DIER_UIE;
	TIM7->ARR = 0x3E7F;
	//TIM7->EGR = TIM_EGR_TG;


	// ********************************************************************************************************
	// ********************************************************************************************************
	// ********************************************************************************************************
	// ADC - complete

	ADC1->CR |= ADC_CR_ADCAL; //start calibration
	while ((ADC1->CR & ADC_CR_ADCAL) != 0)
	{
		;
	}

	ADC1->CR |= ADC_CR_ADEN;
	while(!(ADC1->ISR & ADC_ISR_ADRDY))    //wait till ready
	{
		;
	}


	ADC1->CFGR1 |= ADC_CFGR1_DMAEN | ADC_CFGR1_DMACFG; //set mode
	ADC1->SMPR |= ADC_SMPR_SMP_1 | ADC_SMPR1_SMPR_0; //13.5 cycles - must be ok.
	ADC1->CHSELR = ADC_CHSELR_CHSEL0 | ADC_CHSELR_CHSEL1 | ADC_CHSELR_CHSEL2 | ADC_CHSELR_CHSEL3 | ADC_CHSELR_CHSEL6;

	//ADC1->CHSELR = ADC_CHSELR_CHSEL1;
	//sampling time - 11718.75Hz 10.666666 uS 137cycles
	//93750Hz = 10.6uS for 6 channels = 1.7uS per channel
	//adcclk = 14MHz = 71.428 nS
	//for 1.7uSpch sampling time must be <11cycles
	//130cycles for 5 channels - 10.66uS
	//so 14MHz clock must be at least 12187500 Hz - it's ok (-4..+5% @ t=-40..+105)




	DMA1_Channel1->CCR = DMA_CCR_CIRC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_MINC | DMA_CCR_PL; //also highest prio
	DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
	DMA1_Channel1->CMAR = (uint32_t)&adc_input;
	DMA1_Channel1->CNDTR = 5;

	DMA1_Channel1->CCR |= DMA_CCR_EN; //start!
	ADC1->CR |= ADC_CR_ADSTART; //bang!


	// ********************************************************************************************************
	// ********************************************************************************************************
	// ********************************************************************************************************
	// 165 AND 595 SERIAL IO

	//NVIC_EnableIRQ(ADC1_COMP_IRQn);
	//NVIC_SetPriority(ADC1_COMP_IRQn, 0);
	//ADC1->CR |= ADC_CR_ADSTART; /* start the ADC conversions */


	//spi1 - RX 16, DMA CH2-RX, CH3-TX(@TODO:check if need)
	SPI1->CR1 = SPI_CR1_BR_0 | SPI_CR1_BR_2 | SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI; //div16 - 3MHz
	SPI1->CR2 = SPI_CR2_DS; //rx dma, 16bit         // SPI_CR2_RXDMAEN
	SPI1->CR1 |= SPI_CR1_SPE;
    //
	// DMA1_Channel2->CCR = DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0; //
	// DMA1_Channel2->CNDTR = 1;
	// DMA1_Channel2->CPAR = &SPI1->DR;
	// DMA1_Channel2->CMAR = &ser_input;

	//When the SPI is used only to receive data, it is possible to enable only the SPI Rx DMA channel.
	// DMA1_Channel3->CCR = DMA_CCR_MINC | DMA_CCR_DIR;
	// DMA1_Channel3->CNDTR = 2;
	// DMA1_Channel3->CPAR = &SPI1->DR;
	// DMA1_Channel3->CMAR = &ser_input;

	//DMA1_Channel2->CCR |= DMA_CCR_EN; //start me!!
	//DMA1_Channel3->CCR |= DMA_CCR_EN; //then me??

	//spi2 - TX 24, DMA CH5-TX
	SPI2->CR1 = SPI_CR1_BR_0 | SPI_CR1_BR_2 | SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
	SPI2->CR2 = SPI_CR2_TXDMAEN | SPI_CR2_DS; //tx dma, 16bit
	SPI2->CR1 |= SPI_CR1_SPE;

	DMA1_Channel5->CCR = DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_MINC | DMA_CCR_DIR;
	DMA1_Channel5->CPAR = (uint32_t)&SPI2->DR;
	DMA1_Channel5->CMAR = (uint32_t)&ser_output;
	DMA1_Channel5->CNDTR = 2;

	// ********************************************************************************************************
	// ********************************************************************************************************
	// ********************************************************************************************************
	// MIDI - complete

	//uart1   31250, 1536
	USART1->BRR = 0x600;
	USART1->CR1 = USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE | USART_CR1_UE; // | USART_CR1_TXEIE
	//that is all we need??
	NVIC_SetPriority(USART1_IRQn, IRQ_PRIORITY_MIDI);


	// ********************************************************************************************************
	// ********************************************************************************************************
	// ********************************************************************************************************
	// FREQUENCY COUNTER, AUTOTUNE - complete



	//tim1,17 - frequency counters
	SYSCFG->CFGR1 |= SYSCFG_CFGR1_TIM1_DMA_RMP | SYSCFG_CFGR1_TIM17_DMA_RMP2; //remap t1 to ch6 and t17 to ch7

	TIM1->CR1 = TIM_CR1_CKD_1 | TIM_CR1_URS;
	TIM1->DIER = TIM_DIER_CC1DE;
	TIM1->SMCR = TIM_SMCR_TS_2;
	TIM1->CCMR1 = TIM_CCMR1_CC1S_0;
	TIM1->CCER = TIM_CCER_CC1E; // @TODO: try with this: | TIM_CCER_CC1P; //once more
	TIM1->ARR = 0xFFFF;

	//set prescaler
	TIM1->PSC = 0x3F; //750kHz
	TIM1->EGR = TIM_EGR_UG;

	DMA1_Channel6->CCR = DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_MINC; //DMA_CCR_CIRC |
	DMA1_Channel6->CPAR = (uint32_t)&TIM1->CCR1;
	DMA1_Channel6->CMAR = (uint32_t)ft_freq;
	//DMA1_Channel6->CNDTR = 32;

	//DMA1_Channel6->CCR |= DMA_CCR_EN;

	TIM1->CR1 |= TIM_CR1_CEN; //start freq count!


	//now same with tim17 - osc2 freq counter
	TIM17->CR1 = TIM_CR1_CKD_1 | TIM_CR1_URS;
	TIM17->DIER = TIM_DIER_CC1DE;
	TIM17->SMCR = TIM_SMCR_TS_2;
	TIM17->CCMR1 = TIM_CCMR1_CC1S_0;
	TIM17->CCER = TIM_CCER_CC1E; // @TODO: try with this: | TIM_CCER_CC1P; //once more
	TIM17->ARR = 0xFFFF;

	//set prescaler
	TIM17->PSC = 0x3F; //750kHz 11.444091796875 minimal frequency
	TIM17->EGR = TIM_EGR_UG;

	DMA1_Channel7->CCR = DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_MINC; //DMA_CCR_CIRC |
	DMA1_Channel7->CPAR = (uint32_t)&TIM17->CCR1;
	DMA1_Channel7->CMAR = (uint32_t)&ft_freq;
	//DMA1_Channel7->CNDTR = 32;

	//DMA1_Channel7->CCR |= DMA_CCR_EN;

	TIM17->CR1 |= TIM_CR1_CEN; //start freq count!

	//enable 32bit timer2 for filter tuning
	TIM2->CR1 = TIM_CR1_CEN;

	//configure PB02 external interrupt
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PB;
	EXTI->IMR |= EXTI_IMR_IM2;
	EXTI->RTSR |= EXTI_RTSR_RT2;
	EXTI->FTSR |= EXTI_FTSR_FT2;
	NVIC_SetPriority(EXTI2_3_IRQn,IRQ_PRIORITY_LFOSYNC);
	NVIC_EnableIRQ(EXTI2_3_IRQn);

	//usb??

	TIM6->CR1 |= TIM_CR1_CEN; // !!!!!!!!!!!!!!!!!!!1START ME!
	TIM7->CR1 |= TIM_CR1_CEN;

	NVIC_SetPriority(TIM6_DAC_IRQn, IRQ_PRIORITY_SR);
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	NVIC_SetPriority(TIM7_IRQn, IRQ_PRIORITY_CR);
	NVIC_EnableIRQ(TIM7_IRQn);

	NVIC_EnableIRQ(USART1_IRQn);

	GPIOB->BSRR = 0x00000003; //output enable sync disable
}

//extern volatile uint32_t ser_input;
extern volatile uint8_t midi_channel_switch;


void neutr_main(void)
{
	load_parameters();
	//load default menu page and display process

	ft_delay(200);
	//exticalib();
	//ft_delay(100);
	msp_root();

	while(1)
	{
		eventscheck();

		if (neutron_lfo->asyncmain)
			neutron_lfo->asyncmain();

		uint8_t ms = (uint8_t)(ser_input>>8);
		if (0xF - ms != midi_channel_switch)
		{
			midi_channel_switch = 0xF - ms;
			va_panic();
		}
	}
}















