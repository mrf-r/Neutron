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
 * neutr_lfo_4poly.c
 *
 *  Created on: Apr 17, 2019
 *
 *
 *    thanks kebby for inspiration
 */

#include "neutr_lfo_common.h"
#include "neutron.h"
#include "menu.h"

//unroll
#define REPEAT2(SOME) SOME SOME
#define REPEAT4(SOME) REPEAT2(SOME) REPEAT2(SOME)
#define REPEAT8(SOME) REPEAT4(SOME) REPEAT4(SOME)
#define REPEAT16(SOME) REPEAT8(SOME) REPEAT8(SOME)

#define LCG_A 0x08838435
#define LCG_B 0x36196368
#define NOISE_SHAPE 0xCCCC
#define NOISE_SHIFT 2048
//NOISE_SHAPE - 1st order hp filter coef
//NOISE_SHIFT - less is more loud, must be power of 2

#define VOICE_MAX_AMP 0x3328
#define VOICE_MAX_SHIFT 0x1994
#define VOICE_MID 0x19999999
#define VOICE_ACTIVE 0x3FFFFFFF
/*
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

nl4p_voice_t nl4p_voices[5];

uint16_t nl4p_detune;
uint16_t nl4p_waveform;
uint32_t nl4p_attack = 0x002C5772; // sorry, it's just glinc[0];
uint32_t nl4p_release = 0x002C5772; //idk how to make it properly

uint8_t nl4p_damperstate;
uint8_t nl4p_currentvoice;

uint32_t nl4p_rand;
uint32_t nl4p_fltshaper;

//there are 3 buffers, 16 samples each
uint32_t nl4p_buffer[64];
volatile uint8_t nl4p_bpos = 0;
volatile uint8_t nl4p_lub = 0;
uint8_t nl4p_upd_cv = 0;


uint8_t keysyncsate = 0;
volatile uint32_t nl4p_timed[32]; //MUST BE LESS THAN 3E80 - half of processing time. don't forget about other stuff like midi, leds...


*/
extern volatile knob_status_t knob_status[KNOB_TOTAL];

extern volatile int32_t pp_o1pwshift;

void nl4p_lfoleds(uint32_t frame)
{
	uint8_t pos = frame & 0x1F;
	uint8_t i;
	if (pos == 0) //
	{
		for (i = 0; i < 5; i++)
		{
			lforam.ledvals[i] = lforam.nl4p_voices[i].amplitude>>27;
		}
	}
	else // pwm leds
	{
		for (i = 0; i<5; i++)
		{
			if (lforam.ledvals[i]>pos)
			{
				led_set(LED_LFO1SINE<<i, 1);
			}
			else
			{
				led_set(LED_LFO1SINE<<i, 0);
			}
		}
	}
}


