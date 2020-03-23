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
 * menu_page_def.c
 *
 *  Created on: 22 февр. 2019 г.
 */

#include "neutron.h"
#include "menu.h"

//void msp_o1range_press(void){;}
//void msp_o2range_press(void){;}
//void msp_osync_press(void){;}
//void msp_paraph_press(void){;}
//void msp_lfosync_press(void){;}
//void msp_fltmode_press(void){;}
//void msp_fltkeytr_press(void){;}


void msp_o1range_press(void);
void msp_o2range_press(void);
void msp_osync_press(void);
void msp_paraph_press(void);
void msp_lfosync_press(void);
void msp_fltmode_press(void);
void msp_fltkeytr_press(void);

const menu_page_t mspage_root =
{
		msp_o1range_press,   //BTNEN_O1RANGE
		msp_o2range_press,   //BTNEN_O2RANGE
		msp_osync_press,     //BTNEN_OSCSYNC
		msp_paraph_press,    //BTNEN_PARAPH                -simple switch
		msp_lfosync_press,   //BTNEN_LFOSYNC
		msp_fltmode_press,   //BTNEN_FLTMODE
		msp_fltkeytr_press,  //BTNEN_FLTKEYTRK             -simple switch
		0,                   //BTNEN_O1RANGE_OFF
		0,                   //BTNEN_O2RANGE_OFF
		0,                   //BTNEN_OSCSYNC_OFF
		0,                   //BTNEN_PARAPH_OFF
		0,                   //BTNEN_LFOSYNC_OFF
		0,                   //BTNEN_FLTMODE_OFF
		0,                   //BTNEN_FLTKEYTRK_OFF
};

const neutr_lfo_t *const nlfo[16] =
{
		//normal
		&nl_ex_classic, //0 - classic
		&nl_2andy,      //1 - assign2
		&nl_ex_mono,    //2 - monophonic oscillator
		&nl_4poly,      //3 - polyphonic
		&nl_5math,      //4 -
		//inverted leds
		&nl_1classic,  //5 -
		&nl0_boot,//&nl_8sdc,      //6 -
		&nl0_boot,      //7 -
		&nl0_boot,      //8 -
		&nl0_boot,      //9 -
		//just to be shure
		&nl0_boot,    //A
		&nl0_boot,    //B
		&nl0_boot,    //C
		&nl0_boot,    //D
		&nl0_boot,    //E
		&nl0_boot     //F
};


uint8_t portaind = 1;

void porta_default(uint16_t val)
{
	parameters.o1glide = val;
	parameters.o2glide = val;
	portaind = 0;
}

void porta_o1(uint16_t val)
{
	parameters.o1glide = val;
	page.buttons[BTNEN_O1RANGE_OFF] = msp_root;
	page.buttons[BTNEN_O2RANGE_OFF] = msp_root;
	portaind = 1;
}

void porta_o2(uint16_t val)
{
	parameters.o2glide = val;
	page.buttons[BTNEN_O1RANGE_OFF] = msp_root;
	page.buttons[BTNEN_O2RANGE_OFF] = msp_root;
	portaind = 1;
}

//i'm bad at naming things, sorry
void How_do_you_like_it_Elon_Musk(uint16_t val)
{
	knob_lock(KNOB_LFOSHAPE,0,0,0);
	//explanation from past: zero callback leads to activate lfoshape without knob state
	//code is strange because of 48kHz processing
	(void)val;
}

void How_do_you_like_it_rate(uint16_t val)
{
	knob_lock(KNOB_LFORATE,0,0,0);
	(void)val;
}

void msp_root()

