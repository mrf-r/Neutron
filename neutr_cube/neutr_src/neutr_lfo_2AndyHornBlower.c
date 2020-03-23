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
 * neutr_lfo_2AndyHornBlower.c
 *
 *  Created on: May 6, 2019
 */

//this guy brought life to whole project. huge thanks!

/*
[QUOTE=AndyHornBlower;13965169]Would it be possible to make one or both LFO outputs be a second assignable CC output, instead of an LFO?
I don't know if they're separate outputs of the MCU, or if the unipolar one is just the main one via a diode.  Even just one more CC->CV output would be good.
Maybe the Rate knob could be an attenuator for it (multiplied in software).
If it would have to be from a limited number of choices, due to the user interface restrictions for setting it,
I vote for one of them to be CC#2 (Breath).  Some people would probably want CC#11 (Expression).  Velocity would be a good option too.[/QUOTE]
 */
#include "neutr_lfo_common.h"
#include "neutron.h"
#include "menu.h"
/*
volatile uint16_t nl2ahb_breath = 0;
volatile uint16_t nl2ahb_pitch;
*/
extern volatile uint8_t va_last_noteon_velo;
extern volatile uint16_t pp_modwheel;
extern volatile uint8_t pp_aftertouch;

void nl2ahb_leds(uint32_t frame)
{
	uint8_t pos = frame & 0x1F;
	uint8_t typesel = (lfoshape * 5)>>12;//(adc_val[POT_LFOSHAPE] * 5)>>12;
	if (pos == 0) //
	{
		led_set(LED_LFO1SINE | LED_LFO2TRIANGLE | LED_LFO3RAMPDWN | LED_LFO4SQUARE | LED_LFO5RAMPUP, 0);
		led_set(LED_LFO1SINE<<typesel, 1);
	}
	uint32_t output = 0;
	switch (typesel)
	{
	case 0:
		output = lforam.nl2ahb_pitch;
		//osc 1 note
		break;
	case 1:
		output = lforam.nl2ahb_breath<<2;
		//breath
		break;
	case 2:
		output = va_last_noteon_velo<<9;
		//velocity
		break;
	case 3:
		output = pp_modwheel<<2;
		//modwheel
		break;
	case 4:
		//aftertouch
		output = pp_aftertouch<<9;

		break;
	}
	output = (output * lforate)>>12;
	LFO_OUTPUT = output;
}

void nl2ahb_init(uint8_t prev)
{
	lforam.nl2ahb_breath = 0;
	lforam.nl2ahb_pitch = 0;
	NVIC_DisableIRQ(EXTI2_3_IRQn);
	(void)prev;
}

void nl2ahb_sr()
{
	;
}

const neutr_lfo_t nl_2andy =
{
		0, //void (*retrig)(void);
		//0,  //void (*midistart)(void);
		//0,  //void (*midiclock)(void);
		0,           //void (*cr)(void);
		nl2ahb_sr,     //void (*sr)(void);
		nl2ahb_leds,   //void (*lfoleds)(uint32_t time);
		0,           //void (*noteon)(uint8_t note, uint32_t ts);
		0,           //void (*noteoff)(uint8_t note, uint32_t ts);
		0,           //void (*damper)(uint8_t val, uint32_t ts);
		0,           //void (*panic)(void);
		//0,           //void (*irqsync)(uint8_t type);
		0,            //void (*asyncmain)(void);
		nl2ahb_init             //void (*initialize)(uint8_t prevmode);
};