void nl4p_uvd(uint8_t voice)
{
	uint32_t r_inc,r_shift,r_recp;
	uint32_t f_inc,f_shift,f_recp;
	int32_t o1p = (lforam.nl4p_voices[voice].note<<9) + pp_o1pwshift;
	if (o1p < 0)
	{
		o1p += 0x1800;
	}
	else
	{
		if (o1p > 0xFC00)
			o1p -= 0x1800;
	}

	uint16_t tablepos = o1p>>8;
	uint8_t subpos = (uint8_t)o1p;

	r_inc = (uint32_t)((oscinc[tablepos+1].inc - oscinc[tablepos].inc)>>8)*subpos+oscinc[tablepos].inc;
	r_shift =  oscinc[tablepos].shift;
	uint8_t is = oscinc[tablepos+1].shift-oscinc[tablepos].shift;
	uint16_t ni = oscinc[tablepos+1].recp>>is;
	r_recp = oscinc[tablepos].recp - ((uint32_t)(oscinc[tablepos].recp - ni)*subpos>>8);

	if (lforam.nl4p_detune > 0x7FF)
	{
		o1p += (lforam.nl4p_detune>>3)&0xFF;
		tablepos = o1p>>8;
		subpos = (uint8_t)o1p;

		f_inc = (uint32_t)((oscinc[tablepos+1].inc - oscinc[tablepos].inc)>>8)*subpos+oscinc[tablepos].inc;
		f_shift =  oscinc[tablepos].shift;
		is = oscinc[tablepos+1].shift-oscinc[tablepos].shift;
		ni = oscinc[tablepos+1].recp>>is;
		f_recp = oscinc[tablepos].recp - ((uint32_t)(oscinc[tablepos].recp - ni)*subpos>>8);
		NVIC_DisableIRQ(TIM7_IRQn);
		lforam.nl4p_voices[voice].r_inc = r_inc;
		lforam.nl4p_voices[voice].r_shift = r_shift;
		lforam.nl4p_voices[voice].r_recp = r_recp;
		lforam.nl4p_voices[voice].f_inc = f_inc;
		lforam.nl4p_voices[voice].f_shift = f_shift;
		lforam.nl4p_voices[voice].f_recp = f_recp;
		if (lforam.nl4p_waveform>>11)
		{
			lforam.nl4p_voices[voice].f_inv = 1;
		}
		else
		{
			lforam.nl4p_voices[voice].f_inv = 0;
		}
		NVIC_EnableIRQ(TIM7_IRQn);
	}
	else
	{
		NVIC_DisableIRQ(TIM7_IRQn);
		lforam.nl4p_voices[voice].r_inc = r_inc;
		lforam.nl4p_voices[voice].r_shift = r_shift;
		lforam.nl4p_voices[voice].r_recp = r_recp;
		lforam.nl4p_voices[voice].f_inc = r_inc;
		lforam.nl4p_voices[voice].f_shift = r_shift;
		lforam.nl4p_voices[voice].f_recp = r_recp;
		if (lforam.nl4p_waveform>>11)
		{
			lforam.nl4p_voices[voice].f_phase = ~(lforam.nl4p_voices[voice].r_phase + 0x80000000 + (lforam.nl4p_detune<<20)); //21
			lforam.nl4p_voices[voice].f_inv = 1;
		}
		else
		{
			lforam.nl4p_voices[voice].f_phase = lforam.nl4p_voices[voice].r_phase + 0x80000000 + (lforam.nl4p_detune<<20);
			lforam.nl4p_voices[voice].f_inv = 0;
		}
		NVIC_EnableIRQ(TIM7_IRQn);
	}
}

void nl4p_async()
{
	nl4p_uvd(lforam.nl4p_upd_cv++);
	if (lforam.nl4p_upd_cv == 5)
		lforam.nl4p_upd_cv = 0;
}

/*
 * i need that description.
 *
 * calculate envelopes * osc mix
 * also calculate shift amp value
 *
 * calculate oscillators and add them to buf
 *
 * add overall shift value
 *
 * calculate noise, filter it and add to buffer
 *
 * total amp calculate, make limiter
 */

