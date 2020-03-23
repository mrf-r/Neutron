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
 * neutr_lfo_common.c
 *
 *  Created on: Aug 6, 2019
 */
#include "neutron.h"
#include "menu.h"
#include "neutr_lfo_common.h"

volatile lforam_t lforam;


volatile uint32_t exti_timcomp = 0x1A0; //0x226;
//260 - one sample before, 250 - one sample after
//1F0

void exti_comp_debug(uint16_t val)
{
	//exti_timcomp = 0x240 - (val>>5); //val>>2; disable write possibility
	led_set(LED_OSC2_32, exti_timcomp& 0x1);
	led_set(LED_OSC1_32, (exti_timcomp >>1) & 0x1);
	led_set(LED_OSC2_16, (exti_timcomp >>2) & 0x1);
	led_set(LED_OSC1_16, (exti_timcomp >>3) & 0x1);
	led_set(LED_OSC2_8,  (exti_timcomp >>4) & 0x1);
	led_set(LED_OSC1_8,  (exti_timcomp >>5) & 0x1);
	(void)val;
}



void EXTI2_3_IRQHandler()
{
	int32_t tpos = TIM6->CNT;
	int32_t t2pos = TIM2->CNT;
	
	tpos = exti_timcomp - tpos; //0x3E8 - tpos - exti_timcomp;
	if (tpos < 0)
	{
		tpos += 0x3E8;
	}
	uint8_t state = GPIOB->IDR & GPIO_IDR_2;

	//we are in highest interrupt, don't sleep!
	lforam.ztpos = lforam.irqpos;
	lforam.irqstate = (state>>2) + 1;
	lforam.irqpos = tpos;

	

	//save period
	uint32_t actual_period = t2pos - lforam.zt2p;
	lforam.zt2p = t2pos;
	lforam.t2lp = actual_period;
	
	/*
	if (neutron_lfo->irqsync)
	{
		neutron_lfo->irqsync(state>>2, tpos);
	}
	 */
	/*
	if (tunemode)
	{
		if (exttune)
			exttune(state);
	}
	else
	{

		if (neutron_lfo->irqsync)
			neutron_lfo->irqsync(state>>2);
	}
	*/
	EXTI->PR |= EXTI_PR_PIF2;

	NVIC_DisableIRQ(EXTI2_3_IRQn); //TODO: dangerous extsync

	global_debug(DEB_EI_BANG);
}

typedef enum
{
	LFO_MODE_CLASSIC = 0,   //0
	LFO_MODE_AHB,           //1
	LFO_MODE_MONOOSC,       //2
	LFO_MODE_POLY5,         //3
	LFO_MODE_MATHMUL,       //4
	LFO_MODE_1OLD,          //5
	LFO_MODE_COMB1          //6
}lfomodes_t;

void nl_init_classic(uint8_t prevmode)
{
	if ((prevmode != LFO_MODE_CLASSIC)||(prevmode != LFO_MODE_MONOOSC)||(prevmode != LFO_MODE_1OLD))
	{
		//
		lforam.fullcomp = exti_timcomp << JC_FULLCOMP_SHIFT;
		lforam.phaseshift = 0;
		lforam.irqstate = 0;
		lforam.midisyncstate = 0;
		lforam.mclk_lastvaliddelta = 1000;
		lforam.mclk_ztimestamp = 0;
		lforam.mclk_delta = 0;
		NVIC_EnableIRQ(EXTI2_3_IRQn);
	}
}

void nl_init_disableexti(uint8_t prevmode)
{
	NVIC_DisableIRQ(EXTI2_3_IRQn);
	(void)prevmode;
}
extern volatile uint32_t timestamp;

void midistart()
{
	lforam.mclk_startreq = 1;
}
extern volatile uint32_t timestamp; //DO NOT MODIFY THIS!!!
void midiclock()
{
	//this must be fast, no processing here
	uint32_t delta = timestamp - lforam.mclk_ztimestamp;
	if (delta < 6000)
	{
		lforam.mclk_delta = delta;
		lforam.mclk_phase = lforam.phaseacc;
	}
	lforam.mclk_ztimestamp = timestamp;
}

void nlcom_cr()
{
	uint32_t delta = timestamp - lforam.mclk_ztimestamp;
	if (delta> 6000)
	{
		lforam.midisyncstate = 1;
	}
}

void nlcom_async()
{
	//this processing contains a LOT of divisions - TWO(!), so it only can be asynchronous
	//independend output of inc/shift/recp values and inc compensation values
	if(lforam.mclk_delta)
	{
		uint16_t dpos = lforate>0xFFF?0xFFF:lforate;
		uint32_t divr = syncdivr[31 - (dpos>>7)].divr;
		uint32_t maxmod = syncdivr[31 - (dpos>>7)].maxmod;

		int32_t delta = lforam.mclk_delta;
		if (delta < 600)
			delta = 600; //200bpm
		if (delta > 3000)
			delta = 3000; //40bpm

		//initial value - max speed
		lforam.mclk_lastvaliddelta = delta;

		uint32_t inc = 0xFFFFFFFF / (delta * divr);

		//now inc is ready, but we need to compensate drift

		//drift compensation

		uint32_t capturedphase = (lforam.mclk_phase - maxmod)>>1;
		uint32_t phasemod = (capturedphase>>15) * divr;
		phasemod = (phasemod>>16) * maxmod;
		capturedphase = capturedphase - phasemod; //this is modulo
		int32_t rsphase = (maxmod>>1) - capturedphase;
		rsphase = rsphase / ((int32_t)(delta * 32));

		//add compensation
		inc = inc + rsphase;
		//overall it is less than 1% affection, bet keep in mind;

		lforam.ms_inc = inc;
		lforam.ms_shift = 0;
		lforam.ms_recp = 0; //safety
		lforam.mclk_delta = 0;
	}
	else
	{
		if (lforam.midisyncstate)
		{
			lforam.midisyncstate = 0;
			uint16_t dpos = lforate>0xFFF?0xFFF:lforate;
			uint32_t divr = syncdivr[31 - (dpos>>7)].divr;
			uint32_t delta = lforam.mclk_lastvaliddelta;
			uint32_t inc = 0xFFFFFFFF / (delta * divr);
			lforam.ms_inc = inc;
			lforam.ms_shift = 0;
			lforam.ms_recp = 0; //safety
			lforam.mclk_delta = 0;
		}
	}

}
