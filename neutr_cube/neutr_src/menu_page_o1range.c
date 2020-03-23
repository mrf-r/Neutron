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
 * menu_page_o1range.c
 *
 *  Created on: Mar 11, 2019
 */
#include "menu.h"

const menu_page_t mspage_o1range;
const menu_page_t mspage_o1menu;
void msp_o1menu(void);

void msp_o1range_press()
{
	led_blink(LEDBTN_OSC2RANGE | LEDBTN_PARAPHONIC | LEDBTN_FLTKEYTRK, DEFAULT_BLINK_TIME);
	page = mspage_o1range;

	knob_lock(KNOB_PORTA,0,1,porta_o1);
	//lfoleds stays
}

void msp_tuneboth()
{
	led_set(LED_OSC1_16 | LED_OSC1_32 | LED_OSC1_8 | LED_OSC2_16 | LED_OSC2_32 | LED_OSC2_8 | LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE, 1);
	ft_o1tune();
	ft_o2tune();
	msp_root();
}

void msp_o1wide()
{
	if (parameters.o1widerange)
	{
		parameters.o1widerange = 0;
		led_set(OSC1_FULLRANGE | LEDBTN_OSC1RANGE,0);
	}
	else
	{
		parameters.o1widerange = 1;
		led_set(OSC1_FULLRANGE | LEDBTN_OSC1RANGE,1);
	}
	page.buttons[BTNEN_O1RANGE_OFF] = msp_root;
	page.buttons[BTNEN_FLTKEYTRK] = msp_tuneboth;
}

void msp_o1octave()
{
	parameters.o1oct++;
	if (parameters.o1oct == 3)
	{
		parameters.o1oct = 0;
	}
	msp_root();
}
void msp_o1autotune(void);

const menu_page_t mspage_o1range =
{
		0,                   //BTNEN_O1RANGE
		msp_o1wide,       //BTNEN_O2RANGE
		0,                   //BTNEN_OSCSYNC
		msp_o1menu,          //BTNEN_PARAPH
		0,                   //BTNEN_LFOSYNC
		0,                   //BTNEN_FLTMODE
		msp_o1autotune,      //BTNEN_FLTKEYTRK
		msp_o1octave,        //BTNEN_O1RANGE_OFF
		0,                   //BTNEN_O2RANGE_OFF
		0,                   //BTNEN_OSCSYNC_OFF
		0,                   //BTNEN_PARAPH_OFF
		0,                   //BTNEN_LFOSYNC_OFF
		0,                   //BTNEN_FLTMODE_OFF
		0,                   //BTNEN_FLTKEYTRK_OFF
};


//extern volatile uint16_t adc_val[POT_TOTAL];

void o1_setpwrange(uint16_t potval)
{
	parameters.o1pwrange = (potval * 13)>>12; //adc - 0 to 0x0FFF
}
void knob_na2(uint16_t potval)
{
	(void)potval;
}