void nl4p_slotprocess(uint32_t* wb)
{
	uint32_t i;

	//calculate env * osc mix
	//uint32_t total_shift = 0;
	uint32_t total_amp = 0;

	uint32_t mul_f = (VOICE_MAX_AMP * sine[(uint8_t)((lforam.nl4p_waveform>>4)+0x80)])>>17; //max value is both at same lvl. min - rise only
	uint32_t mul_r = VOICE_MAX_AMP - mul_f;

	//clear buffer
	uint32_t *wptr = wb;
	REPEAT16 (*wptr = 0; wptr++;)


	for (i = 0; i<5; i++)
	{
		int32_t dif = lforam.nl4p_voices[i].active - lforam.nl4p_voices[i].amplitude;
		int32_t envinc;
		if (dif < 0)
		{
			envinc = dif/4194304 - 1;
			envinc *= lforam.nl4p_release;
			if (envinc < dif)
				envinc = dif;
		}
		else
		{
			envinc = dif/4194304 + 1;
			envinc *= lforam.nl4p_attack;
			if (envinc > dif)
				envinc = dif;
		}
		lforam.nl4p_voices[i].amplitude += envinc; //0 to 7FFFFFFF

		uint32_t amp = lforam.nl4p_voices[i].amplitude >>14; //0 to FFFF
		//calculate shift
		//total_shift += VOICE_MID - VOICE_MAX_SHIFT * amp;
		total_amp += amp;

		//calculate total attenuation
		uint32_t osc_mul_f = (amp * mul_f)>>16;
		uint32_t osc_mul_r = (amp * mul_r)>>16;


		//calculate voices



		// calc sawr
		uint32_t ph = lforam.nl4p_voices[i].r_phase;
		uint32_t inc = lforam.nl4p_voices[i].r_inc;
		uint32_t shift = lforam.nl4p_voices[i].r_shift;
		uint32_t recp = lforam.nl4p_voices[i].r_recp;
		wptr = wb;
		uint32_t sawr;
		REPEAT16(  \
				ph += inc; \
				sawr = ph; \
				if (sawr>(~inc))
				{
					//limit high
					uint32_t delta = (sawr + inc)>>shift;
					uint32_t rev = (delta * recp)>>13;
					rev = rev*rev;
					sawr -= rev;
				}
				else
				{
					if (sawr<inc)
					{
						//limit low
						uint32_t delta = (inc - sawr)>>shift;
						uint32_t rev = (delta * recp)>>13;
						rev = rev*rev;
						sawr += rev;
					}
				} \
				*wptr += (sawr>>16) * osc_mul_r; \
				wptr++; \
				)
		lforam.nl4p_voices[i].r_phase = ph;

		// calc sawd
		ph = lforam.nl4p_voices[i].f_phase;
		inc = lforam.nl4p_voices[i].f_inc;
		shift = lforam.nl4p_voices[i].f_shift;
		recp = lforam.nl4p_voices[i].f_recp;
		wptr = wb;
		if (lforam.nl4p_voices[i].f_inv)
		{
			// falling saw
			REPEAT16(  \
					ph -= inc; \
					sawr = ph; \
					if (sawr>(~inc))
					{
						//limit high
						uint32_t delta = (sawr + inc)>>shift;
						uint32_t rev = (delta * recp)>>13;
						rev = rev*rev;
						sawr -= rev;
					}
					else
					{
						if (sawr<inc)
						{
							//limit low
							uint32_t delta = (inc - sawr)>>shift;
							uint32_t rev = (delta * recp)>>13;
							rev = rev*rev;
							sawr += rev;
						}
					} \
					*wptr += (sawr>>16) * osc_mul_f; \
					wptr++; \
					)

		}
		else
		{
			// rising saw
			REPEAT16(  \
					ph += inc; \
					sawr = ph; \
					if (sawr>(~inc))
					{
						//limit high
						uint32_t delta = (sawr + inc)>>shift;
						uint32_t rev = (delta * recp)>>13;
						rev = rev*rev;
						sawr -= rev;
					}
					else
					{
						if (sawr<inc)
						{
							//limit low
							uint32_t delta = (inc - sawr)>>shift;
							uint32_t rev = (delta * recp)>>13;
							rev = rev*rev;
							sawr += rev;
						}
					} \
					*wptr += (sawr>>16) * osc_mul_f; \
					wptr++; \
					)

		}
		lforam.nl4p_voices[i].f_phase = ph;
	}
	//now add total shift and dither
	uint32_t total_shift = 0x80000000 - (VOICE_MAX_SHIFT * total_amp);

	wptr = wb;
	REPEAT16( *wptr += total_shift; wptr++;)
/*
	//now add dithering and shift signal to shiftval
	uint32_t rand = nl4p_rand;
	uint32_t fltacc = nl4p_fltshaper;
	wptr = wb;

	uint32_t ddif;

	REPEAT16( \
			rand = rand * LCG_A + LCG_B; \
			ddif = rand - fltacc; \
			fltacc = (ddif>>16)*NOISE_SHAPE + fltacc; \
			ddif = ddif/NOISE_SHIFT; \
			*wptr += dif + total_shift; \
			wptr++; \
			)
	nl4p_rand = rand;
	nl4p_fltshaper = fltacc;

*/
}
//////////////////////////////////////////////////////////////////////////////////////////

