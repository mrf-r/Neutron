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
 * menu_page_osync.c
 *
 *  Created on: Mar 8, 2019
 */


#include "neutron.h"
#include "menu.h"

const menu_page_t mspage_osync;
void msp_assignmenu(void);
const menu_page_t mspage_assignmenu;


/*
 * assign tuning process
 * press keytrk while holding osc sync
 * then you enter assign tune page
 * turn lfo shape to switch octave
 * key is always C
 * you need to tune 1.000V 2.000V 3.000V ...
 * you can switch between scale adjust and offset adjust by pressing osc sync
 * if osc sunc is lit then scale, else offset
 * press and hold osc range buttons to increase or decrease coefficients
 * you can see lowest part of a number in binary format on octave leds
 */

void msl_lfoassign(uint32_t frame)
{
	if ((frame & 0x7F) == 0)
	{
		led_set(LED_LFO1SINE | LED_LFO2TRIANGLE | LED_LFO3RAMPDWN | LED_LFO4SQUARE | LED_LFO5RAMPUP, 0);
		if (parameters.asmode < 5)
		{
			led_set(LED_LFO1SINE << parameters.asmode, 1);
		}
	}
}

void msp_osync_press()
{
	led_blink(LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE | LEDBTN_LFOSYNC | LEDBTN_FLTKEYTRK, DEFAULT_BLINK_TIME);
	page = mspage_osync;
	lfoledproc = msl_lfoassign;
	if (parameters.gateretrig)
	{
		led_set(LEDBTN_LFOSYNC, 1);
	}
	else
	{
		led_set(LEDBTN_LFOSYNC, 0);
	}
}
void msp_osync()
{
	if (parameters.osync)
	{
		parameters.osync = 0;
	}
	else
	{
		parameters.osync = 1;
	}
	msp_root();
}
void msp_assigndec()
{
	if (parameters.asmode > 0)
	{
		parameters.asmode--;
	}
	page.buttons[BTNEN_OSCSYNC_OFF] = msp_root;
}
void msp_assigninc()
{
	if (parameters.asmode < 4)
	{
		parameters.asmode++;
	}
	page.buttons[BTNEN_OSCSYNC_OFF] = msp_root;
}
void msp_envretrig()
{
	if (parameters.gateretrig)
	{
		parameters.gateretrig = 0;
		led_set(LEDBTN_LFOSYNC, 0);
	}
	else
	{
		parameters.gateretrig = 1;
		led_set(LEDBTN_LFOSYNC, 1);
	}
	page.buttons[BTNEN_OSCSYNC_OFF] = msp_root;
}
const menu_page_t mspage_osync =
{
		msp_assigndec,       //BTNEN_O1RANGE
		msp_assigninc,       //BTNEN_O2RANGE
		0,                   //BTNEN_OSCSYNC
		0,                   //BTNEN_PARAPH
		msp_envretrig,       //BTNEN_LFOSYNC
		0,                   //BTNEN_FLTMODE
		msp_assignmenu,    //BTNEN_FLTKEYTRK
		0,                   //BTNEN_O1RANGE_OFF
		0,                   //BTNEN_O2RANGE_OFF
		msp_osync,          //BTNEN_OSCSYNC_OFF
		0,                   //BTNEN_PARAPH_OFF
		0,                   //BTNEN_LFOSYNC_OFF
		0,                   //BTNEN_FLTMODE_OFF
		0,                   //BTNEN_FLTKEYTRK_OFF
};


