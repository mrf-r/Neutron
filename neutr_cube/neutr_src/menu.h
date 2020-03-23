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
 * menu.h
 *
 *  Created on: Mar 8, 2019
 */

#ifndef MENU_H_
#define MENU_H_

#include "neutron.h"

#define MSP_TUNESHIFT_FILTER 3
#define MSP_TUNESHIFT_ASOFF  3
#define MSP_TUNESHIFT_ASCF   3
#define MSP_TUNESHIFT_OSCCF  3
#define MSP_TUNESHIFT_OSCOFF 3

#define DEFAULT_BLINK_TIME 5
#define DEFAULT_BLINK_AFTERTOUCH 3
//        ^ must be some logic behind this. i don't know


#define LEDS_ALL (LEDBTN_FLTKEYTRK|LEDBTN_FLTKEYTRK|LED_LFO5RAMPUP|\
		LED_LFO4SQUARE|LED_LFO3RAMPDWN|LED_LFO2TRIANGLE|LED_LFO1SINE|\
		LEDBTN_LFOSYNC|LED_FILTERLP|LED_FILTERHP|LED_FILTERBP|LEDBTN_OSC2RANGE\
		|LEDBTN_OSC1RANGE|LEDBTN_PARAPHONIC|LEDBTN_OSCSYNC|LED_OSC2_32\
		|LED_OSC2_16|LED_OSC2_8|LED_OSC1_32|LED_OSC1_16|LED_OSC1_8)

typedef struct
{
	void (*buttons[BTNEN_TOTAL])(void);
}menu_page_t;


//extern volatile uint8_t o1widerange;
//extern volatile uint8_t o2widerange;

extern volatile uint8_t tunemode;
extern volatile int8_t shiftvalue;
extern volatile uint8_t menublink;

typedef struct//do not save parameters from pots (i.e. glidetime or lfo freq)
{
	//osc
	volatile uint16_t o1tunecf;
	volatile uint16_t o1tuneoctcf;
	volatile int16_t  o1tuneoff;
	volatile uint16_t o2tunecf;
	volatile uint16_t o2tuneoctcf;
	volatile int16_t  o2tuneoff;
	//DO NOT FORGET TO CHANGE nvm.c AFTER EDITING PARAMETERS SET !
	volatile uint8_t  o1oct;
	volatile uint8_t  o2oct;
	volatile uint8_t  o1pwrange;
	volatile uint8_t  o2pwrange;
	volatile uint8_t  o1pitch;
	volatile uint8_t  o2pitch;
	volatile uint8_t  o1widerange;
	volatile uint8_t  o2widerange;
	volatile uint8_t  o1moddis;
	volatile uint8_t  o2moddis;
	volatile uint16_t o1glide;
	volatile uint16_t o2glide;

	//osc additional
	volatile uint8_t osync;
	volatile uint8_t paraphony;
	// uint8_t o1modoff;
	// uint8_t o2modoff;
	//uint8_t o1widerange;
	//uint8_t o2widerange;

	//filter
	volatile uint16_t flttunecf;
	volatile uint8_t filtertype;
	volatile uint8_t filterkeytrack;

	//cv out
	volatile uint16_t astunecf;
	volatile int16_t astuneoff;
	volatile uint8_t asmode;

	//gate
	volatile uint8_t gateretrig;

	//lfo
	volatile uint8_t lfokeysync;
	volatile uint8_t lfoextsynctype;
	volatile uint32_t lfosyncstart;
	volatile uint8_t lfomidisync;
	volatile uint8_t lfomode;

	volatile uint32_t validity_key;

}parameters_t;


extern volatile parameters_t parameters;
extern volatile menu_page_t page;
extern void (*lfoledproc)(uint32_t frame);

extern const neutr_lfo_t nl0_boot;
extern const neutr_lfo_t nl_1classic;
extern const neutr_lfo_t nl_2andy;
extern const neutr_lfo_t nl_3mono;
extern const neutr_lfo_t nl_4poly;
extern const neutr_lfo_t nl_5math;
extern const neutr_lfo_t nl_ex_mono;
extern const neutr_lfo_t nl_ex_classic;
extern const neutr_lfo_t nl_8sdc;
//@TODO: place other here
void nl_init_classic(uint8_t prevmode);

void led_blink(uint32_t led_bmp, uint16_t blinks);
void led_set(uint32_t led_bmp, uint8_t val);
void msp_root(void);

void porta_default(uint16_t val);
void porta_o1(uint16_t val);
void porta_o2(uint16_t val);


//knobs
typedef struct
{
	uint16_t lock_value;
	uint8_t lock;
	uint8_t compare_val;
	void (*cbk)(uint16_t val);
}knob_status_t;
void knob_na(uint16_t);
void knob_process(void);
void knob_lock(knobselect_en knob, uint16_t val, uint8_t type, void (*callback)(uint16_t val)); //type 1-from noise 2-to value 0-off
extern volatile uint16_t lfoshape;
extern volatile uint16_t lforate;


void porta_default(uint16_t val);
void porta_o1(uint16_t val);
void porta_o2(uint16_t val);





#endif /* MENU_H_ */

