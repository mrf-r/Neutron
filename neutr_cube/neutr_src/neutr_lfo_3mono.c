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
 * neutr_lfo_3mono.c
 *
 *  Created on: Apr 17, 2019
 *
*/

/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

 THIS IS DEPRECATED
 GOTO 13ex file for actual version

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *
 *
 *      TODO: add "DO NOT TOUCH" sign to 13ex
 *
 *slava: кстати чтобы отлаживаться нормально попробуй ключ компилятора -g3 это для того чтоб ты все макросы в отладке мог видеть
 *
 *	please, do not write into aa code. tri has 17 bit shift that is equal to normalization and division by two at same time
 *
 * -E to see preprocessed
 *
 */


#include "neutr_lfo_common.h"
#include "neutron.h"
#include "menu.h"

uint32_t nl3m_sintri_as_async();
uint32_t nl3m_sintri_as_esris();
uint32_t nl3m_sintri_as_esfal();
uint32_t nl3m_sintri_as_esrst();
uint32_t nl3m_trisaw_as_async();
uint32_t nl3m_trisaw_as_esris();
uint32_t nl3m_trisaw_as_esfal();
uint32_t nl3m_trisaw_as_esrst();
uint32_t nl3m_sawsqr_as_async();
uint32_t nl3m_sawsqr_as_esris();
uint32_t nl3m_sawsqr_as_esfal();
uint32_t nl3m_sawsqr_as_esrst();
uint32_t nl3m_sqrpwm_as_async();
uint32_t nl3m_sqrpwm_as_esris();
uint32_t nl3m_sqrpwm_as_esfal();
uint32_t nl3m_sqrpwm_as_esrst();
uint32_t nl3m_sintri_ms_async();
uint32_t nl3m_sintri_ms_esris();
uint32_t nl3m_sintri_ms_esfal();
uint32_t nl3m_sintri_ms_esrst();
uint32_t nl3m_trisaw_ms_async();
uint32_t nl3m_trisaw_ms_esris();
uint32_t nl3m_trisaw_ms_esfal();
uint32_t nl3m_trisaw_ms_esrst();
uint32_t nl3m_sawsqr_ms_async();
uint32_t nl3m_sawsqr_ms_esris();
uint32_t nl3m_sawsqr_ms_esfal();
uint32_t nl3m_sawsqr_ms_esrst();
uint32_t nl3m_sqrpwm_ms_async();
uint32_t nl3m_sqrpwm_ms_esris();
uint32_t nl3m_sqrpwm_ms_esfal();
uint32_t nl3m_sqrpwm_ms_esrst();

uint32_t (*const nl3m_srproc[32])(void) =
{
	nl3m_sintri_as_async,
	nl3m_sintri_as_esris,
	nl3m_sintri_as_esfal,
	nl3m_sintri_as_esrst,
	nl3m_trisaw_as_async,
	nl3m_trisaw_as_esris,
	nl3m_trisaw_as_esfal,
	nl3m_trisaw_as_esrst,
	nl3m_sawsqr_as_async,
	nl3m_sawsqr_as_esris,
	nl3m_sawsqr_as_esfal,
	nl3m_sawsqr_as_esrst,
	nl3m_sqrpwm_as_async,
	nl3m_sqrpwm_as_esris,
	nl3m_sqrpwm_as_esfal,
	nl3m_sqrpwm_as_esrst,
	nl3m_sintri_ms_async,
	nl3m_sintri_ms_esris,
	nl3m_sintri_ms_esfal,
	nl3m_sintri_ms_esrst,
	nl3m_trisaw_ms_async,
	nl3m_trisaw_ms_esris,
	nl3m_trisaw_ms_esfal,
	nl3m_trisaw_ms_esrst,
	nl3m_sawsqr_ms_async,
	nl3m_sawsqr_ms_esris,
	nl3m_sawsqr_ms_esfal,
	nl3m_sawsqr_ms_esrst,
	nl3m_sqrpwm_ms_async,
	nl3m_sqrpwm_ms_esris,
	nl3m_sqrpwm_ms_esfal,
	nl3m_sqrpwm_ms_esrst
};

extern volatile uint32_t pp_o2p;
extern volatile int32_t pp_o2pwshift;


