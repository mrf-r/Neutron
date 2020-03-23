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
 * proc_pitch.c
 *
 *  Created on: 11 џэт. 2019 у.
 */

/*
 * how to calibrate Neutron under mrf firmware
 * you can calibrate:
 * 	-osc1 octave scale and offset
 * 	-osc1 midi key scale
 * 	-osc2 octave scale and offset
 * 	-osc2 key scale
 * 	-filter key scale
 * 	-assign output scale and offset (when using as osc1 or 2 cv out)
 *
 * you need:
 * 	-calibrated cv source
 * 	-tuner
 * 	-standart din-5 midi keyboard (or usb connection to host midi source)
 *
 *	0 - calibrate external cv source as You like (1.000 v/oct better)
 *	1 - turn your Neutron on and play it as usual for half a hour (warm up circuits)
 * 	2 - connect external cv source to osc mod inputs.
 * 	3 - trim potentiometers on board to get exact key scale
 * 	bear in mind, that ocs1/osc2 individual inputs scales are slightly differ from combined osc1+2 input,
 * 	it's up to You, which input you prefer or use more often.
 * 	You do not need to use midi or usb while performing this operation.
 *
 * 	4 - enter osc menu and press autotune or tune manually
 * 	there are two internal cv sources, that are summed:
 * 		first is octave switch source, that got scale and offset parameters
 * 		second is note+pitchbend+glide source, that only got scale setting
 *
 */
#include "neutron.h"
#include "menu.h"

extern volatile uint16_t dac_output[CV_TOTAL];
extern volatile uint16_t adc_val[POT_TOTAL];

#define DAC_O1PITCH dac_output[CV_OSC1COARSE]
#define DAC_O1OCTAVE dac_output[CV_OSC1FINE]

#define DAC_O2PITCH dac_output[CV_OSC2COARSE]
#define DAC_O2OCTAVE dac_output[CV_OSC2FINE]

#define DAC_FILTER dac_output[CV_FILTERCUT]
#define DAC_ASSIGN dac_output[CV_ASSIGN]

volatile uint8_t pp_aftertouch = 0;
volatile uint16_t pp_modwheel = 0;
//volatile int16_t pp_pitchwheel = 0;

extern volatile uint8_t va_last_noteon_velo;

volatile int32_t pp_o1pwshift = 0;
volatile int32_t pp_o2pwshift = 0;
volatile uint32_t pp_o1np;
volatile uint32_t pp_o2np;
volatile uint32_t pp_o1p;
volatile uint32_t pp_o2p;

volatile uint8_t pp_tunemode = 0;

void pp_setdac(uint8_t mode)
{
	uint32_t note = (((uint32_t)0x00001337 * adc_val[POT_LFOSHAPE])>>21) * 0x1800; //TODO: range is too high
	pp_tunemode = mode;
	int32_t out;
	switch (mode)
	{
	case TUNEM_O1OCT:
		out = (note * parameters.o1tuneoctcf)>>15;
		DAC_O1PITCH = (((uint32_t)0x7200 * parameters.o1tunecf)>>15);
		DAC_O1OCTAVE = out;
		break;
	case TUNEM_O1P:
		out = ((note * parameters.o1tunecf)>>15);
		DAC_O1OCTAVE = ((((uint32_t)0x3000 * parameters.o1tuneoctcf)>>15) + parameters.o1tuneoff);
		DAC_O1PITCH = out;
		break;
	case TUNEM_O1OFF:
		out = ((note * parameters.o1tuneoctcf)>>15) + parameters.o1tuneoff;
		DAC_O1PITCH = (((uint32_t)0x7200 * parameters.o1tunecf)>>15);
		DAC_O1OCTAVE = out;
		break;

	case TUNEM_O2OCT:
		out = (note * parameters.o2tuneoctcf)>>15;
		DAC_O2PITCH = (((uint32_t)0x7200 * parameters.o2tunecf)>>15);
		DAC_O2OCTAVE = out;
		break;
	case TUNEM_O2P:
		out = ((note * parameters.o2tunecf)>>15);
		DAC_O2OCTAVE = ((((uint32_t)0x3000 * parameters.o2tuneoctcf)>>15) + parameters.o2tuneoff);
		DAC_O2PITCH = out;
		break;
	case TUNEM_O2OFF:
		out = ((note * parameters.o2tuneoctcf)>>15) + parameters.o2tuneoff;
		DAC_O2PITCH = (((uint32_t)0x7200 * parameters.o2tunecf)>>15);
		DAC_O2OCTAVE = out;
		break;

	case TUNEM_FLT:
		out = ((note * parameters.flttunecf)>>15);
		DAC_FILTER = out;
		break;
	case TUNEM_AS:
		out = ((note * parameters.astunecf)>>15) + parameters.astuneoff;
		DAC_ASSIGN = out;
		break;
	}
}



