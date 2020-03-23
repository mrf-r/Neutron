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
 * knobs.c
 *
 *  Created on: Jul 14, 2019
 */

#include "neutron.h"
#include "menu.h"


extern volatile uint16_t adc_val[POT_TOTAL];

//!old non optimal version here. it works


//if you add something here, don't forget to edit knob_process()
/*
typedef struct
{
	uint16_t lock_value;
	uint8_t lock;
	uint8_t compare_val;
	void (*cbk)(uint16_t val);
}knob_status_t;

*/


knob_status_t knob_status[KNOB_TOTAL] =
		{
				{
						0,0,0,knob_na
				},
				{
						0,0,0,knob_na
				},
				{
						0,0,0,knob_na
				},
		};

void k_process0(uint8_t i, uint16_t adc) //unlocked - adc thru
{
	knob_status[i].lock_value = adc;
	knob_status[i].cbk(adc);
}
void k_process1(uint8_t i, uint16_t adc) //adc filter - exiting range of current pos
{
	int32_t delta = knob_status[i].compare_val - (adc>>4);
	if (delta<0)
		delta = -delta;
	if (delta > 0x3)
	{
		knob_status[i].lock = 0;
		knob_status[i].cbk(adc);
	}
}
void k_process2(uint8_t i, uint16_t adc) //pot functionality switch - entering range of parameter value
{
	int32_t delta = knob_status[i].lock_value - adc;
	if (delta<0)
		delta = -delta;
	if (delta < 0x48)
	{
		knob_status[i].lock = 0;
		knob_status[i].cbk(adc);
	}
}
void k_process_dummy(uint8_t i, uint16_t adc)
{
	(void)i;
	(void)adc;
}
void (*const k_process[4])(uint8_t i, uint16_t adc) =
{
	k_process0,
	k_process1,
	k_process2,
	k_process_dummy
};

inline uint16_t knob_adc_val(knobselect_en knob)
{
	switch (knob)
	{
	case 0:
		return adc_val[POT_LFORATE];
		break;
	case 1:
		return adc_val[POT_LFOSHAPE];
		break;
	case 2:
		return adc_val[POT_GLIDE];
		break;
	}
	return 0;
}

void knob_na(uint16_t val)
{
	(void)val;
}

void knob_process()
{
	k_process[knob_status[0].lock&0x3](0,adc_val[POT_LFORATE]);
	k_process[knob_status[1].lock&0x3](1,adc_val[POT_LFOSHAPE]);
	k_process[knob_status[2].lock&0x3](2,adc_val[POT_GLIDE]);
}
/*
 * type 0 - normal mode - every time calls callback
 * type 1 - wait till pot average moves
 * type 2 - wait till pot achieve val
 */
void knob_lock(knobselect_en knob, uint16_t val, uint8_t type, void (*callback)(uint16_t val))
{
	knob_status[knob].lock = type;
	knob_status[knob].lock_value = val;
	knob_status[knob].compare_val = knob_adc_val(knob)>>4;
	if (callback)
		knob_status[knob].cbk = callback;
	else
		knob_status[knob].cbk = knob_na;
}