/*
//subsample position
uint32_t nl3m_esrstsspos;

//0 or 180 phase shift if rise or fall
uint32_t nl3m_esrstssphaseadd;

//phase and both osc's wave positions at the sync moment
uint32_t nl3m_esrstsswave1;
uint32_t nl3m_esrstsswave2;
uint32_t nl3m_esrstssphase;

volatile int32_t nl3m_pitch;
volatile uint32_t nl3m_divr;

volatile uint32_t nl3m_ztimestamp = 0;
volatile uint16_t nl3m_mclkdelta = 0;
volatile uint32_t nl3m_mclkphase;

*/

#define CODEBLOCK_START_AS                                                                                       \
	uint16_t tablepos = lforate>>4;                                                                        \
	uint8_t subpos = (uint8_t)(lforate<<4);                                                                \
	uint32_t inc = (uint32_t)((lfoinc[tablepos+1].inc - lfoinc[tablepos].inc)>>8)*subpos+lfoinc[tablepos].inc;          \
	uint16_t shift =  lfoinc[tablepos].shift;                                                                           \
	uint8_t is = lfoinc[tablepos+1].shift-lfoinc[tablepos].shift;                                                       \
	uint16_t ni = lfoinc[tablepos+1].recp>>is;                                                                          \
	uint16_t recp = lfoinc[tablepos].recp - ((uint32_t)(lfoinc[tablepos].recp - ni)*subpos>>8);                         \
	uint32_t out = 0;                                                                                                   \
	nl1c_lfoacc += inc;                                                                                                 \
	uint32_t pp = nl1c_lfoacc + parameters.lfophaseshift;

#define CODEBLOCK_START_NS                                                                                       \
	int32_t pitch = lforate*(3+7*parameters.lfokeysync) + lforam.nl3m_pitch;                                                                \
	if (pitch<0)                                                                                                        \
		pitch = 0;                                                                                                      \
	else                                                                                                                \
		if (pitch > 0xFFFF)                                                                                             \
			pitch = 0xFFFF;                                                                                             \
	uint16_t tablepos = pitch>>8;                                                                                       \
	uint8_t subpos = (uint8_t)pitch;                                                                                    \
	uint32_t inc = (uint32_t)((oscinc[tablepos+1].inc - oscinc[tablepos].inc)>>8)*subpos+oscinc[tablepos].inc;          \
	uint16_t shift =  oscinc[tablepos].shift;                                                                           \
	uint8_t is = oscinc[tablepos+1].shift-oscinc[tablepos].shift;                                                       \
	uint16_t ni = oscinc[tablepos+1].recp>>is;                                                                          \
	uint16_t recp = oscinc[tablepos].recp - ((uint32_t)(oscinc[tablepos].recp - ni)*subpos>>8);                         \
	uint32_t out = 0;                                                                                                   \
	lforam.phaseacc += inc;                                                                                                 \
	uint32_t pp = lforam.phaseacc + lforam.phaseshift;
/*
#define CODEBLOCK_START_ASES                                                                                       \
	uint16_t tablepos = lforate>>4;                                                                        \
	uint8_t subpos = (uint8_t)(lforate<<4);                                                                \
	uint32_t inc = (uint32_t)((lfoinc[tablepos+1].inc - lfoinc[tablepos].inc)>>8)*subpos+lfoinc[tablepos].inc;          \
	uint16_t shift =  lfoinc[tablepos].shift;                                                                           \
	uint32_t out = 0;                                                                                                   \
	nl1c_lfoacc += inc;                                                                                                 \
	uint32_t pp = nl1c_lfoacc;
	*/
#define CODEBLOCK_START_MS                                                                                         \
	uint32_t inc = lforam.ms_inc;                                                                                            \
	uint16_t shift = lforam.ms_shift;                                                                                        \
	uint16_t recp = lforam.ms_recp;                                                                                          \
	uint32_t out = 0;                                                                                                   \
	lforam.phaseacc += inc;                                                                                                 \
	uint32_t pp = lforam.phaseacc + (parameters.lfosyncstart<<16) + lforam.phaseshift;



#define CODEBLOCK_SYNCRISE                                                                                         \
	uint32_t sscf = (lforam.irqpos * 0x20C49B)>>15;                                                                       \
	lforam.subsamplepos = sscf;                                                                                             \
	uint32_t ssnextphase = (((inc>>shift)*sscf)>>(16-shift)) + (parameters.lfosyncstart);                           \
	sscf = (uint16_t)(-sscf);                                                                                           \
	sscf = (sscf * sscf)>>16;                                                                                           \
	lforam.prevphase = pp;                                                                                             \
	lforam.nextphaseflip = 0;                                                                                           \
	lforam.irqstate = 3;
