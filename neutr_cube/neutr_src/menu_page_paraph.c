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
 * menu_page_paraph.c
 *
 *  Created on: Mar 12, 2019
 */

#include "neutron.h"
#include "menu.h"

const menu_page_t mspage_paraph;

void msp_paraph_press()
{
	if (parameters.o1moddis)
		led_set(LEDBTN_OSC1RANGE, 1);
	else
		led_set(LEDBTN_OSC1RANGE, 0);
	if (parameters.o2moddis)
			led_set(LEDBTN_OSC2RANGE, 1);
		else
			led_set(LEDBTN_OSC2RANGE, 0);
	led_blink(LEDBTN_OSCSYNC | LEDBTN_FLTKEYTRK | LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE, DEFAULT_BLINK_TIME);
	page = mspage_paraph;
	//lfoleds
	led_set(LEDBTN_OSCSYNC, 0);
}

void msp_paraph()
{
	parameters.paraphony++;
	if (parameters.paraphony == 3)
	{
		parameters.paraphony = 0;
	}
	if (parameters.paraphony)
	{
		O1MODPORT = O1MODDIS;
		O2MODPORT = O2MODDIS;
		parameters.o1moddis = 1;
		parameters.o2moddis = 1;
	}
	else
	{
		O1MODPORT = O1MODEN;
		O2MODPORT = O2MODEN;
		parameters.o1moddis = 0;
		parameters.o2moddis = 0;
	}
	//do not send this to msp_root!
	va_setmode(parameters.paraphony); //@TODO: check thread access problems!!!
	msp_root();
}

void msp_initialize()
{
	nvm_initialize();
	msp_root();
	led_blink(LEDS_ALL, 15);
}

void msp_savenvm()
{
	uint8_t savestate = save_parameters();
	msp_root();
	if (savestate)
	{
		led_set(LEDS_ALL ,0);
		led_blink(LED_LFO1SINE << savestate, 50);
	}
	else
	{
		led_blink(LEDS_ALL ,DEFAULT_BLINK_TIME);
	}
	//led_blink(LEDS_ALL, 15);
}


void msp_savemenu()
{
	led_blink(LEDBTN_FLTKEYTRK, DEFAULT_BLINK_TIME);
	page.buttons[BTNEN_OSCSYNC_OFF] = msp_savenvm;
	page.buttons[BTNEN_FLTKEYTRK] = msp_initialize;
}

void msp_o1modsw()
{
	if (parameters.o1moddis)
	{
		parameters.o1moddis = 0;
		O1MODPORT = O1MODEN;
		led_set(LEDBTN_OSC1RANGE, 0);
	}
	else
	{
		parameters.o1moddis = 1;
		O1MODPORT = O1MODDIS;
		led_set(LEDBTN_OSC1RANGE, 1);
	}
	page.buttons[BTNEN_PARAPH_OFF] = msp_root;
}

void msp_o2modsw()
{
	if (parameters.o2moddis)
	{
		parameters.o2moddis = 0;
		O2MODPORT = O2MODEN;
		led_set(LEDBTN_OSC2RANGE, 0);
	}
	else
	{
		parameters.o2moddis = 1;
		O2MODPORT = O2MODDIS;
		led_set(LEDBTN_OSC2RANGE, 1);
	}
	page.buttons[BTNEN_PARAPH_OFF] = msp_root;
}

void msp_panic()
{
	//@TODO: place here
	va_panic();
	page.buttons[BTNEN_PARAPH_OFF] = msp_root;
}

const menu_page_t mspage_paraph =
{
		msp_o1modsw,         //BTNEN_O1RANGE
		msp_o2modsw,         //BTNEN_O2RANGE
		msp_savemenu,        //BTNEN_OSCSYNC
		0,                   //BTNEN_PARAPH
		0,                   //BTNEN_LFOSYNC
		0,                   //BTNEN_FLTMODE
		msp_panic,           //BTNEN_FLTKEYTRK
		0,                   //BTNEN_O1RANGE_OFF
		0,                   //BTNEN_O2RANGE_OFF
		0,                   //BTNEN_OSCSYNC_OFF
		msp_paraph,          //BTNEN_PARAPH_OFF
		0,                   //BTNEN_LFOSYNC_OFF
		0,                   //BTNEN_FLTMODE_OFF
		0,                   //BTNEN_FLTKEYTRK_OFF
};


