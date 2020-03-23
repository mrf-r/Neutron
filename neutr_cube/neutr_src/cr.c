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

 * cr.c
 *
 *  Created on: Mar 6, 2019
 */


//all leds and spi are here

#include "neutron.h"
#include "menu.h"


volatile uint32_t ser_input;
volatile uint32_t ser_output;
uint32_t leds = 0x400000;


volatile uint8_t input_state;
volatile uint8_t lastchange;
volatile uint16_t blinktime;
volatile uint32_t blinktbmp;
volatile uint8_t blinkpos;
volatile uint8_t menublink = 0;


volatile uint8_t retrigrequest = 0;


extern volatile uint32_t ft_time;

//leds blink = 2929.6875 / 512 = 5.7220458984375 Hz


void led_blink(uint32_t led_bmp, uint16_t blinks)
{
	blinkpos = ft_time & 0xFF;
	if (led_bmp)
	{
		blinktbmp = led_bmp;
		blinktime = blinks;
	}
	else
	{
		blinktbmp = 0;
		blinktime = 0;
	}

}

void led_set(uint32_t led_bmp, uint8_t val)
{
	if(val)
	{
		leds |= led_bmp;
	}
	else
	{
		leds &= ~led_bmp;
	}
}

void TIM7_IRQHandler(void) // 2929.6875 Hz
{
	uint32_t t2start = TIM2->CNT;
	global_debug(DEB_CR_START);

	ser_input = SPI1->DR;
	//spi transfers
	SPI165_PORT = SPI165_DIS;
	SPI595_PORT = SPI595_DIS;
	//do something
	uint8_t newchange;
	uint8_t currentaction;
	newchange = ((uint8_t)ser_input) ^ input_state;
	currentaction = newchange & lastchange; //
	lastchange = newchange;
	if (currentaction)
	{
		input_state = (uint8_t)ser_input;
		buttonpress(currentaction | (input_state<<8));
	}

	SPI165_PORT = SPI165_EN;
	SPI595_PORT = SPI595_EN;

	//send ser_output with dma
	DMA1_Channel5->CCR &= ~DMA_CCR_EN;
	DMA1_Channel5->CNDTR = 2;
	DMA1_Channel5->CCR |= DMA_CCR_EN;

	//start spi transfers
	SPI1->DR = ser_input;

	//calculate leds and blink

	//page LFO leds handling
	ft_time++;
	if (lfoledproc)
		lfoledproc(ft_time);

	//blink

	ser_output = ~leds;
	if (blinktime)
	{
		if ((ft_time & 0xFF) == blinkpos)
		{

			blinktime--;
		}
		if (((ft_time-blinkpos) & 0xFF) > 0x80) //first is not, and only then it starts
		{
			ser_output = ~(leds ^ blinktbmp);
		}
	}
	if (menublink)
	{
		if ((ft_time & 0x1FF) > 0x100)
		{
			ser_output |= LEDBTN_PARAPHONIC;
		}
		else
		{
			ser_output &= ~LEDBTN_PARAPHONIC;
		}
	}

	if (retrigrequest)
	{
		if (retrigrequest == 2)
		{
			retrigrequest = 1;
			GATEPORT = GATEOFF;
		}
		else
		{
			retrigrequest = 0;
			GATEPORT = GATEON;
		}
	}

	if (neutron_lfo->cr)
		neutron_lfo->cr();

	global_debug(DEB_CR_PPP);


	knob_process();
	cr_pitchprocess();



	//we're still in interrupt, let's get out
	TIM7->SR &= ~TIM_SR_UIF; //clear


	global_debug(DEB_CR_END);
	t2start = TIM2->CNT - t2start;
	if (t2start > debug_cr_maxtime)
		debug_cr_maxtime = t2start;
}