#define CODEBLOCK_SYNCFALL                                                                                         \
	uint32_t sscf = (lforam.irqpos * 0x20C49B)>>15;                                                                       \
	lforam.subsamplepos = sscf;                                                                                             \
	uint32_t ssnextphase = (((inc>>shift)*sscf)>>(16-shift)) + (parameters.lfosyncstart) + 0x80000000;              \
	sscf = (uint16_t)(-sscf);                                                                                           \
	sscf = (sscf * sscf)>>16;                                                                                           \
	lforam.prevphase = pp;                                                                                             \
	lforam.nextphaseflip = 0x80000000;                                                                                  \
	lforam.irqstate = 3;
#define CODEBLOCK_SYNCRESET                                                                                        \
	uint32_t sscf = lforam.subsamplepos;                                                                                    \
	lforam.phaseacc = (((inc>>shift)*sscf)>>(16-shift)) + (parameters.lfosyncstart) + lforam.nextphaseflip;             \
	sscf = (sscf * sscf)>>16;                                                                                           \
	lforam.irqstate = 0;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sine

#define CODEBLOCK_ASO_SINE                                \
	{                                                          \
	uint8_t tpos = pp>>24;                                     \
	uint8_t spos = (uint8_t)(pp>>16);                          \
	int16_t delta = sine[tpos + 1] - sine[tpos];               \
	out += (sine[tpos] + ((delta * spos)>>8))*woutmul;         \
	}

#define CODEBLOCK_ES1_SINE(wsave)                           \
	{                                                          \
	uint32_t siner;                                            \
	{                                                          \
		uint8_t tpos = pp>>24;                                 \
		uint8_t spos = (uint8_t)(pp>>16);                      \
		int16_t delta = sine[tpos + 1] - sine[tpos];           \
		siner = sine[tpos] + ((delta * spos)>>8);              \
	}                                                          \
	uint32_t sinernxt;                                         \
	{                                                          \
		uint8_t tpos = ssnextphase>>24;                        \
		uint8_t spos = (uint8_t)(ssnextphase>>16);             \
		int16_t delta = sine[tpos + 1] - sine[tpos];           \
		sinernxt = sine[tpos] + ((delta * spos)>>8);           \
	}                                                          \
	int32_t sinedelta = (sinernxt - siner)/2;                  \
	wsave = siner;                                             \
	if ((pp>>31) == (ssnextphase>>31))                         \
	{                                                          \
		siner += sscf * sinedelta;                     \
	}                                                          \
	else                                                       \
	{                                                          \
		if (pp>>31)                                            \
		{                                                      \
			siner += (sscf * (sinedelta + (inc>>17)))/65536;   \
		}                                                      \
		else                                                   \
		{                                                      \
			siner += (sscf * (sinedelta - (inc>>17)))/65536;   \
		}                                                      \
	}                                                          \
	out += siner * woutmul;                                    \
	}