void msl_o1tune(uint32_t frame)
{
	if ((frame & 0x7F) == 0)
	{
		switch (tunemode)
		{
		case 0:
			//parameters.o1pwrange = (adc_val[POT_LFOSHAPE] * 13)>>12; //adc - 0 to 0x0FFF
			led_set(LED_OSC2_32, parameters.o1pwrange & 0x1);
			led_set(LED_OSC1_32, (parameters.o1pwrange >>1) & 0x1);
			led_set(LED_OSC2_16, (parameters.o1pwrange >>2) & 0x1);
			led_set(LED_OSC1_16, (parameters.o1pwrange >>3) & 0x1);
			led_set(LED_OSC2_8,  (parameters.o1pwrange >>4) & 0x1);
			led_set(LED_OSC1_8,  (parameters.o1pwrange >>5) & 0x1);
			break;
		case 1:
			parameters.o1tuneoctcf += shiftvalue<<MSP_TUNESHIFT_OSCCF;
			led_set(LED_OSC2_32, (parameters.o1tuneoctcf >>(0 + MSP_TUNESHIFT_OSCCF)) & 0x1);
			led_set(LED_OSC1_32, (parameters.o1tuneoctcf >>(1 + MSP_TUNESHIFT_OSCCF)) & 0x1);
			led_set(LED_OSC2_16, (parameters.o1tuneoctcf >>(2 + MSP_TUNESHIFT_OSCCF)) & 0x1);
			led_set(LED_OSC1_16, (parameters.o1tuneoctcf >>(3 + MSP_TUNESHIFT_OSCCF)) & 0x1);
			led_set(LED_OSC2_8,  (parameters.o1tuneoctcf >>(4 + MSP_TUNESHIFT_OSCCF)) & 0x1);
			led_set(LED_OSC1_8,  (parameters.o1tuneoctcf >>(5 + MSP_TUNESHIFT_OSCCF)) & 0x1);
			pp_setdac(TUNEM_O1OCT);
			break;
		case 2:
			parameters.o1tunecf += shiftvalue<<MSP_TUNESHIFT_OSCCF;
			led_set(LED_OSC2_32, (parameters.o1tunecf >>(0 + MSP_TUNESHIFT_OSCCF)) & 0x1);
			led_set(LED_OSC1_32, (parameters.o1tunecf >>(1 + MSP_TUNESHIFT_OSCCF)) & 0x1);
			led_set(LED_OSC2_16, (parameters.o1tunecf >>(2 + MSP_TUNESHIFT_OSCCF)) & 0x1);
			led_set(LED_OSC1_16, (parameters.o1tunecf >>(3 + MSP_TUNESHIFT_OSCCF)) & 0x1);
			led_set(LED_OSC2_8,  (parameters.o1tunecf >>(4 + MSP_TUNESHIFT_OSCCF)) & 0x1);
			led_set(LED_OSC1_8,  (parameters.o1tunecf >>(5 + MSP_TUNESHIFT_OSCCF)) & 0x1);
			pp_setdac(TUNEM_O1P);
			break;
		case 3:
			parameters.o1tuneoff += shiftvalue<<MSP_TUNESHIFT_OSCOFF; //@TODO: limits, dude!!!!!!!!!!!!!!!!!!
			led_set(LED_OSC2_32, (parameters.o1tuneoff >>(0 + MSP_TUNESHIFT_OSCOFF)) & 0x1);
			led_set(LED_OSC1_32, (parameters.o1tuneoff >>(1 + MSP_TUNESHIFT_OSCOFF)) & 0x1);
			led_set(LED_OSC2_16, (parameters.o1tuneoff >>(2 + MSP_TUNESHIFT_OSCOFF)) & 0x1);
			led_set(LED_OSC1_16, (parameters.o1tuneoff >>(3 + MSP_TUNESHIFT_OSCOFF)) & 0x1);
			led_set(LED_OSC2_8,  (parameters.o1tuneoff >>(4 + MSP_TUNESHIFT_OSCOFF)) & 0x1);
			led_set(LED_OSC1_8,  (parameters.o1tuneoff >>(5 + MSP_TUNESHIFT_OSCOFF)) & 0x1);
			pp_setdac(TUNEM_O1OFF);
			break;
		}
	}
}

void msp_o1menu()
{
	led_blink(LED_LFO1SINE | LED_LFO2TRIANGLE | LED_LFO3RAMPDWN | LED_LFO4SQUARE | LED_LFO5RAMPUP, DEFAULT_BLINK_AFTERTOUCH );
	page = mspage_o1menu;
	lfoledproc = msl_o1tune;
	knob_lock(KNOB_LFOSHAPE,0,1,o1_setpwrange);
	led_set(LEDBTN_OSCSYNC | LEDBTN_FLTKEYTRK | LED_LFO1SINE | LED_LFO2TRIANGLE | LED_LFO3RAMPDWN | LED_LFO4SQUARE | LED_LFO5RAMPUP, 1);
	led_set(LEDBTN_PARAPHONIC | LEDBTN_LFOSYNC | LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE, 0);
	tunemode = 0;
	shiftvalue = 0;
	menublink = 1;
}
//