void pp_seto1(uint8_t note, uint8_t glide)
{
	if (note > 115)
		note -= 12;
	pp_o1np = note<<24;
	if (!glide)
	{
		pp_o1p = note<<24;
	}
}
void pp_seto2(uint8_t note, uint8_t glide)
{
	if (note > 115)
		note -= 12;
	pp_o2np = note<<24;
	if (!glide)
	{
		pp_o2p = note<<24;
	}
}

void pp_pitchwheel(int16_t value)
{
	//pw center-0x0 max-0x1FFF min-0xE000
	//params 0x0 to 0xA
	//notes 0x0000 to 0xFE00
	//one note 0x200
	pp_o1pwshift = (parameters.o1pwrange * value)/16;//@TODO: check asm: ASL()
	pp_o2pwshift = (parameters.o2pwrange * value)/16;
}


#include "neutr_lfo_common.h" //extern volatile uint16_t nl2ahb_pitch;

void cr_pitchprocess(void)
{
	//3kHz rate
	int32_t o1res,o2res;

	//osc1
	int32_t dif = pp_o1np - pp_o1p;
	if (dif<0)
	{
		//negative
		int32_t difl = (-dif)>>23;
		difl = -(sqrttbl[difl]);
		int32_t glir = difl * glinc[parameters.o1glide>>4];
		if (glir < dif)
		{
			glir = dif;
		}
		pp_o1p += glir;
	}
	else
	{
		//positive
		int32_t difl = dif>>23;
		difl = sqrttbl[difl];
		int32_t glir = difl * glinc[parameters.o1glide>>4];
		if (glir > dif)
		{
			glir = dif;
		}
		pp_o1p += glir;
	}
	//add pw
	o1res = (pp_o1p>>15) + pp_o1pwshift;
	lforam.nl2ahb_pitch = o1res;
	//limit
	if (o1res > 0xE600)
		o1res -= 0x1800;
	else
		if (o1res < 0)
			o1res += 0x1800;

	//osc2
	dif = pp_o2np - pp_o2p;
	if (dif<0)
	{
		//negative
		int32_t difl = (-dif)>>23;
		difl = -(sqrttbl[difl]);
		int32_t glir = difl * glinc[parameters.o2glide>>4];
		if (glir < dif)
		{
			glir = dif;
		}
		pp_o2p += glir;
	}
	else
	{
		//positive
		int32_t difl = dif>>23;
		difl = sqrttbl[difl];
		int32_t glir = difl * glinc[parameters.o2glide>>4];
		if (glir > dif)
		{
			glir = dif;
		}
		pp_o2p += glir;
	}
	//add pw
	o2res = (pp_o2p>>15) + pp_o2pwshift;
	//limit
	if (o2res > 0xE600)
		o2res -= 0x1800;
	else
		if (o2res < 0)
			o2res += 0x1800;

	if (!pp_tunemode)
	{
		DAC_O1PITCH = (o1res * parameters.o1tunecf)>>15;
		DAC_O2PITCH = (o2res * parameters.o2tunecf)>>15;

		//assign out
		uint32_t out = ~0;
		switch (parameters.asmode)
		{
		case 0:
			//osc1
			while (o1res>0x7800)
				o1res-=0x1800;
			out = ((o1res * parameters.astunecf)>>14) + parameters.astuneoff; //@TODO: input div by 2
			break;
		case 1:
			//osc2
			while (o2res>0x7800)
				o2res-=0x1800;
			out = ((o2res * parameters.astunecf)>>14) + parameters.astuneoff;
			break;
		case 2:
			//velocity
			out = va_last_noteon_velo<<9;
			break;
		case 3:
			//modwheel
			out = pp_modwheel<<2;
			break;
		case 4:
			//aftertouch
			out = pp_aftertouch<<9;
			break;
		}
		DAC_ASSIGN = out;

		//filter
		if (parameters.filterkeytrack)
		{
			DAC_FILTER = (o1res * parameters.flttunecf)>>15;
		}
		else
		{
			DAC_FILTER = 0;
		}
	}
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//notes are 8 bits shifted left - 0x0000 to 0x7F00
//glide and pitchshift got 32 steps only

//scale can be tuned from 0.6 to 1.3
//normalized to 15 bit unsigned 32768

//keyboard
//pitch = note*cf1

//pitchwheel
//pwval = pw*param*coef

//cr
//dac_value1 = glide process + pitchwheel

//octave buttons
//dac_value2 = octave*cf2+offset
void pp_octaveo1(uint8_t octave)
{
	DAC_O1OCTAVE = (((uint32_t)octave*12 * parameters.o1tuneoctcf)>>6) + parameters.o1tuneoff;
}
void pp_octaveo2(uint8_t octave)
{
	DAC_O2OCTAVE = (((uint32_t)octave*12 * parameters.o2tuneoctcf)>>6) + parameters.o2tuneoff;
}






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