void msl_assigntune(uint32_t frame)
{
	if ((frame & 0x7F) == 0)
	{
		if (tunemode)
		{
			parameters.astuneoff += shiftvalue<<MSP_TUNESHIFT_ASOFF;
			led_set(LED_OSC2_32, (parameters.astuneoff >>(0 + MSP_TUNESHIFT_ASOFF)) & 0x1);
			led_set(LED_OSC1_32, (parameters.astuneoff >>(1 + MSP_TUNESHIFT_ASOFF)) & 0x1);
			led_set(LED_OSC2_16, (parameters.astuneoff >>(2 + MSP_TUNESHIFT_ASOFF)) & 0x1);
			led_set(LED_OSC1_16, (parameters.astuneoff >>(3 + MSP_TUNESHIFT_ASOFF)) & 0x1);
			led_set(LED_OSC2_8,  (parameters.astuneoff >>(4 + MSP_TUNESHIFT_ASOFF)) & 0x1);
			led_set(LED_OSC1_8,  (parameters.astuneoff >>(5 + MSP_TUNESHIFT_ASOFF)) & 0x1);
		}
		else
		{
			parameters.astunecf += shiftvalue<<MSP_TUNESHIFT_ASCF;
			led_set(LED_OSC2_32, (parameters.astunecf >>(0 + MSP_TUNESHIFT_ASCF)) & 0x1);
			led_set(LED_OSC1_32, (parameters.astunecf >>(1 + MSP_TUNESHIFT_ASCF)) & 0x1);
			led_set(LED_OSC2_16, (parameters.astunecf >>(2 + MSP_TUNESHIFT_ASCF)) & 0x1);
			led_set(LED_OSC1_16, (parameters.astunecf >>(3 + MSP_TUNESHIFT_ASCF)) & 0x1);
			led_set(LED_OSC2_8,  (parameters.astunecf >>(4 + MSP_TUNESHIFT_ASCF)) & 0x1);
			led_set(LED_OSC1_8,  (parameters.astunecf >>(5 + MSP_TUNESHIFT_ASCF)) & 0x1);
		}
		pp_setdac(TUNEM_AS);
	}
}

void msp_assignmenu()
{
	//  @TODO: disable sr write to this DAC channel
	led_blink(LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE | LEDBTN_OSCSYNC, DEFAULT_BLINK_TIME);
	page = mspage_assignmenu;
	lfoledproc = msl_assigntune;
	led_set(LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE | LEDBTN_OSCSYNC | LED_LFO1SINE | LED_LFO2TRIANGLE | LED_LFO3RAMPDWN | LED_LFO4SQUARE | LED_LFO5RAMPUP, 1);
	led_set(LEDBTN_PARAPHONIC | LEDBTN_FLTKEYTRK | LEDBTN_LFOSYNC , 0);
	tunemode = 0;
	shiftvalue = 0;
	menublink = 1;
}
//


void msp_astunedec()
{
	shiftvalue = -1;
}
void msp_astuneinc()
{
	shiftvalue = 1;
}
void msp_astunestop()
{
	shiftvalue = 0;
}
void msp_astunemode()
{
	if (tunemode)
	{
		tunemode = 0;
		led_blink(LED_OSC1_16 | LED_OSC2_16, DEFAULT_BLINK_AFTERTOUCH);
	}
	else
	{
		tunemode = 1;
		led_blink(LED_OSC1_32 | LED_OSC2_32, DEFAULT_BLINK_AFTERTOUCH);
	}
}
const menu_page_t mspage_assignmenu=
{
		msp_astunedec,       //BTNEN_O1RANGE
		msp_astuneinc,       //BTNEN_O2RANGE
		msp_astunemode,      //BTNEN_OSCSYNC
		msp_root,            //BTNEN_PARAPH
		msp_root,            //BTNEN_LFOSYNC
		msp_root,            //BTNEN_FLTMODE
		0,                   //BTNEN_FLTKEYTRK
		msp_astunestop,      //BTNEN_O1RANGE_OFF
		msp_astunestop,      //BTNEN_O2RANGE_OFF
		0,                   //BTNEN_OSCSYNC_OFF
		0,                   //BTNEN_PARAPH_OFF
		0,                   //BTNEN_LFOSYNC_OFF
		0,                   //BTNEN_FLTMODE_OFF
		0,                   //BTNEN_FLTKEYTRK_OFF
};