void nl4p_sr()
{
	LFO_OUTPUT = lforam.nl4p_buffer[lforam.nl4p_bpos++]>>16;
	if (lforam.nl4p_bpos == 64)
		lforam.nl4p_bpos = 0;
}

/*
uint8_t nl4p_rate_activemode;
uint8_t nl4p_shape_activemode;
uint16_t nl4p_rate_la;
uint16_t nl4p_shape_la;
uint8_t nl4p_zkeysync;


void nl4p_cr_deprecated2()
{
	if (nl4p_zkeysync != parameters.lfokeysync)
	{
		nl4p_rate_la = adc_val[POT_LFORATE];
		nl4p_shape_la = adc_val[POT_LFOSHAPE];
		nl4p_zkeysync = parameters.lfokeysync;
	}

	if (nl4p_shape_activemode == parameters.lfokeysync)
	{
		if (parameters.lfokeysync)
		{
			nl4p_detune = adc_val[POT_LFOSHAPE];
		}
		else
		{
			nl4p_waveform = adc_val[POT_LFOSHAPE];
		}
	}
	else
	{
		int16_t delta = nl4p_shape_la - adc_val[POT_LFOSHAPE];
		if (delta<0)
			delta = -delta;
		if (delta > 0x3F)
		{
			nl4p_shape_activemode = parameters.lfokeysync;
		}
	}

	if (nl4p_rate_activemode == parameters.lfokeysync)
	{
		if (parameters.lfokeysync)
		{
			nl4p_attack = glinc[adc_val[POT_LFORATE]>>4];
			nl4p_release = glinc[adc_val[POT_LFORATE]>>4];
		}
		else
		{
			nl4p_release = glinc[adc_val[POT_LFORATE]>>4];
		}
	}
	else
	{
		int16_t delta = nl4p_rate_la - adc_val[POT_LFORATE];
		if (delta<0)
			delta = -delta;
		if (delta > 0x3F)
		{
			nl4p_rate_activemode = parameters.lfokeysync;
		}
	}

	nl4p_zkeysync = parameters.lfokeysync;

	uint8_t btu = nl4p_bpos>>4;
	while (btu != nl4p_lub)
	{
		nl4p_slotprocess(&nl4p_buffer[nl4p_lub<<4]);
		nl4p_lub++;
		if (nl4p_lub == 4)
			nl4p_lub = 0;
		btu = nl4p_bpos>>4;
	}
}

void nl4p_cr_deprecated()
{
	//if lid than a+r and detune else r only and wave
	if (parameters.lfokeysync)
	{
		nl4p_detune = adc_val[POT_LFOSHAPE];
		nl4p_attack = glinc[adc_val[POT_LFORATE]>>4];
		nl4p_release = glinc[adc_val[POT_LFORATE]>>4];
	}
	else
	{
		nl4p_waveform = adc_val[POT_LFOSHAPE];
		nl4p_release = glinc[adc_val[POT_LFORATE]>>4];
	}

	uint8_t btu = nl4p_bpos>>4;
	while (btu != nl4p_lub)
	{
		nl4p_slotprocess(&nl4p_buffer[nl4p_lub<<4]);
		nl4p_lub++;
		if (nl4p_lub == 4)
			nl4p_lub = 0;
		btu = nl4p_bpos>>4;
	}
}
*/
void nl4p_pdet(uint16_t pot)
{
	lforam.nl4p_detune = pot;
}
void nl4p_penv(uint16_t pot)
{
	lforam.nl4p_attack = glinc[pot>>4];
	lforam.nl4p_release = glinc[pot>>4];
}
void nl4p_pwf(uint16_t pot)
{
	lforam.nl4p_waveform = pot;
}
void nl4p_prel(uint16_t pot)
{
	lforam.nl4p_release = glinc[pot>>4];
}

