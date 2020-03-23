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
 * neutr_lfo_common.h
 *
 *  Created on: Aug 7, 2019
 */

#ifndef NEUTR_LFO_COMMON_H_
#define NEUTR_LFO_COMMON_H_

#include "stdint.h"

//so we don't analyze signal frequency or period, just actual difference between PB2 state changes
//key here is comparing T2 value, that is real with calculated t6 value, that is jitter affected
//t6 value calculation : timestamp delta * 0x3E8 + T6 - zT6
//after comparison we will get either ~0 or ~0x3E8 difference with some noise

//we must define borders

//Jitter Compensation
//timestamp value to switch compensation on
#define JC_TS_THRESHOLD 0x40
//maxdif - value from 500 to 999, where 500 - possible wrong, 999 - almost not working
#define JC_MAXDIF  800
#define JC_FULLCOMP_SHIFT 8


#define DTS 0x1000


typedef struct
{
	uint32_t r_phase;
	uint32_t r_inc;
	uint32_t r_shift;
	uint32_t r_recp;
	uint32_t f_phase;
	uint32_t f_inc;
	uint32_t f_shift;
	uint32_t f_recp;
	uint32_t f_inv;
	uint32_t note;
	uint32_t priority;
	uint32_t active;
	uint32_t amplitude;
	uint32_t to_release;
}nl4p_voice_t;

typedef union
{
	volatile int16_t dd[DTS];
	struct
	{
		//common

		uint32_t phaseacc;
		volatile uint32_t phaseshift;

		uint8_t irqstate;
		uint16_t irqpos;
		uint32_t zts;       //timestamp        - fsts
		uint16_t ztpos;     //prev timer6 pos  - fits
		uint32_t zt2p;      //prev t2pos       - fiti
		uint32_t t2lp;      //timer2lastperiod - fits
		uint32_t zssp;      //subsamplepos     - fsts
		uint32_t fullcomp;   //full t2 compensation - initially equals to exti_timcomp << JC_FULLCOMP_SHIFT

		uint8_t sync_blink;
		uint8_t ledvals[5];

		//midi clock
		uint32_t ms_inc;
		uint16_t ms_shift;
		uint16_t ms_recp;

		uint32_t mclk_ztimestamp;
		uint32_t mclk_lastvaliddelta;
		uint16_t mclk_delta;
		uint32_t mclk_phase;
		uint8_t mclk_startreq;

		uint8_t midisyncstate;

		//1 - classic
		uint16_t shape_min; //@TODO: you need to be in sync with other lfo modes !
		uint16_t shape_max; //also you need to sync with MAXLED

		//2 - AHB
		uint16_t nl2ahb_breath;
		uint16_t nl2ahb_pitch;

		//3 - mono
		//subsample position
		uint32_t subsamplepos;
		//0 or 180 phase shift if rise or fall
		uint32_t nextphaseflip;
		//phase and both osc's wave positions at the sync moment
		uint32_t wsave_one;
		uint32_t wsave_two;
		uint32_t prevphase;
		int32_t nl3m_pitch;
		uint32_t nl3m_divr;


		//4 - poly
		nl4p_voice_t nl4p_voices[5];
		uint16_t nl4p_detune;
		uint16_t nl4p_waveform;
		uint32_t nl4p_attack; // sorry, it's just glinc[0];
		uint32_t nl4p_release; //idk how to make it properly
		uint8_t nl4p_damperstate;
		uint8_t nl4p_currentvoice;
		uint32_t nl4p_rand;  //??
		uint32_t nl4p_fltshaper;
		//there are 3 buffers, 16 samples each
		uint32_t nl4p_buffer[64];
		uint8_t nl4p_bpos;
		uint8_t nl4p_lub;
		uint8_t nl4p_upd_cv;
		uint8_t nl4p_keysyncsate;
		uint32_t nl4p_timed[32]; //MUST BE LESS THAN 3E80 - half of processing time. don't forget about other stuff like midi, leds...

		//5-test
		int32_t nl5t_zout;
		int32_t nl5t_zsample;
		uint32_t nl5t_nextsample;
		uint8_t nl5t_status;
		uint32_t nl5t_limstate;

	};
}lforam_t;



void nl4p_panic(void);
void nl_init_classic(uint8_t prevmode);
extern volatile lforam_t lforam;
extern volatile uint32_t exti_timcomp;
extern volatile uint32_t timestamp; //DO NOT MODIFY THIS!!!

#endif /* NEUTR_LFO_COMMON_H_ */
