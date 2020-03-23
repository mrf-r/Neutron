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
 * neutr_lfo_5_mathtest.c
 *
 *  Created on: 24 мая 2019 г.
 */

//sanity test

#include "neutron.h"
#include "menu.h"
#include "neutr_lfo_common.h"

extern volatile uint16_t adc_input[POT_TOTAL];
extern volatile uint16_t dac_output[CV_TOTAL];
/*
int32_t nl5t_zout;
int32_t nl5t_zsample;
uint32_t nextsample = 0;
uint8_t nl5t_status;
uint32_t nl5t_limstate = 0;
*/
#define NL5T_HILIM 0x10
#define NL5T_LOLIM 0x01
#define NL5T_CENTER 0x04
#define NL5T_CENTERHIGH 0x8100
#define NL5T_CENTERLOW 0x7E00


#define NL5T_SAT_NONE 0
#define NL5T_SAT_TOP 1
#define NL5T_SAT_BOT 2

void nl5t_sr()
{
	int32_t arg1_unfiltered = lforate - 0x800;
	int32_t arg2_filtered = lfoshape - 0x800; //adc_val[POT_LFOSHAPE] - 0x800;

	int32_t sample = ((arg1_unfiltered * arg2_filtered)/64)+32768;
	int32_t out = sample;


	if (parameters.lfokeysync)
	{
		//naive way
		if (out>0xFFFF)
		{
			out = 0xFFFF;
			lforam.nl5t_status |= NL5T_HILIM;
		}
		else
		{
			if (out<0)
			{
				out = 0;
				lforam.nl5t_status |= NL5T_LOLIM;
			}
		}
	}
	else
	{



	}



	if(out<0)
	{
		if(lforam.nl5t_zout>0)
		{
			//key
		}
		else
		{
			out = 0;
		}
	}


	if ((out<NL5T_CENTERHIGH)&&(out>NL5T_CENTERLOW))
	{
		lforam.nl5t_status |= NL5T_CENTER;
	}
	LFO_OUTPUT = lforam.nl5t_zout;
	lforam.nl5t_zout = out;
	lforam.nl5t_zsample = sample;
}

//extern uint8_t ledvals[5];

void nl5t_leds(uint32_t frame)
{
	led_set(LED_LFO1SINE, lforam.nl5t_status & 0x1);       //clipping less than 0
	led_set(LED_LFO2TRIANGLE, lforam.nl5t_status & 0x2);   //
	led_set(LED_LFO3RAMPDWN, lforam.nl5t_status & 0x4);    //zero cross
	led_set(LED_LFO4SQUARE, lforam.nl5t_status & 0x8);     //
	led_set(LED_LFO5RAMPUP, lforam.nl5t_status & 0x10);    //clipping more than 65535
	lforam.nl5t_status = 0;
	(void)frame;
}

void nl5t_init(uint8_t prev)
{
	NVIC_DisableIRQ(EXTI2_3_IRQn);
	lforam.nl5t_nextsample = 0;
	lforam.nl5t_limstate = 0;
	(void)prev;
}
const neutr_lfo_t nl_5math =
{
		0, //void (*retrig)(void);
		//0,  //void (*midistart)(void);
		//0,  //void (*midiclock)(void);
		0,           //void (*cr)(void);
		nl5t_sr,     //void (*sr)(void);
		nl5t_leds,   //void (*lfoleds)(uint32_t time);
		0,           //void (*noteon)(uint8_t note, uint32_t ts);
		0,           //void (*noteoff)(uint8_t note, uint32_t ts);
		0,           //void (*damper)(uint8_t val, uint32_t ts);
		0,           //void (*panic)(void);
		//0,           //void (*irqsync)(uint8_t type);
		0,            //void (*asyncmain)(void);
		nl5t_init             //void (*initialize)(uint8_t prevmode);
};