void nl4p_cr()
{
	lforam.nl4p_keysyncsate = (lforam.nl4p_keysyncsate<<1) + (parameters.lfokeysync & 1);
	if ((lforam.nl4p_keysyncsate & 0x3) == 2)
	{
		knob_lock(KNOB_LFOSHAPE,0,1,nl4p_pwf);
		knob_lock(KNOB_LFORATE,0,1,nl4p_prel);
	}
	if ((lforam.nl4p_keysyncsate & 0x3) == 1)
	{
		knob_lock(KNOB_LFOSHAPE,0,1,nl4p_pdet);
		knob_lock(KNOB_LFORATE,0,1,nl4p_penv);
	}


	if (knob_status[KNOB_LFORATE].cbk == knob_na) //
	{
		if (parameters.lfokeysync)
		{
			knob_lock(KNOB_LFOSHAPE,0,0,nl4p_pdet);
			knob_lock(KNOB_LFORATE,0,0,nl4p_penv);
		}
		else
		{
			knob_lock(KNOB_LFOSHAPE,0,1,nl4p_pwf);
			knob_lock(KNOB_LFORATE,0,1,nl4p_prel);
		}
	}

	uint8_t btu = lforam.nl4p_bpos>>4;
	while (btu != lforam.nl4p_lub)
	{
		nl4p_slotprocess(&lforam.nl4p_buffer[lforam.nl4p_lub<<4]);
		lforam.nl4p_lub++;
		if (lforam.nl4p_lub == 4)
			lforam.nl4p_lub = 0;
		btu = lforam.nl4p_bpos>>4;
	}
}




void nl4p_setfrq(uint8_t voice, uint8_t note, uint32_t ts);