#define CODEBLOCK_ES2_SINE(wsave)                           \
	{                                                          \
	uint32_t siner;                                            \
	uint8_t tpos = pp>>24;                                     \
	uint8_t spos = (uint8_t)(pp>>16);                          \
	int16_t delta = sine[tpos + 1] - sine[tpos];               \
	siner = sine[tpos] + ((delta * spos)>>8);                  \
	int32_t sinedelta = (wsave - siner)/2;                     \
	if ((pp>>31) == (lforam.prevphase>>31))                         \
	{                                                          \
		siner += sscf * sinedelta;                             \
	}                                                          \
	else                                                       \
	{                                                          \
		if (pp>>31)                                            \
		{                                                      \
			siner += (sscf * (sinedelta - (inc>>17)))/65536;   \
		}                                                      \
		else                                                   \
		{                                                      \
			siner += (sscf * (sinedelta + (inc>>17)))/65536;   \
		}                                                      \
	}                                                          \
	out += siner * woutmul;                                    \
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// triangle

#define CODEBLOCK_ASO_TRIANGLE                            \
	{                                                          \
	uint32_t trir;                                             \
	if(pp>>31)                                                 \
		trir = ~pp;                                            \
	else                                                       \
		trir = pp;                                             \
	trir = trir<<1;                                            \
	if (trir>(~inc))                                           \
	{                                                          \
		uint32_t delta = (trir + inc)>>shift;                  \
		uint32_t rev = (delta * recp)>>13;                     \
		rev = (rev*rev)>>17;                                   \
		trir = trir - (rev * (inc>>16));                       \
	}                                                          \
	else                                                       \
	{                                                          \
		if (trir<inc)                                          \
		{                                                      \
			uint32_t delta = (inc - trir)>>shift;              \
			uint32_t rev = (delta * recp)>>13;                 \
			rev = (rev*rev)>>17;                               \
			trir = trir + (rev * (inc>>16));                   \
		}                                                      \
	}                                                          \
	out += (trir>>16)*woutmul;                                  \
	}

#define CODEBLOCK_ES1_TRIANGLE(wsave)                       \
	{                                                          \
	uint32_t trir;                                             \
	if(pp>>31)                                                 \
		trir = ~pp;                                            \
	else                                                       \
		trir = pp;                                             \
	trir = trir<<1;                                            \
	uint32_t trirnxt;                                          \
	if(ssnextphase>>31)                                        \
		trirnxt = ~ssnextphase;                                \
	else                                                       \
		trirnxt = ssnextphase;                                 \
	trirnxt = trirnxt<<1;                                      \
	int32_t tridelta = (trirnxt>>17) - (trir>>17);             \
	wsave = trir;                                              \
	if ((pp>>31) == (ssnextphase>>31))                         \
	{                                                          \
		trir += sscf * tridelta;                               \
	}                                                          \
	else                                                       \
	{                                                          \
		if (pp>>31)                                            \
		{                                                      \
			trir += sscf * (tridelta + (inc>>17));             \
		}                                                      \
		else                                                   \
		{                                                      \
			trir += sscf * (tridelta - (inc>>17));             \
		}                                                      \
	}                                                          \
	out += (trir>>16)*woutmul;                                  \
	}                                                           \
	(void)recp;


#define CODEBLOCK_ES2_TRIANGLE(wsave)                       \
	{                                                          \
	uint32_t trir;                                             \
	if(pp>>31)                                                 \
		trir = ~pp;                                            \
	else                                                       \
		trir = pp;                                             \
	trir = trir<<1;                                            \
	int32_t tridelta = (wsave>>17) - (trir>>17);               \
	if ((pp>>31) == (lforam.prevphase>>31))                         \
	{                                                          \
		trir += sscf * tridelta;                               \
	}                                                          \
	else                                                       \
	{                                                          \
		if (pp>>31)                                            \
		{                                                      \
			trir += sscf * (tridelta - (inc>>17));             \
		}                                                      \
		else                                                   \
		{                                                      \
			trir += sscf * (tridelta + (inc>>17));             \
		}                                                      \
	}                                                          \
	out += (trir>>16)*woutmul;                                  \
	}                                                           \
	(void)recp;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// saw down

#define CODEBLOCK_ASO_SAWDWN                              \
	{                                                          \
	uint32_t saw2r = ~pp;                                      \
	if (saw2r>(~inc))                                          \
	{                                                          \
		uint32_t delta = (saw2r + inc)>>shift;                 \
		uint32_t rev = (delta * recp)>>13;                     \
		rev = rev*rev;                                         \
		saw2r -= rev;                                          \
	}                                                          \
	else                                                       \
	{                                                          \
		if (saw2r<inc)                                         \
		{                                                      \
			uint32_t delta = (inc - saw2r)>>shift;             \
			uint32_t rev = (delta * recp)>>13;                 \
			rev = rev*rev;                                     \
			saw2r += rev;                                      \
		}                                                      \
	}                                                          \
	out += (saw2r>>16)*woutmul;                                 \
	}

#define CODEBLOCK_ES1_SAWDWN(wsave)                         \
	{                                                          \
	uint32_t saw2r = ~pp;                                      \
	uint32_t saw2nxt = ~ssnextphase;                           \
	int32_t saw2rdelta = (saw2nxt>>17) - (saw2r>>17);          \
	wsave = ~pp;                                               \
	saw2r += sscf * saw2rdelta;                                \
	out += (saw2r>>16)*woutmul;                                 \
	}                                                           \
	(void)recp;

#define CODEBLOCK_ES2_SAWDWN(wsave)                         \
	{                                                          \
	uint32_t saw2r = ~pp;                                      \
	int32_t saw2rdelta = (wsave>>17) - (saw2r>>17);            \
	saw2r += sscf * saw2rdelta;                                \
	out += (saw2r>>16)*woutmul;                                 \
	}                                                           \
	(void)recp;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// saw up

#define CODEBLOCK_ASO_SAWUP                               \
	{                                                          \
	uint32_t saw1r = pp + 0x80000000;                          \
	if (saw1r>(~inc))                                          \
	{                                                          \
		uint32_t delta = (saw1r + inc)>>shift;                 \
		uint32_t rev = (delta * recp)>>13;                     \
		rev = rev*rev;                                         \
		saw1r -= rev;                                          \
	}                                                          \
	else                                                       \
	{                                                          \
		if (saw1r<inc)                                         \
		{                                                      \
			uint32_t delta = (inc - saw1r)>>shift;             \
			uint32_t rev = (delta * recp)>>13;                 \
			rev = rev*rev;                                     \
			saw1r += rev;                                      \
		}                                                      \
	}                                                          \
	out += (saw1r>>16)*woutmul;                                 \
	}

#define CODEBLOCK_ES1_SAWUP(wsave)                          \
	{                                                          \
	uint32_t saw1r = pp + 0x80000000;                          \
	/*uint32_t saw1nxt = ssnextphase + 0x80000000;;      */        \
	int32_t saw1rdelta = (ssnextphase>>17) - (pp>>17);         \
	wsave = saw1r;                                             \
	saw1r += sscf * saw1rdelta;                                \
	out += (saw1r>>16)*woutmul;                                 \
	}                                                           \
	(void)recp;

#define CODEBLOCK_ES2_SAWUP(wsave)                          \
	{                                                          \
	uint32_t saw1r = pp + 0x80000000;                          \
	int32_t saw1rdelta = (wsave>>17) - (saw1r>>17);            \
	saw1r += sscf * saw1rdelta;                                \
	out += (saw1r>>16)*woutmul;                                 \
	}                                                           \
	(void)recp;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pwm
#define CODEBLOCK_ASO_SAWDPWM                             \
	{                                                          \
	uint32_t saw2r = ~pp;                                      \
	if (saw2r>(~inc))                                          \
	{                                                          \
		uint32_t delta = (saw2r + inc)>>shift;                 \
		uint32_t rev = (delta * recp)>>13;                     \
		rev = rev*rev;                                         \
		saw2r -= rev;                                          \
	}                                                          \
	else                                                       \
	{                                                          \
		if (saw2r<inc)                                         \
		{                                                      \
			uint32_t delta = (inc - saw2r)>>shift;             \
			uint32_t rev = (delta * recp)>>13;                 \
			rev = rev*rev;                                     \
			saw2r += rev;                                      \
		}                                                      \
	}                                                          \
	out += (saw2r>>16)*0x7FFF;                                  \
	}

#define CODEBLOCK_ES1_SAWDPWM(wsave)                        \
	{                                                          \
	uint32_t saw2r = ~pp;                                      \
	uint32_t saw2nxt = ~ssnextphase;                           \
	int32_t saw2rdelta = (saw2nxt>>17) - (saw2r>>17);          \
	wsave = ~pp;                                               \
	saw2r += sscf * saw2rdelta;                                \
	out += (saw2r>>16)*0x7FFF;                                  \
	}                                                           \
	(void)recp;

#define CODEBLOCK_ES2_SAWDPWM(wsave)                        \
	{                                                          \
	uint32_t saw2r = ~pp;                                      \
	int32_t saw2rdelta = (wsave>>17) - (saw2r>>17);            \
	saw2r += sscf * saw2rdelta;                                \
	out += (saw2r>>16)*0x7FFF;                                  \
	}                                                           \
	(void)recp;

#define CODEBLOCK_ASO_SAWUPWM                             \
	{                                                          \
	uint32_t saw1r = pp + (woutmul<<15);                       \
	if (saw1r>(~inc))                                          \
	{                                                          \
		uint32_t delta = (saw1r + inc)>>shift;                 \
		uint32_t rev = (delta * recp)>>13;                     \
		rev = rev*rev;                                         \
		saw1r -= rev;                                          \
	}                                                          \
	else                                                       \
	{                                                          \
		if (saw1r<inc)                                         \
		{                                                      \
			uint32_t delta = (inc - saw1r)>>shift;             \
			uint32_t rev = (delta * recp)>>13;                 \
			rev = rev*rev;                                     \
			saw1r += rev;                                      \
		}                                                      \
	}                                                          \
	out += (saw1r>>16)*0x7FFF;                                  \
	}

#define CODEBLOCK_ES1_SAWUPWM(wsave)                        \
	{                                                          \
	uint32_t saw1r = pp + (woutmul<<15);                       \
	/*int32_t saw1nxt = ssnextphase + 0x80000000;; */             \
	int32_t saw1rdelta = (ssnextphase>>17) - (pp>>17);         \
	wsave = saw1r;                                             \
	saw1r += sscf * saw1rdelta;                                \
	out += (saw1r>>16)*0x7FFF;                                  \
	}                                                           \
	(void)recp;

#define CODEBLOCK_ES2_SAWUPWM(wsave)                        \
	{                                                          \
	uint32_t saw1r = pp + (woutmul<<15);                       \
	int32_t saw1rdelta = (wsave>>17) - (saw1r>>17);            \
	saw1r += sscf * saw1rdelta;                                \
	out += (saw1r>>16)*0x7FFF;                                  \
	}                                                           \
	(void)recp;

/*
#define CODEBLOCK_WSUM(cbwone, cbwtwo) \
		uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F]; \
		cbwone                                                \
		woutmul = 0xFFFF - woutmul;                           \
		cbwtwo                                                \
		return out;

#define CODEBLOCK_WSWITCH(cbwone, cbwtwo) \
		uint32_t woutmul = 0xFFFF                             \
		cbwone                                                \
		woutmul = 0;                           \
		cbwtwo                                                \
		return out; //keeping calculation load almost same??

#define CODEBLOCK_WSUM(waveone, wavetwo, synctype)                          \
		uint32_t nlex_ ## waveone ## wavetwo ## _async_ ## synctype()       \
		{                                                                   \
		uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];               \
		CODEBLOCK_ASO_ ## waveone                                           \
		woutmul = 0xFFFF - woutmul;                                         \
		CODEBLOCK_START_ ## synctype                                        \
		CODEBLOCK_ASO_ ## wavetwo                                           \
		return out;                                                         \
		}                                                                   \
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// END

uint32_t nl3m_sintri_as_async()
{
	CODEBLOCK_START_NS
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ASO_SINE
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ASO_TRIANGLE
	return out;
}
uint32_t nl3m_sintri_as_esris()
{
	CODEBLOCK_START_NS
	CODEBLOCK_SYNCRISE
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES1_SINE(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES1_TRIANGLE(lforam.wsave_two)
	return out;

}
uint32_t nl3m_sintri_as_esfal()
{
	CODEBLOCK_START_NS
	CODEBLOCK_SYNCFALL
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES1_SINE(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES1_TRIANGLE(lforam.wsave_two)
	return out;

}
uint32_t nl3m_sintri_as_esrst()
{
	CODEBLOCK_START_NS
	CODEBLOCK_SYNCRESET
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES2_SINE(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES2_TRIANGLE(lforam.wsave_two)
	return out;

}
uint32_t nl3m_trisaw_as_async()
{
	CODEBLOCK_START_NS

	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ASO_TRIANGLE
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ASO_SAWDWN
	return out;
}
uint32_t nl3m_trisaw_as_esris()
{
	CODEBLOCK_START_NS
	CODEBLOCK_SYNCRISE
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES1_TRIANGLE(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES1_SAWDWN(lforam.wsave_two)
	return out;
}
uint32_t nl3m_trisaw_as_esfal()
{
	CODEBLOCK_START_NS
	CODEBLOCK_SYNCFALL
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES1_TRIANGLE(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES1_SAWDWN(lforam.wsave_two)
	return out;
}
uint32_t nl3m_trisaw_as_esrst()
{
	CODEBLOCK_START_NS
	CODEBLOCK_SYNCRESET
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES2_TRIANGLE(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES2_SAWDWN(lforam.wsave_two)
	return out;
}

uint32_t nl3m_sawsqr_as_async()
{
	CODEBLOCK_START_NS

	uint32_t woutmul = 0xFFFF - (sine[(lfoshape>>3)&0x7F]>>1);
	CODEBLOCK_ASO_SAWDWN
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ASO_SAWUP
	return out;
}
uint32_t nl3m_sawsqr_as_esris()
{
	CODEBLOCK_START_NS
	CODEBLOCK_SYNCRISE
	uint32_t woutmul = 0xFFFF - (sine[(lfoshape>>3)&0x7F]>>1);
	CODEBLOCK_ES1_SAWDWN(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES1_SAWUP(lforam.wsave_two)
	return out;
}
uint32_t nl3m_sawsqr_as_esfal()
{
	CODEBLOCK_START_NS
	CODEBLOCK_SYNCFALL
	uint32_t woutmul = 0xFFFF - (sine[(lfoshape>>3)&0x7F]>>1);
	CODEBLOCK_ES1_SAWDWN(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES1_SAWUP(lforam.wsave_two)
	return out;
}
uint32_t nl3m_sawsqr_as_esrst()
{
	CODEBLOCK_START_NS
	CODEBLOCK_SYNCRESET
	uint32_t woutmul = 0xFFFF - (sine[(lfoshape>>3)&0x7F]>>1);
	CODEBLOCK_ES2_SAWDWN(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES2_SAWUP(lforam.wsave_two)
	return out;
}

uint32_t nl3m_sqrpwm_as_async()
{
	CODEBLOCK_START_NS

	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ASO_SAWDPWM
	CODEBLOCK_ASO_SAWUPWM
	return out;
}
uint32_t nl3m_sqrpwm_as_esris()
{
	CODEBLOCK_START_NS
	CODEBLOCK_SYNCRISE
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES1_SAWDPWM(lforam.wsave_one)
	CODEBLOCK_ES1_SAWUPWM(lforam.wsave_two)
	return out;
}
uint32_t nl3m_sqrpwm_as_esfal()
{
	CODEBLOCK_START_NS
	CODEBLOCK_SYNCFALL
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES1_SAWDPWM(lforam.wsave_one)
	CODEBLOCK_ES1_SAWUPWM(lforam.wsave_two)
	return out;
}
uint32_t nl3m_sqrpwm_as_esrst()
{
	CODEBLOCK_START_NS
	CODEBLOCK_SYNCRESET
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES2_SAWDPWM(lforam.wsave_one)
	CODEBLOCK_ES2_SAWUPWM(lforam.wsave_two)
	return out;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t nl3m_sintri_ms_async()
{
	CODEBLOCK_START_MS

	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ASO_SINE
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ASO_TRIANGLE
	return out;
}
uint32_t nl3m_sintri_ms_esris()
{
	CODEBLOCK_START_MS
	CODEBLOCK_SYNCRISE
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES1_SINE(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES1_TRIANGLE(lforam.wsave_two)
	return out;
}
uint32_t nl3m_sintri_ms_esfal()
{
	CODEBLOCK_START_MS
	CODEBLOCK_SYNCFALL
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES1_SINE(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES1_TRIANGLE(lforam.wsave_two)
	return out;
}
uint32_t nl3m_sintri_ms_esrst()
{
	CODEBLOCK_START_MS
	CODEBLOCK_SYNCRESET
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES2_SINE(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES2_TRIANGLE(lforam.wsave_two)
	return out;
}
uint32_t nl3m_trisaw_ms_async()
{
	CODEBLOCK_START_MS

	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ASO_TRIANGLE
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ASO_SAWDWN
	return out;
}
uint32_t nl3m_trisaw_ms_esris()
{
	CODEBLOCK_START_MS
	CODEBLOCK_SYNCRISE
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES1_TRIANGLE(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES1_SAWDWN(lforam.wsave_two)
	return out;
}
uint32_t nl3m_trisaw_ms_esfal()
{
	CODEBLOCK_START_MS
	CODEBLOCK_SYNCFALL
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES1_TRIANGLE(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES1_SAWDWN(lforam.wsave_two)
	return out;
}
uint32_t nl3m_trisaw_ms_esrst()
{
	CODEBLOCK_START_MS
	CODEBLOCK_SYNCRESET
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES2_TRIANGLE(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES2_SAWDWN(lforam.wsave_two)
	return out;
}
uint32_t nl3m_sawsqr_ms_async()
{
	CODEBLOCK_START_MS

	uint32_t woutmul = 0xFFFF - (sine[(lfoshape>>3)&0x7F]>>1);
	CODEBLOCK_ASO_SAWDWN
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ASO_SAWUP
	return out;
}
uint32_t nl3m_sawsqr_ms_esris()
{
	CODEBLOCK_START_MS
	CODEBLOCK_SYNCRISE
	uint32_t woutmul = 0xFFFF - (sine[(lfoshape>>3)&0x7F]>>1);
	CODEBLOCK_ES1_SAWDWN(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES1_SAWUP(lforam.wsave_two)
	return out;
}
uint32_t nl3m_sawsqr_ms_esfal()
{
	CODEBLOCK_START_MS
	CODEBLOCK_SYNCFALL
	uint32_t woutmul = 0xFFFF - (sine[(lfoshape>>3)&0x7F]>>1);
	CODEBLOCK_ES1_SAWDWN(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES1_SAWUP(lforam.wsave_two)
	return out;
}
uint32_t nl3m_sawsqr_ms_esrst()
{
	CODEBLOCK_START_MS
	CODEBLOCK_SYNCRESET
	uint32_t woutmul = 0xFFFF - (sine[(lfoshape>>3)&0x7F]>>1);
	CODEBLOCK_ES2_SAWDWN(lforam.wsave_one)
	woutmul = 0xFFFF - woutmul;
	CODEBLOCK_ES2_SAWUP(lforam.wsave_two)
	return out;
}
uint32_t nl3m_sqrpwm_ms_async()
{
	CODEBLOCK_START_MS

	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ASO_SAWDPWM
	CODEBLOCK_ASO_SAWUPWM
	return out;
}
uint32_t nl3m_sqrpwm_ms_esris()
{
	CODEBLOCK_START_MS
	CODEBLOCK_SYNCRISE
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES1_SAWDPWM(lforam.wsave_one)
	CODEBLOCK_ES1_SAWUPWM(lforam.wsave_two)
	return out;
}
uint32_t nl3m_sqrpwm_ms_esfal()
{
	CODEBLOCK_START_MS
	CODEBLOCK_SYNCFALL
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES1_SAWDPWM(lforam.wsave_one)
	CODEBLOCK_ES1_SAWUPWM(lforam.wsave_two)
	return out;
}
uint32_t nl3m_sqrpwm_ms_esrst()
{
	CODEBLOCK_START_MS
	CODEBLOCK_SYNCRESET
	uint32_t woutmul = 0xFFFF - sine[(lfoshape>>3)&0x7F];
	CODEBLOCK_ES2_SAWDPWM(lforam.wsave_one)
	CODEBLOCK_ES2_SAWUPWM(lforam.wsave_two)
	return out;
}

void nl3m_sr()
{


	uint8_t wsel = lfoshape>>10;
	uint8_t wctype;
	if (lforam.irqstate == 3)
		wctype = (parameters.lfomidisync<<4) | (wsel<<2) | lforam.irqstate; //TODO improve this comparison
	else
		wctype = (parameters.lfomidisync<<4) | (wsel<<2) | (lforam.irqstate & (parameters.lfoextsynctype + 1));
	//nl3m_debug[nl3m_dp++] = wctype;
	uint32_t value = nl3m_srproc[wctype]();
	LFO_OUTPUT = value>>16;


	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   parameters.lfomidisync;
}


void nlcom_cr(void);

void nl3m_cr()
{
	lforam.nl3m_pitch = (pp_o2p>>15) + pp_o2pwshift - (0x1800 + 0x3800*parameters.lfokeysync);

	nlcom_cr();



	//i think it doesn't work, cause normal mode is pitch and sync mode is inc.
	//we shoult extend pitch scale and translate inc to pitch, but that is weird.
	//i like weird...

}


//extern uint8_t ledvals[5];
void nl3m_leds(uint32_t frame)
{
	//@TODO: reduce framerate by 256, add pwm for brightness control
	//calc lfo leds
	uint8_t pos = frame & 0x1F;
	uint8_t i;
	if (pos == 0) //
	{
		uint8_t lfo_shape[5] = {0,0,0,0,0};
		lfo_shape[lfoshape>>10] = (0x3FF - (lfoshape & 0x3FF))>>2;
		//set lfo leds
		for(i = 0; i < 5; i++)
		{
			lforam.ledvals[i] = lfo_shape[i]>>3;
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
void nlcom_async(void);

const neutr_lfo_t nl_3mono =
{
		0, //void (*retrig)(void);
		//nl3m_midistart,  //void (*midistart)(void);
		//nl3m_midiclock,  //void (*midiclock)(void);
		nl3m_cr,      //void (*cr)(void);
		nl3m_sr,     //void (*sr)(void);
		nl3m_leds,   //void (*lfoleds)(uint32_t time);
		0,           //void (*noteon)(uint8_t note, uint32_t ts);
		0,           //void (*noteoff)(uint8_t note, uint32_t ts);
		0,           //void (*damper)(uint8_t val, uint32_t ts);
		0,           //void (*panic)(void);
		//nl3m_irq,           //void (*irqsync)(uint8_t type);
		nlcom_async,            //void (*asyncmain)(void);
		0             //void (*initialize)(uint8_t prevmode);
};



