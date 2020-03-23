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
 * menu_page_fltmode.c
 *
 *  Created on: 12 марта 2019 г.
 */

#include "neutron.h"
#include "menu.h"

const menu_page_t mspage_fltmode;
const menu_page_t mspage_ftune;
void msp_ftunemenu(void);

void msp_fltkeytr_press(void)
{
	//default - filter keytrack on/off (always to osc1)
	if (parameters.filterkeytrack)
	{
		parameters.filterkeytrack = 0;
		led_set(LEDBTN_FLTKEYTRK, 0);
	}
	else
	{
		parameters.filterkeytrack = 1;
		led_set(LEDBTN_FLTKEYTRK, 1);
	}
}

void msp_fltmode_press()
{
	led_blink(LEDBTN_PARAPHONIC, DEFAULT_BLINK_TIME);
	page = mspage_fltmode;
	// lfoleds
}
void msp_fltmode()
{
	parameters.filtertype++;
	if (parameters.filtertype == 3)
	{
		parameters.filtertype = 0;
	}
	msp_root();
}

const menu_page_t mspage_fltmode =
{
		0,                   //BTNEN_O1RANGE
		0,                   //BTNEN_O2RANGE
		0,                   //BTNEN_OSCSYNC
		msp_ftunemenu,       //BTNEN_PARAPH
		0,                   //BTNEN_LFOSYNC
		0,                   //BTNEN_FLTMODE
		0,                   //BTNEN_FLTKEYTRK
		0,                   //BTNEN_O1RANGE_OFF
		0,                   //BTNEN_O2RANGE_OFF
		0,                   //BTNEN_OSCSYNC_OFF
		0,                   //BTNEN_PARAPH_OFF
		0,                   //BTNEN_LFOSYNC_OFF
		msp_fltmode,         //BTNEN_FLTMODE_OFF
		0,                   //BTNEN_FLTKEYTRK_OFF
};

void msl_flttune(uint32_t frame)
{
	if ((frame & 0x7F) == 0)
	{
		parameters.flttunecf += shiftvalue << MSP_TUNESHIFT_FILTER;
		led_set(LED_OSC2_32, (parameters.flttunecf >>(MSP_TUNESHIFT_FILTER + 0)) & 0x1);
		led_set(LED_OSC1_32, (parameters.flttunecf >>(MSP_TUNESHIFT_FILTER + 1)) & 0x1);
		led_set(LED_OSC2_16, (parameters.flttunecf >>(MSP_TUNESHIFT_FILTER + 2)) & 0x1);
		led_set(LED_OSC1_16, (parameters.flttunecf >>(MSP_TUNESHIFT_FILTER + 3)) & 0x1);
		led_set(LED_OSC2_8,  (parameters.flttunecf >>(MSP_TUNESHIFT_FILTER + 4)) & 0x1);
		led_set(LED_OSC1_8,  (parameters.flttunecf >>(MSP_TUNESHIFT_FILTER + 5)) & 0x1);
		pp_setdac(TUNEM_FLT);
	}
}

void msp_fltautotune()
{
	//@TODO: place code here
	//nl4p_test();
}

extern const menu_page_t mspage_assignmenu;

void msp_ftunemenu()
{
	led_blink(LED_OSC1_16 | LED_OSC2_16, DEFAULT_BLINK_AFTERTOUCH);
	page = mspage_assignmenu;
	//page.buttons[BTNEN_FLTKEYTRK] = msp_fltautotune;
	page.buttons[BTNEN_OSCSYNC] = 0;

	lfoledproc = msl_flttune;
	led_set(LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE | LEDBTN_FLTKEYTRK | LED_LFO1SINE | LED_LFO2TRIANGLE | LED_LFO3RAMPDWN | LED_LFO4SQUARE | LED_LFO5RAMPUP, 1);
	led_set(LEDBTN_PARAPHONIC | LEDBTN_LFOSYNC | LEDBTN_OSCSYNC, 0);
	//parameters.filterkeytrack = 1;
	shiftvalue = 0;
	menublink = 1;
}
//
