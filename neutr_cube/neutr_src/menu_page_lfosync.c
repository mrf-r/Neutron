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
 * menu_page_lfosync.c
 *
 *  Created on: 12 марта 2019 г.
 */

#include "neutron.h"
#include "menu.h"
#include "neutr_lfo_common.h"

const menu_page_t mspage_lfosync;

//extern volatile uint16_t adc_val[POT_TOTAL];

void msl_lfosync(uint32_t frame)
{
	//parameters.lfosyncstart = adc_val[POT_GLIDE]<<4;
	if ((frame & 0x7F) == 0)
	{
		led_set(LED_LFO1SINE | LED_LFO2TRIANGLE | LED_LFO3RAMPDWN | LED_LFO4SQUARE | LED_LFO5RAMPUP, 0);
		if (parameters.lfomode < 5)
		{
			led_set(LED_LFO1SINE << parameters.lfomode, 1);
		}
		else
		{
			led_set(LED_LFO1SINE | LED_LFO2TRIANGLE | LED_LFO3RAMPDWN | LED_LFO4SQUARE | LED_LFO5RAMPUP, 1);
			if (parameters.lfomode < 10)
			{
				led_set(LED_LFO1SINE << (parameters.lfomode - 5), 0);
			}
		}
	}
}
void exti_comp_debug(uint16_t val);
void msk_trigphase(uint16_t adc)
{
	parameters.lfosyncstart = adc<<20;
}
void msk_phaseshift(uint16_t adc)
{
	lforam.phaseshift = adc<<20;
}
void msp_lfosync_press()
{
	led_blink(LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE | LEDBTN_OSCSYNC | LED_FILTERBP | LED_FILTERHP | LED_FILTERLP, DEFAULT_BLINK_TIME);
	page = mspage_lfosync;
	lfoledproc = msl_lfosync;
	led_set(LEDBTN_FLTKEYTRK | LED_FILTERBP | LED_FILTERHP | LED_FILTERLP | LEDBTN_PARAPHONIC, 0);
	led_set(LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE, 1);
	switch (parameters.lfoextsynctype)
	{
	case 0:
		led_set(LED_FILTERHP, 1);
		break;
	case 1:
		led_set(LED_FILTERLP, 1);
		break;
	case 2:
		led_set(LED_FILTERBP, 1);
		break;
	}

	if (parameters.lfomidisync)
	{
		led_set(LEDBTN_OSCSYNC, 1);
	}
	else
	{
		led_set(LEDBTN_OSCSYNC, 0);
	}
	knob_lock(KNOB_LFOSHAPE,parameters.lfosyncstart>>20,1,msk_trigphase); //THIS IS SIMPLE SWITCH
	knob_lock(KNOB_LFORATE,lforam.phaseshift>>20,2,msk_phaseshift);
	knob_lock(KNOB_PORTA,0,1,exti_comp_debug);
}

void msp_lfosync()
{
	if (parameters.lfokeysync)
	{
		parameters.lfokeysync = 0;
	}
	else
	{
		parameters.lfokeysync = 1;
	}
	msp_root();
}

void msp_lsdec()
{
	if (parameters.lfomode > 0)
	{
		parameters.lfomode--;
	}
	page.buttons[BTNEN_LFOSYNC_OFF] = msp_root;
}
void msp_lsinc()
{
	if (parameters.lfomode < 9)
	{
		parameters.lfomode++;
	}
	page.buttons[BTNEN_LFOSYNC_OFF] = msp_root;
}
void msp_lstempo()
{
	if (parameters.lfomidisync)
	{
		parameters.lfomidisync = 0;
		led_set(LEDBTN_OSCSYNC, 0);
	}
	else
	{
		parameters.lfomidisync = 1;
		led_set(LEDBTN_OSCSYNC, 1);
	}
	page.buttons[BTNEN_LFOSYNC_OFF] = msp_root;
}
void msp_lsedge()
{
	parameters.lfoextsynctype++;
	if (parameters.lfoextsynctype == 3)
	{
		parameters.lfoextsynctype = 0;
	}
	led_set(LED_FILTERBP | LED_FILTERHP | LED_FILTERLP, 0);
	switch (parameters.lfoextsynctype)
	{
	case 0:
		led_set(LED_FILTERHP, 1);
		break;
	case 1:
		led_set(LED_FILTERLP, 1);
		break;
	case 2:
		led_set(LED_FILTERBP, 1);
		break;
	}
	page.buttons[BTNEN_LFOSYNC_OFF] = msp_root;
}

//////////////////////////////////////////////////////////////////////////////

const menu_page_t mspage_lfosync =
{
		msp_lsdec,           //BTNEN_O1RANGE
		msp_lsinc,           //BTNEN_O2RANGE
		msp_lstempo,         //BTNEN_OSCSYNC
		0,                   //BTNEN_PARAPH
		0,                   //BTNEN_LFOSYNC
		msp_lsedge,          //BTNEN_FLTMODE
		0,                   //BTNEN_FLTKEYTRK
		0,                   //BTNEN_O1RANGE_OFF
		0,                   //BTNEN_O2RANGE_OFF
		0,                   //BTNEN_OSCSYNC_OFF
		0,                   //BTNEN_PARAPH_OFF
		msp_lfosync,         //BTNEN_LFOSYNC_OFF
		0,                   //BTNEN_FLTMODE_OFF
		0,                   //BTNEN_FLTKEYTRK_OFF
};