void msp_osctunemode()
{
	tunemode++;
	if (tunemode == 4)
	{
		tunemode = 0;
	}

	switch (tunemode)
	{
	case 0:
		led_set(LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE, 0);
		led_blink(LED_LFO1SINE | LED_LFO2TRIANGLE | LED_LFO3RAMPDWN | LED_LFO4SQUARE | LED_LFO5RAMPUP, DEFAULT_BLINK_AFTERTOUCH );
		O1MODPORT = O1MODEN;
		O2MODPORT = O2MODEN;
		knob_lock(KNOB_LFOSHAPE,0,1,o1_setpwrange);
		break;
	case 1:
		led_blink(LED_OSC1_8 | LED_OSC2_8, DEFAULT_BLINK_AFTERTOUCH );
		led_set(LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE, 1);
		led_set(OSC1_FULLRANGE | OSC2_FULLRANGE, 1);
		break;
	case 2:
		led_blink(LED_OSC1_16 | LED_OSC2_16, DEFAULT_BLINK_AFTERTOUCH );
		break;
	case 3: //offset
		O1MODPORT = O1MODDIS;
		O2MODPORT = O2MODDIS;
		led_blink(LED_OSC1_32 | LED_OSC2_32, DEFAULT_BLINK_AFTERTOUCH );
		break;
	}
}

void msp_o1tunemode()
{
	tunemode++;
	if (tunemode == 4)
	{
		tunemode = 0;
	}

	switch (tunemode)
	{
	case 0:
		led_set(LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE, 0);
		led_blink(LED_LFO1SINE | LED_LFO2TRIANGLE | LED_LFO3RAMPDWN | LED_LFO4SQUARE | LED_LFO5RAMPUP, DEFAULT_BLINK_AFTERTOUCH );
		O1MODPORT = O1MODEN;
		//O2MODPORT = O2MODEN;
		knob_lock(KNOB_LFOSHAPE,0,1,o1_setpwrange);
		break;
	case 1:
		led_blink(LED_OSC1_8, DEFAULT_BLINK_AFTERTOUCH );
		led_set(LEDBTN_OSC1RANGE | LEDBTN_OSC2RANGE, 1);
		led_set(OSC1_FULLRANGE, 1);
		knob_lock(KNOB_LFOSHAPE,0,0,knob_na2);
		break;
	case 2:
		led_blink(LED_OSC1_16, DEFAULT_BLINK_AFTERTOUCH );
		break;
	case 3: //offset
		O1MODPORT = O1MODDIS;
		led_blink(LED_OSC1_32, DEFAULT_BLINK_AFTERTOUCH );
		break;
	}
}

void msp_o1autotune()
{
	led_set(LED_OSC1_16 | LED_OSC1_32 | LED_OSC1_8 | LEDBTN_OSC1RANGE, 1);
	ft_o1tune();
	msp_root();
}

void msp_astunedec(void);
void msp_astuneinc(void);
void msp_astunestop(void);

const menu_page_t mspage_o1menu =
{
		msp_astunedec,       //BTNEN_O1RANGE
		msp_astuneinc,       //BTNEN_O2RANGE
		msp_o1tunemode,      //BTNEN_OSCSYNC
		msp_root,            //BTNEN_PARAPH
		msp_root,            //BTNEN_LFOSYNC
		msp_root,            //BTNEN_FLTMODE
		msp_o1autotune,      //BTNEN_FLTKEYTRK
		msp_astunestop,      //BTNEN_O1RANGE_OFF
		msp_astunestop,      //BTNEN_O2RANGE_OFF
		0,                   //BTNEN_OSCSYNC_OFF
		0,                   //BTNEN_PARAPH_OFF
		0,                   //BTNEN_LFOSYNC_OFF
		0,                   //BTNEN_FLTMODE_OFF
		0,                   //BTNEN_FLTKEYTRK_OFF
};