{
	led_blink(0,0);//clear all blinks
	menublink = 0;

	knob_lock(KNOB_PORTA,0,portaind,porta_default);
	knob_lock(KNOB_LFOSHAPE,lfoshape,2,How_do_you_like_it_Elon_Musk);
	knob_lock(KNOB_LFORATE,lforate,2,How_do_you_like_it_rate);
	//1 - reset leds

	//led_set(LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE,0);
	pp_setdac(TUNEM_NORMAL);

	if (parameters.o1widerange)
	{
		led_set(OSC1_FULLRANGE | LEDBTN_OSC1RANGE,1);
	}
	else
	{
		led_set(OSC1_FULLRANGE | LEDBTN_OSC1RANGE,0);
	}
	if (parameters.o1moddis)
	{
		O1MODPORT = O1MODDIS;
	}
	else
	{
		O1MODPORT = O1MODEN;
	}
	led_set(LED_OSC1_8 | LED_OSC1_16 | LED_OSC1_32,0);
	switch (parameters.o1oct)
	{
	case 0:
		led_set(LED_OSC1_32,1);
		break;
	case 1:
		led_set(LED_OSC1_16,1);
		break;
	case 2:
		led_set(LED_OSC1_8,1);
		break;
	}
	if (parameters.o2widerange)
	{
		led_set(OSC2_FULLRANGE | LEDBTN_OSC2RANGE,1);
	}
	else
	{
		led_set(OSC2_FULLRANGE | LEDBTN_OSC2RANGE,0);
	}
	if (parameters.o2moddis)
	{
		O2MODPORT = O2MODDIS;
	}
	else
	{
		O2MODPORT = O2MODEN;
	}
	led_set(LED_OSC2_8 | LED_OSC2_16 | LED_OSC2_32,0);
	switch (parameters.o2oct)
	{
	case 0:
		led_set(LED_OSC2_32,1);
		break;
	case 1:
		led_set(LED_OSC2_16,1);
		break;
	case 2:
		led_set(LED_OSC2_8,1);
		break;
	}

	pp_octaveo1(parameters.o1oct);
	pp_octaveo2(parameters.o2oct);

	switch (parameters.filtertype)
	{
	case 0: //lowpass
		led_set(LED_FILTERLP | FILTERMODE_1 | FILTERMODE_2,1);
		led_set(LED_FILTERBP | LED_FILTERHP,0);
		break;
	case 1: //bandpass
		led_set(LED_FILTERLP | LED_FILTERHP | FILTERMODE_2,0);
		led_set(LED_FILTERBP | FILTERMODE_1,1);
		break;
	case 2: //highpass
		led_set(LED_FILTERLP | LED_FILTERBP | FILTERMODE_1,0);
		led_set(LED_FILTERHP | FILTERMODE_2,1);
		break;
	}

	if (parameters.filterkeytrack)
	{
		led_set(LEDBTN_FLTKEYTRK, 1);
	}
	else
	{
		led_set(LEDBTN_FLTKEYTRK, 0);
	}

	if (parameters.osync)
	{
		OSCSYNCPORT = OSCSYNCON;
		led_set(LEDBTN_OSCSYNC, 1);
	}
	else
	{
		OSCSYNCPORT = OSCSYNCOFF;
		led_set(LEDBTN_OSCSYNC, 0);
	}

	switch (parameters.paraphony)
	{
	case 0:
		led_set(LEDBTN_PARAPHONIC,0);
		break;
	case 1:
		led_set(LEDBTN_PARAPHONIC,1);
		break;
	case 2:
		led_set(LEDBTN_PARAPHONIC,1); //HALFLED?
		break;
	}

	if (parameters.lfokeysync)
	{
		led_set(LEDBTN_LFOSYNC, 1);
	}
	else
	{
		led_set(LEDBTN_LFOSYNC, 0);
	}

	// 2 - load frame pointer
	if (neutron_lfo != nlfo[parameters.lfomode & 0xF])
	{
		uint8_t prevmode = 0;
		for (uint8_t i = 0; i<16; i++)
		{
			if (neutron_lfo == nlfo[i])
				prevmode = i;
		}
		neutron_lfo = nlfo[parameters.lfomode & 0xF];
		neutron_lfo->initialize(prevmode); //hello hardfault, but it is MUST!!
	}

	lfoledproc = neutron_lfo->lfoleds; // msl_def_timer;

	// 3 - load buttons

	page = mspage_root;

	// 4 - reenable sound


	OUTPUTPORT = OUTPUTENABLE; //output enable sync disable
	// PA07  GPIO OUT       osc1 enable cv mod
	// PC15  GPIO OUT       osc2 enable cv mod

}


