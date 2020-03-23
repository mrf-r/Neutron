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
 * sr.c
 *
 *  Created on: 30 но€б. 2018 г.
 */


#include "neutron.h"


//because of analog modulations, adc reads from 0 to about 0xF10
//we compensate and 1st order IIR it at the same time
//compensation is one for all channels.

#define ADC_FILTER_CUT (1<<ADC_FILTER_E)
const uint16_t ADC_FILTER_N = (0x1000 * ADC_FILTER_CUT / POT_MAX_VALUE);
const uint16_t ADC_FILTER_R = (0x8000 - ADC_FILTER_CUT);
const uint32_t ADC_SCALER = (0xFFF0000 / POT_MAX_VALUE);

volatile uint16_t adc_input[POT_TOTAL];
volatile uint16_t adc_val[POT_TOTAL];
volatile uint16_t dac_output[CV_TOTAL] = {0,0,0,0,0,0,0,0};

uint32_t timestamp; //DO NOT MODIFY THIS!!!
void nl0b_na()
{
	;
}
const neutr_lfo_t nl0_boot =
{
		0, //void (*retrig)(void);
		//0, //void (*midistart)(void);
		//0, //void (*midiclock)(void);
		0, //void (*cr)(void);
		nl0b_na, //void (*sr)(void);
		0, //void (*lfoleds)(uint32_t time);
		0, //void (*noteon)(uint8_t note, uint32_t ts);
		0, //void (*noteoff)(uint8_t note, uint32_t ts);
		0, //void (*damper)(uint8_t val, uint32_t ts);
		0, //void (*panic)(void);
		//0, //void (*irqsync)(uint8_t type);
		0, //void (*asyncmain)(void);
		0  //void (*initialize)(uint8_t prevmode);
};

volatile neutr_lfo_t* neutron_lfo = &nl0_boot;

volatile uint16_t lfoshape;
volatile uint16_t lforate;

#include "menu.h"
extern volatile knob_status_t knob_status[KNOB_TOTAL];

void TIM6_DAC_IRQHandler() //our main SR()
{
	uint32_t t2start = TIM2->CNT;
	global_debug(DEB_SR_START);

	//disable DAC
	DAC_PORT = DAC_DISABLE;
	//switch DAC direction
	timestamp++;
	DACDIR_PORT = dacdir[timestamp&0x7]; //cross talk shape2->pitch1 eliminated!
	DAC1->DHR12L1 = dac_output[timestamp&0x7]; //no dithering @3kHz
/*
	//osc1 shape
	adc_val[POT_OSC1SHAPE] = (adc_val[POT_OSC1SHAPE]*ADC_FILTER_R + adc_input[POT_OSC1SHAPE] * ADC_FILTER_N)>>15;
	OSCWAVEPORT = o1w[adc_val[POT_OSC1SHAPE]>>10]; //this must be made only when dac output active?
	dac_output[CV_OSC1WAVE] = ~sine[(adc_val[POT_OSC1SHAPE]>>3)&0xFF];

	//osc2 shape
	adc_val[POT_OSC2SHAPE] = (adc_val[POT_OSC2SHAPE]*ADC_FILTER_R + adc_input[POT_OSC2SHAPE] * ADC_FILTER_N)>>15;
	OSCWAVEPORT = o2w[adc_val[POT_OSC2SHAPE]>>10]; //this must be made only when dac output active?
	dac_output[CV_OSC2WAVE] = ~sine[(adc_val[POT_OSC2SHAPE]>>3)&0xFF];
*/

	uint32_t p_o1shp = (adc_input[POT_OSC1SHAPE] * ADC_SCALER) >> 19;
	OSCWAVEPORT = o1w[p_o1shp>>7]; //this must be made only when dac output active?
	dac_output[CV_OSC1WAVE] = ~sine[(p_o1shp)&0xFF];

	uint32_t p_o2shp = (adc_input[POT_OSC2SHAPE] * ADC_SCALER) >> 19;
	OSCWAVEPORT = o2w[p_o2shp>>7]; //this must be made only when dac output active?
	dac_output[CV_OSC2WAVE] = ~sine[(p_o2shp)&0xFF];



	//lfo shape
	//adc_val[POT_LFOSHAPE] = (adc_val[POT_LFOSHAPE]*ADC_FILTER_R + adc_input[POT_LFOSHAPE] * ADC_FILTER_N)>>15;
	adc_val[POT_LFOSHAPE] = (adc_input[POT_LFOSHAPE] * ADC_SCALER)>>16;
	if (adc_val[POT_LFOSHAPE] > 0xFFF) //THIS MUST BE LIMITED!!!!!!!
		adc_val[POT_LFOSHAPE] = 0xFFF;
	if (knob_status[KNOB_LFOSHAPE].cbk == knob_na)
		lfoshape = adc_val[POT_LFOSHAPE];

	//lfo rate
	//adc_val[POT_LFORATE] = (adc_val[POT_LFORATE]*ADC_FILTER_R + adc_input[POT_LFORATE] * ADC_FILTER_N)>>15;
	adc_val[POT_LFORATE] = (adc_input[POT_LFORATE] * ADC_SCALER)>>16;
	if (knob_status[KNOB_LFORATE].cbk == knob_na)
		lforate = adc_val[POT_LFORATE];

	//glide
/*
	adc_val[POT_GLIDE] = (adc_val[POT_GLIDE]*ADC_FILTER_R + adc_input[POT_GLIDE] * ADC_FILTER_N)>>15;
	if (adc_val[POT_GLIDE] > 0xFFF)
		adc_val[POT_GLIDE] = 0xFFF;
*/

	adc_val[POT_GLIDE] = adc_input[POT_GLIDE];

	ADC1->CR |= ADC_CR_ADSTART; //restart ADC
	DAC_PORT = DAC_ENABLE;

	global_debug(DEB_SR_HWESS);
	//calculate LFO
	neutron_lfo->sr(); //must be implemented!


	//we're still in interrupt, let's get out
	TIM6->SR &= ~TIM_SR_UIF; //clear

	global_debug(DEB_SR_END);


	t2start = TIM2->CNT - t2start;
	if (t2start > debug_sr_maxtime)
		debug_sr_maxtime = t2start;

}