void nl4p_test() // can be deleted
{

	NVIC_DisableIRQ(TIM6_DAC_IRQn);
	NVIC_DisableIRQ(TIM7_IRQn);


	nl4p_setfrq(0,126,0);
	nl4p_setfrq(1,126,0);
	nl4p_setfrq(2,126,0);
	nl4p_setfrq(4,126,0);
	nl4p_setfrq(5,126,0);
	//if lid than a+r and detune else r only and wave
	lforam.nl4p_detune = 0x820;
	lforam.nl4p_attack = glinc[1];
	lforam.nl4p_release = glinc[0x7];
	lforam.nl4p_waveform = 0x3FF;


	//init
	/*
	nl4p_r_amp = 0x1800;
	nl4p_f_amp = 0x1800;
	nl4p_attack = 0x002C5772;
	nl4p_release = 0x002C5772;
	nl4p_f_com = 1;
	*/
	uint8_t i;
	for (i = 0; i<5; i++)
	{
		//
		lforam.nl4p_voices[i].r_phase = 0;
		lforam.nl4p_voices[i].f_phase = 0;
		lforam.nl4p_voices[i].amplitude = 0;
	}
	for(i = 0; i<32; i++)
	{
		uint32_t startpos = TIM2->CNT;
		nl4p_slotprocess(&lforam.nl4p_buffer[0]);
		lforam.nl4p_timed[i] = TIM2->CNT - startpos;
	}
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	NVIC_EnableIRQ(TIM7_IRQn);
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
/*
пока не забыл:
-запилить нормальный аллокатор. простой циклический перегружает рабочие голоса
- проверить микширование голосов. должно быть норм, но хз.
- доделать треугольный шум уже наконец. прогнать реактором
- проверить огибающую, если что переделать с цээр на эсэр, такты еще есть
- вроде все
из остальных задач - найти взаимосв€зь кривого пича с чем-нибудь
ну как то странно кос€чит на конкретных нотах.
-цапы
-осцилл€тор

сделатть 32бит тоталприорити счетчик который на каждую ноту инкрементируетс€ - вместо 24 часов 4 бил€ нотных событий
*/


void nl4p_setfrq(uint8_t voice, uint8_t note, uint32_t ts)
{
	lforam.nl4p_voices[voice].priority = ts;
	lforam.nl4p_voices[voice].note = note;
	lforam.nl4p_voices[voice].to_release = 0;
	nl4p_uvd(voice);
	lforam.nl4p_voices[voice].active = VOICE_ACTIVE;
	if (parameters.gateretrig)
		lforam.nl4p_voices[voice].amplitude = 0;
}

void nl4p_noteon(uint8_t note, uint32_t ts) //it is possible to catch failure every 24 hours 51 minutes and 18 seconds. good luck!
{
	uint8_t i;

	for (i=0; i<5; i++)
	{
		if (note == lforam.nl4p_voices[i].note)
		{
			nl4p_setfrq(i,note, ts);
			return;
		}
	}
	uint32_t av_minpr = 0xFFFFFFFF;
	uint32_t iv_minpr = 0xFFFFFFFF;
	uint8_t avn = 0xFF;
	uint8_t ivn = 0xFF;
	for (i=0; i<5; i++)
	{
		if (lforam.nl4p_voices[i].active)
		{
			if (lforam.nl4p_voices[i].priority < av_minpr)
			{
				av_minpr = lforam.nl4p_voices[i].priority;
				avn = i;
			}
		}
		else
		{
			if (lforam.nl4p_voices[i].priority < iv_minpr)
			{
				iv_minpr = lforam.nl4p_voices[i].priority;
				ivn = i;
			}
		}
	}
	if (ivn == 0xFF)
		nl4p_setfrq(avn, note, ts);
	else
		nl4p_setfrq(ivn, note, ts);
}

void nl4p_noteoff(uint8_t note, uint32_t ts)
{
	uint8_t i;
	for (i=0; i<5; i++)
	{
		if (note == lforam.nl4p_voices[i].note)
		{
			if (lforam.nl4p_damperstate)
			{
				lforam.nl4p_voices[i].to_release = 1;
			}
			else
			{
				lforam.nl4p_voices[i].priority = ts;
				lforam.nl4p_voices[i].active = 0;
			}
		}
	}

}
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
void nl4p_damper(uint8_t state, uint32_t ts)
{
	if (state)
		lforam.nl4p_damperstate = 1;
	else
	{
		lforam.nl4p_damperstate = 0;
		uint8_t i;
		for (i=0; i<5; i++)
		{
			if (lforam.nl4p_voices[i].to_release)
			{
				lforam.nl4p_voices[i].priority = ts;
				lforam.nl4p_voices[i].to_release = 0;
				lforam.nl4p_voices[i].active = 0;
			}
		}
	}
}
void nl4p_panic()
{
	uint8_t i;
	for (i=0; i<5; i++)
	{
		lforam.nl4p_voices[i].active = 0;
		lforam.nl4p_voices[i].amplitude = 0;
		lforam.nl4p_voices[i].priority = 0;
	}
}

void nl4p_init(uint8_t prev)
{
	NVIC_DisableIRQ(EXTI2_3_IRQn);
	if(lforam.nl4p_attack > 0x002C5772)
		lforam.nl4p_attack = 0x002C5772;
	if(lforam.nl4p_release > 0x002C5772)
		lforam.nl4p_release = 0x002C5772;
	lforam.nl4p_bpos = 0;
	lforam.nl4p_lub = 0;
	lforam.nl4p_upd_cv = 0;
	lforam.nl4p_keysyncsate = 0;
	nl4p_panic();
	(void)prev;
}

const neutr_lfo_t nl_4poly =
{
	0,               //void (*retrig)(void);
	//0,               //void (*midistart)(void);
	//0,               //void (*midiclock)(void);
	nl4p_cr,         //void (*cr)(void);
	nl4p_sr,         //void (*sr)(void);
	nl4p_lfoleds,    //void (*lfoleds)(uint32_t time);
	nl4p_noteon,     //void (*noteon)(uint8_t note, uint32_t ts);
	nl4p_noteoff,    //void (*noteoff)(uint8_t note, uint32_t ts);
	nl4p_damper,     //void (*damper)(uint8_t val, uint32_t ts);
	nl4p_panic,      //void (*panic)(void);
	//0,               //void (*irqsync)(uint8_t type);
	nl4p_async,            //void (*asyncmain)(void);
	nl4p_init       //void (*initialize)(uint8_t prevmode);
};
