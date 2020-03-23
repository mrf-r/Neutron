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
 * neutron_lfo.c
 *
 *  Created on: 13 марта 2019 г.
 */


#include "neutr_lfo_common.h"
#include "neutron.h"
#include "menu.h"

//old code. for history and shit-controlling
//don't

typedef enum
{
	WAVE_SINE = 0,
	WAVE_TRI,
	WAVE_SAWUP,
	WAVE_SQUARE,
	WAVE_SAWDOWN,
	WAVE_TOTAL
}wavesel_t;

/*
uint32_t nl1c_lfoacc;

volatile uint8_t nl1c_irqstate = 0;
volatile uint16_t nl1c_irqpos;


//subpixel rendering - i don't know how good it is
//0x3E8 resolution rec = 0x418937

volatile uint8_t nl1c_syncstate = 0;
volatile uint32_t nl1c_inc;
volatile uint16_t nl1c_shift;
volatile uint16_t nl1c_recp;

//@TODO: put all this code in lfo classic!
volatile uint16_t lfo_shape_min; //@TODO: you need to be in sync with other lfo modes !
volatile uint16_t lfo_shape_max; //also you need to sync with MAXLED
*/



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void nl1c_sr()
{

	/*
	if (lfoshape > lfo_shape_max)
	{
		lfo_shape_max = lfoshape;
	}
	if (lfoshape < lfo_shape_min)
	{
		lfo_shape_min = lfoshape;
	}
	*/
	lforam.shape_max = lfoshape;	//lfo_shape_max = lfoshape;
	lforam.shape_min = lfoshape;	//lfo_shape_min = lfoshape;
	//handle midi sync also

	uint16_t tablepos = lforate>>4; //adc_val[POT_LFORATE]>>4; //inlet_pitch>>19;
	uint8_t subpos = (uint8_t)(lforate<<4); //(inlet_pitch>>11)&0xFF;

	uint32_t inc = (uint32_t)((lfoinc[tablepos+1].inc - lfoinc[tablepos].inc)>>8)*subpos+lfoinc[tablepos].inc;
	uint16_t shift =  lfoinc[tablepos].shift;
	uint8_t is = lfoinc[tablepos+1].shift-lfoinc[tablepos].shift;
	uint16_t ni = lfoinc[tablepos+1].recp>>is;
	uint16_t recp = lfoinc[tablepos].recp - ((uint32_t)(lfoinc[tablepos].recp - ni)*subpos>>8);

	//this is main output reg
	uint32_t woutput[WAVE_TOTAL];

	//increment
	lforam.phaseacc += inc;
	if (lforam.irqstate == 1)
	{
		if (parameters.lfoextsynctype != 1)
		{
			//normalize position
			uint32_t pos = (lforam.irqpos * 0x20C49B)>>15;
			//pos = ((inc>>16)*pos) + (parameters.lfosyncstart<<16);
			pos = (((inc>>shift)*pos)>>(16-shift)) + (parameters.lfosyncstart);

			lforam.phaseacc = pos;
			lforam.irqstate = 0;
		}
	}
	else
	{
		if (lforam.irqstate == 2)
		{
			if (parameters.lfoextsynctype)
			{
				//normalize position
				uint32_t pos = (lforam.irqpos * 0x20C49B)>>15; /// OH FUCK THOSE SIGNED CONSTANTS AND ARITHMETIC SHIFTS!!!
				//pos = ((inc>>16)*pos) + (parameters.lfosyncstart<<16) + 0x80000000;
				pos = (((inc>>shift)*pos)>>(16-shift)) + (parameters.lfosyncstart) + 0x80000000;

				lforam.phaseacc = pos;
				lforam.irqstate = 0;
			}
		}
	}

	//easy access phase position
	uint32_t pp = lforam.phaseacc + lforam.phaseshift;


	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	//calculate 1-sine
	{
		uint32_t sp = pp;
		uint8_t tpos = sp>>24;
		uint8_t spos = (uint8_t)(sp>>16);
		int16_t delta = sine[tpos + 1] - sine[tpos];
		woutput[WAVE_SINE] = (sine[tpos] + ((delta * spos)>>8))<<16;
	}

	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	//calculate 2-tri
	uint32_t trir;
	if(pp>>31)
		trir = ~pp;
	else
		trir = pp;
	trir = trir<<1;

	if (trir>(~inc))
	{
		//limit high
		uint32_t delta = (trir + inc)>>shift;
		uint32_t rev = (delta * recp)>>13;
		rev = (rev*rev)>>17;
		trir = trir - (rev * (inc>>16));
	}
	else
	{
		if (trir<inc)
		{
			//limit low
			uint32_t delta = (inc - trir)>>shift;
			uint32_t rev = (delta * recp)>>13;
			rev = (rev*rev)>>17;
			trir = trir + (rev * (inc>>16));
		}
	}
	woutput[WAVE_TRI] = trir;

	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	//calculate 5-sawup
	uint32_t saw1r = pp;
	if (saw1r>(~inc))
	{
		//limit high
		uint32_t delta = (saw1r + inc)>>shift;
		uint32_t rev = (delta * recp)>>13;
		rev = rev*rev;
		saw1r -= rev;
	}
	else
	{
		if (saw1r<inc)
		{
			//limit low
			uint32_t delta = (inc - saw1r)>>shift;
			uint32_t rev = (delta * recp)>>13;
			rev = rev*rev;
			saw1r += rev;
		}
	}
	woutput[WAVE_SAWUP] = saw1r;

	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	//calculate 4-sqr pt1
	uint32_t saw2r = pp + 0x80000000;
	if (saw2r>(~inc))
	{
		//limit high
		uint32_t delta = (saw2r + inc)>>shift;
		uint32_t rev = (delta * recp)>>13;
		rev = rev*rev;
		saw2r -= rev;
	}
	else
	{
		if (saw2r<inc)
		{
			//limit low
			uint32_t delta = (inc - saw2r)>>shift;
			uint32_t rev = (delta * recp)>>13;
			rev = rev*rev;
			saw2r += rev;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	//calculate 3-sawdown
	woutput[WAVE_SAWDOWN] = -saw2r;

	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	//calculate 4-sqr pt2
	saw1r = (saw1r>>16) * 0xFFFF + 1;
	saw2r = (saw2r>>16) * 0xFFFF + 1;
	woutput[WAVE_SQUARE] = saw1r - saw2r + 0x80000000;

	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	//mix lfo
	{
		uint8_t wsel = lfoshape>>10;
		uint16_t wpos = sine[(lfoshape>>3)&0x7F]>>1;
		uint16_t nwpos = 0x8000 - wpos;

		uint16_t wout1 = (((woutput[wsel]>>16) * nwpos)>>15) + wpos; //remember about zero level
		uint16_t wout2 = (((woutput[wsel+1]>>16) * wpos)>>15) + nwpos;
		LFO_OUTPUT = ~(wout1+wout2 - 0x8000);
	}
	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
}

void nl1c_retrig(void)
{
	lforam.phaseacc = parameters.lfosyncstart;
}


void nl1c_leds(uint32_t frame)
{
	//@TODO: reduce framerate by 256, add pwm for brightness control
	//calc lfo leds
	uint8_t pos = frame & 0x1F;
	uint8_t i;
	if (pos == 0) //
	{
		uint8_t lfo_shape[5] = {0,0,0,0,0};

		for (i = lforam.shape_min>>10; i < lforam.shape_max>>10; i++)
		{
			lfo_shape[i+1] = 1;
		}
		lfo_shape[lforam.shape_min>>10] = (0x3FF - (lforam.shape_min & 0x3FF))>>2;
		lfo_shape[(lforam.shape_max>>10) + 1] = (lforam.shape_max & 0x3FF)>>2;

		//set lfo leds
		for(i = 0; i < 5; i++)
		{
			lforam.ledvals[i] = lfo_shape[i]>>3;
		}

		//reset watch
		lforam.shape_max = 0x0;
		lforam.shape_min = 0xFFFF;

		if (lforam.sync_blink)
		{
			lforam.sync_blink = 0;
			//@TODO: look at sync slave timer???? make blink
		}
	}
	else // pwm leds
	{
		for (i = 0; i<5; i++)
		{
			if (lforam.ledvals[i]>pos)
			{
				led_set(LED_LFO1SINE<<i, 1);
			}
			else
			{
				led_set(LED_LFO1SINE<<i, 0);
			}
		}
	}
}

void nl_init_classic(uint8_t prevmode);

const neutr_lfo_t nl_1classic =
{
		nl1c_retrig, //void (*retrig)(void);
		//0,  //void (*midistart)(void);
		//0,  //void (*midiclock)(void);
		0,           //void (*cr)(void);
		nl1c_sr,     //void (*sr)(void);
		nl1c_leds,   //void (*lfoleds)(uint32_t time);
		0,           //void (*noteon)(uint8_t note, uint32_t ts);
		0,           //void (*noteoff)(uint8_t note, uint32_t ts);
		0,           //void (*damper)(uint8_t val, uint32_t ts);
		0,           //void (*panic)(void);
		//nl1c_irq,           //void (*irqsync)(uint8_t type);
		0,            //void (*asyncmain)(void);
		nl_init_classic             //void (*initialize)(uint8_t prevmode);
};
