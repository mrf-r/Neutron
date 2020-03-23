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
 * neutr_lfo_13ex.c
 *
 *  Created on: Aug 13, 2019
 */

//that's the core - giant state machine
//main reason is response time

#include "neutr_lfo_common.h"
#include "neutron.h"
#include "menu.h"

volatile uint8_t comp2switch = 1;
volatile uint8_t compshift = 8;

/*
typedef struct
{
	uint32_t t2p;
	uint32_t tsp;
	uint32_t cp;
	uint32_t pdif;
	uint32_t irqpos;
	uint32_t ts;
}deb_fcc_t;

volatile deb_fcc_t deb_fcc[64];
volatile uint8_t deb_fcc_p;
#define CBLK_FCCW \
    deb_fcc[deb_fcc_p].cp       = cp;                 \
    deb_fcc[deb_fcc_p].irqpos   = lforam.irqpos;      \
    deb_fcc[deb_fcc_p].pdif     = pdif;               \
    deb_fcc[deb_fcc_p].t2p      = lforam.t2lp;        \
    deb_fcc[deb_fcc_p].ts       = timestamp;                \
    deb_fcc[deb_fcc_p].tsp      = tsp;                \
    deb_fcc_p = (deb_fcc_p + 1) & 0x3F;
*/








//we need this code to run on BOTH edges!!!!
//there is no periods or frequencies, just momentary intervals between this and prev
void lfo_process_sr_edge_process() //this function is just a copy.
{
	uint32_t tsp = timestamp - lforam.zts;
	lforam.zts = timestamp;
	if ((tsp < JC_TS_THRESHOLD))
	{
		uint32_t cp = tsp * 0x3E8 - lforam.irqpos + lforam.ztpos; //calculated period from SR
		int32_t pdif = cp - lforam.t2lp;

		// CBLK_FCCW

		if (pdif > JC_MAXDIF) //cp larger than actual by 1000 - not yet wrapped
		{	
			lforam.irqpos = 0;

			//comp
			exti_timcomp -= (0x3E8 - lforam.irqpos)>>1;
		}
		else
		{
			if (pdif < (-JC_MAXDIF)) //cp less than actual by 1000 and already overlapped
			{
				lforam.irqpos = 0x3E7;

				//compensation
				exti_timcomp += lforam.irqpos>>1;
			}
		}
	}
}
// #define CODEBLOCK_PURIFY_IRQPOS lfo_process_sr_edge_process();



#define CODEBLOCK_PURIFY_IRQPOS                                                                                          \
	uint32_t tsp = timestamp - lforam.zts;                                                                               \
	lforam.zts = timestamp;                                                                                              \
	if ((tsp < JC_TS_THRESHOLD))                                                                                         \
	{                                                                                                                    \
		uint32_t cp = tsp * 0x3E8 - lforam.irqpos + lforam.ztpos;                                                        \
		int32_t pdif = cp - lforam.t2lp;                                                                                 \
		if (pdif > JC_MAXDIF)                                                                                            \
		{	                                                                                                             \
			lforam.irqpos = 0x3E7;                                                                                           \
			exti_timcomp--;                                                       \
		}                                                                                                                \
		else                                                                                                             \
		{                                                                                                                \
			if (pdif < (-JC_MAXDIF))                                                                                     \
			{                                                                                                            \
				lforam.irqpos = 0;                                                                                   \
				exti_timcomp++;                                                       \
			}                                                                                                            \
		}                                                                                                                \
	}


#define CODEBLOCK_START_AS                                                                                              \
	uint16_t tablepos = lforate>>4;                                                                                     \
	uint8_t subpos = (uint8_t)(lforate<<4);                                                                             \
	uint32_t inc = (uint32_t)((lfoinc[tablepos+1].inc - lfoinc[tablepos].inc)>>8)*subpos+lfoinc[tablepos].inc;          \
	uint16_t shift =  lfoinc[tablepos].shift;                                                                           \
	uint8_t is = lfoinc[tablepos+1].shift-lfoinc[tablepos].shift;                                                       \
	uint16_t ni = lfoinc[tablepos+1].recp>>is;                                                                          \
	uint16_t recp = lfoinc[tablepos].recp - ((uint32_t)(lfoinc[tablepos].recp - ni)*subpos>>8);                         \
	recp = recp;                                                                                                        \
	uint32_t out = 0;                                                                                                   \
	lforam.phaseacc += inc;                                                                                             \
	uint32_t pp = lforam.phaseacc + lforam.phaseshift;
#define CODEBLOCK_START_NS                                                                                              \
	int32_t pitch = lforate*(3+7*parameters.lfokeysync) + lforam.nl3m_pitch;                                            \
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
	recp = recp;                                                                                                        \
	uint32_t out = 0;                                                                                                   \
	lforam.phaseacc += inc;                                                                                             \
	uint32_t pp = lforam.phaseacc + lforam.phaseshift;
#define CODEBLOCK_START_MS                                                                                              \
	uint32_t inc = lforam.ms_inc;                                                                                       \
	uint16_t shift = lforam.ms_shift;                                                                                   \
	uint16_t recp = lforam.ms_recp;                                                                                     \
	recp = recp;                                                                                                        \
	uint32_t out = 0;                                                                                                   \
	lforam.phaseacc += inc;                                                                                             \
	uint32_t pp = lforam.phaseacc + (parameters.lfosyncstart<<16) + lforam.phaseshift;
#define CODEBLOCK_SYNCRISE                                                                                              \
	CODEBLOCK_PURIFY_IRQPOS                                                                                             \
	uint32_t sscf = (lforam.irqpos * 0x20C49B)>>15;                                                                     \
	lforam.subsamplepos = sscf;                                                                                         \
	uint32_t ssnextphase = (((inc>>shift)*sscf)>>(16-shift)) + (parameters.lfosyncstart);                               \
	sscf = (sscf * sscf)>>16;                                                                                           \
	lforam.prevphase = pp;                                                                                              \
	lforam.nextphaseflip = 0;                                                                                           \
	lforam.irqstate = 3;
#define CODEBLOCK_SYNCFALL                                                                                              \
	CODEBLOCK_PURIFY_IRQPOS                                                                                             \
	uint32_t sscf = (lforam.irqpos * 0x20C49B)>>15;                                                                     \
	lforam.subsamplepos = sscf;                                                                                         \
	uint32_t ssnextphase = (((inc>>shift)*sscf)>>(16-shift)) + (parameters.lfosyncstart) + 0x80000000;                  \
	sscf = (sscf * sscf)>>16;                                                                                           \
	lforam.prevphase = pp;                                                                                              \
	lforam.nextphaseflip = 0x80000000;                                                                                  \
	lforam.irqstate = 3;
#define CODEBLOCK_SYNCRESET                                                                                             \
	uint32_t prevphase = lforam.prevphase;                                                                              \
	prevphase = prevphase;                                                                                              \
	uint32_t sscf = lforam.subsamplepos;                                                                                \
	lforam.phaseacc = (((inc>>shift)*sscf)>>(16-shift)) + (parameters.lfosyncstart) + lforam.nextphaseflip;             \
	sscf = (uint16_t)(~sscf);                                                                                           \
	sscf = (sscf * sscf)>>16;                                                                                           \
	pp = lforam.phaseacc + lforam.phaseshift;                                                                           \
	lforam.irqstate = 0;                                                                                                \
	NVIC_EnableIRQ(EXTI2_3_IRQn);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sine

#define CODEBLOCK_ASO_SIN(wsave)                               \
	{                                                          \
	uint8_t tpos = pp>>24;                                     \
	uint8_t spos = (uint8_t)(pp>>16);                          \
	int16_t delta = sine[tpos + 1] - sine[tpos];               \
	out += (sine[tpos] + ((delta * spos)>>8))*woutmul;         \
	}

#define CODEBLOCK_ES1_SIN(wsave)                               \
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
	int32_t sinedelta = (sinernxt>>1) - (siner>>1);            \
	siner = siner << 16;                                       \
	wsave = siner;                                             \
	if ((pp>>31) == (ssnextphase>>31))                         \
	{                                                          \
		siner += sscf * sinedelta;                             \
	}                                                          \
	else                                                       \
	{                                                          \
		if (pp>>31)                                            \
		{                                                      \
			siner += (sscf * (sinedelta + (inc>>17)));         \
		}                                                      \
		else                                                   \
		{                                                      \
			siner += (sscf * (sinedelta - (inc>>17)));         \
		}                                                      \
	}                                                          \
	out += (siner>>16) * woutmul;                              \
	}

#define CODEBLOCK_ES2_SIN(wsave)                               \
	{                                                          \
	uint32_t siner;                                            \
	uint8_t tpos = pp>>24;                                     \
	uint8_t spos = (uint8_t)(pp>>16);                          \
	int16_t delta = sine[tpos + 1] - sine[tpos];               \
	siner = sine[tpos] + ((delta * spos)>>8);                  \
	int32_t sinedelta = (wsave>>17) - (siner>>1);              \
	siner = siner << 16;                                       \
	if ((pp>>31) == (prevphase>>31))                           \
	{                                                          \
		siner += sscf * sinedelta;                             \
	}                                                          \
	else                                                       \
	{                                                          \
		if (pp>>31)                                            \
		{                                                      \
			siner += (sscf * (sinedelta - (inc>>17)));         \
		}                                                      \
		else                                                   \
		{                                                      \
			siner += (sscf * (sinedelta + (inc>>17)));         \
		}                                                      \
	}                                                          \
	out += (siner>>16) * woutmul;                              \
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// triangle

#define CODEBLOCK_ASO_TRI(wsave)                               \
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
	out += (trir>>16)*woutmul;                                 \
	}

#define CODEBLOCK_ES1_TRI(wsave)                               \
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
	out += (trir>>16)*woutmul;                                 \
	}


#define CODEBLOCK_ES2_TRI(wsave)                               \
	{                                                          \
	uint32_t trir;                                             \
	if(pp>>31)                                                 \
		trir = ~pp;                                            \
	else                                                       \
		trir = pp;                                             \
	trir = trir<<1;                                            \
	int32_t tridelta = (wsave>>17) - (trir>>17);               \
	if ((pp>>31) == (prevphase>>31))                           \
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
	out += (trir>>16)*woutmul;                                 \
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// saw down - first wave that transforms into square

#define CODEBLOCK_ASO_SDQ(wsave)                               \
	uint32_t sawsave = 0;                                      \
	sawsave = sawsave;                                         \
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
	sawsave = saw2r>>16;                                       \
	out += (saw2r>>16)*woutmul;                                \
	}

#define CODEBLOCK_ES1_SDQ(wsave)                               \
	uint32_t sawsave = 0;                                      \
	sawsave = sawsave;                                         \
	{                                                          \
	uint32_t saw2r = ~pp;                                      \
	uint32_t saw2nxt = ~ssnextphase;                           \
	int32_t saw2rdelta = (saw2nxt>>17) - (saw2r>>17);          \
	wsave = saw2r;                                             \
	saw2r += sscf * saw2rdelta;                                \
	sawsave = saw2r>>16;                                       \
	out += (saw2r>>16)*woutmul;                                \
	}

#define CODEBLOCK_ES2_SDQ(wsave)                               \
	uint32_t sawsave = 0;                                      \
	sawsave = sawsave;                                         \
	{                                                          \
	uint32_t saw2r = ~pp;                                      \
	int32_t saw2rdelta = (wsave>>17) - (saw2r>>17);            \
	saw2r += sscf * saw2rdelta;                                \
	sawsave = saw2r>>16;                                       \
	out += (saw2r>>16)*woutmul;                                \
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// square - second wave that sdq transforms to

#define CODEBLOCK_ASO_QSD(wsave)                               \
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
	saw1r = sawsave + (saw1r>>16) - 0x7FFF;                    \
	saw1r = saw1r * 0xFFFF;                                    \
	out += (saw1r>>16)*woutmul;                                \
	}

#define CODEBLOCK_ES1_QSD(wsave)                               \
	{                                                          \
	uint32_t saw1r = pp + 0x80000000;                          \
	uint32_t saw1nxt = ssnextphase + 0x80000000;               \
	int32_t saw1rdelta = (saw1nxt>>17) - (saw1r>>17);          \
	wsave = saw1r;                                             \
	saw1r += sscf * saw1rdelta;                                \
	saw1r = sawsave + (saw1r>>16) - 0x7FFF;                    \
	saw1r = saw1r * 0xFFFF;                                    \
	out += (saw1r>>16)*woutmul;                                \
	}

#define CODEBLOCK_ES2_QSD(wsave)                               \
	{                                                          \
	uint32_t saw1r = pp + 0x80000000;                          \
	int32_t saw1rdelta = (wsave>>17) - (saw1r>>17);            \
	saw1r += sscf * saw1rdelta;                                \
	saw1r = sawsave + (saw1r>>16) - 0x7FFF;                    \
	saw1r = saw1r * 0xFFFF;                                    \
	out += (saw1r>>16)*woutmul;                                \
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// square - first wave in sqr-sawrise transformation

#define CODEBLOCK_ASO_QSR(wsave)                               \
	uint32_t sawsave;                                          \
	uint32_t qsramp = sine[(lfoshape>>3)&0x7F];                \
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
	sawsave = saw1r>>16;                                       \
	out += (saw1r>>16)*qsramp;                                 \
	}

#define CODEBLOCK_ES1_QSR(wsave)                               \
	uint32_t sawsave;                                          \
	uint32_t qsramp = sine[(lfoshape>>3)&0x7F];                \
	{                                                          \
	uint32_t saw1r = pp + 0x80000000;                          \
	uint32_t saw1nxt = ssnextphase + 0x80000000;               \
	int32_t saw1rdelta = (saw1nxt>>17) - (saw1r>>17);          \
	wsave = saw1r;                                             \
	saw1r += sscf * saw1rdelta;                                \
	sawsave = saw1r>>16;                                       \
	out += (saw1r>>16)*qsramp;                                 \
	}

#define CODEBLOCK_ES2_QSR(wsave)                               \
	uint32_t sawsave;                                          \
	uint32_t qsramp = sine[(lfoshape>>3)&0x7F];                \
	{                                                          \
	uint32_t saw1r = pp + 0x80000000;                          \
	int32_t saw1rdelta = (wsave>>17) - (saw1r>>17);            \
	saw1r += sscf * saw1rdelta;                                \
	sawsave = saw1r>>16;                                       \
	out += (saw1r>>16)*qsramp;                                 \
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// saw rise - second wave in sqr-sawrise transformation

#define CODEBLOCK_ASO_SRQ(wsave)                               \
	qsramp = 0xFFFF - qsramp;                                  \
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
	saw2r = sawsave + (saw2r>>16) - 0x7FFF;                    \
	saw2r = saw2r * 0xFFFF;                                    \
	out += (saw2r>>16)*qsramp;                                 \
	}

#define CODEBLOCK_ES1_SRQ(wsave)                               \
	qsramp = 0xFFFF - qsramp;                                  \
	{                                                          \
	uint32_t saw2r = ~pp;                                      \
	uint32_t saw2nxt = ~ssnextphase;                           \
	int32_t saw2rdelta = (saw2nxt>>17) - (saw2r>>17);          \
	wsave = saw2r;                                             \
	saw2r += sscf * saw2rdelta;                                \
	saw2r = sawsave + (saw2r>>16) - 0x7FFF;                    \
	saw2r = saw2r * 0xFFFF;                                    \
	out += (saw2r>>16)*qsramp;                                 \
	}

#define CODEBLOCK_ES2_SRQ(wsave)                               \
	qsramp = 0xFFFF - qsramp;                                  \
	{                                                          \
	uint32_t saw2r = ~pp;                                      \
	int32_t saw2rdelta = (wsave>>17) - (saw2r>>17);            \
	saw2r += sscf * saw2rdelta;                                \
	saw2r = sawsave + (saw2r>>16) - 0x7FFF;                    \
	saw2r = saw2r * 0xFFFF;                                    \
	out += (saw2r>>16)*qsramp;                                 \
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// saw down PWM - only dif is max amplitude

#define CODEBLOCK_ASO_SDW(wsave)                               \
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
	out += saw2r>>16;                                          \
	}

#define CODEBLOCK_ES1_SDW(wsave)                               \
	{                                                          \
	uint32_t saw2r = ~pp;                                      \
	uint32_t saw2nxt = ~ssnextphase;                           \
	int32_t saw2rdelta = (saw2nxt>>17) - (saw2r>>17);          \
	wsave = saw2r;                                             \
	saw2r += sscf * saw2rdelta;                                \
	out += saw2r>>16;                                          \
	}

#define CODEBLOCK_ES2_SDW(wsave)                               \
	{                                                          \
	uint32_t saw2r = ~pp;                                      \
	int32_t saw2rdelta = (wsave>>17) - (saw2r>>17);            \
	saw2r += sscf * saw2rdelta;                                \
	out += saw2r>>16;                                          \
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// saw rise for PWM

#define CODEBLOCK_ASO_SRW(wsave)                                     \
	{                                                                \
	uint32_t pwshift = (0x10000 + sine[((lfoshape>>3)&0x7F)])<<15;   \
	uint32_t saw1r = pp + pwshift;                                   \
	if (saw1r>(~inc))                                                \
	{                                                                \
		uint32_t delta = (saw1r + inc)>>shift;                       \
		uint32_t rev = (delta * recp)>>13;                           \
		rev = rev*rev;                                               \
		saw1r -= rev;                                                \
	}                                                                \
	else                                                             \
	{                                                                \
		if (saw1r<inc)                                               \
		{                                                            \
			uint32_t delta = (inc - saw1r)>>shift;                   \
			uint32_t rev = (delta * recp)>>13;                       \
			rev = rev*rev;                                           \
			saw1r += rev;                                            \
		}                                                            \
	}                                                                \
	pwshift = (pwshift>>16) - 1;                                     \
	out = (out + (saw1r>>16) - pwshift)*0xFFFF;                      \
	}

#define CODEBLOCK_ES1_SRW(wsave)                                     \
	{                                                                \
	uint32_t pwshift = (0x10000 + sine[((lfoshape>>3)&0x7F)])<<15;   \
	uint32_t saw1r = pp + pwshift;                                   \
	uint32_t saw1nxt = ssnextphase + pwshift;                        \
	int32_t saw1rdelta = (saw1nxt>>17) - (saw1r>>17);                \
	wsave = saw1r;                                                   \
	saw1r += sscf * saw1rdelta;                                      \
	pwshift = (pwshift>>16) - 1;                                     \
	out = (out + (saw1r>>16) - pwshift)*0xFFFF;                      \
	}

#define CODEBLOCK_ES2_SRW(wsave)                                     \
	{                                                                \
	uint32_t pwshift = (0x10000 + sine[((lfoshape>>3)&0x7F)])<<15;   \
	uint32_t saw1r = pp + pwshift;                                   \
	int32_t saw1rdelta = (wsave>>17) - (saw1r>>17);                  \
	saw1r += sscf * saw1rdelta;                                      \
	pwshift = (pwshift>>16) - 1;                                     \
	out = (out + (saw1r>>16) - pwshift)*0xFFFF;                      \
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CODEBLOCK_WSUM(wone, wtwo, cbtype)                                 \
		uint32_t woutmul = sine[((lfoshape>>3)&0x7F) + 0x80];              \
		CODEBLOCK_##cbtype##_##wone (lforam.wsave_one)                     \
		woutmul = 0xFFFF - woutmul;                                        \
		CODEBLOCK_##cbtype##_##wtwo (lforam.wsave_two)                     \
		return out;
/*
#define CODEBLOCK_WSUM(wone, wtwo, cbtype)                              \
		uint32_t woutmul = 0xFFFF;                                         \
		CODEBLOCK_##cbtype##_##wone (lforam.wsave_one)                     \
		woutmul = 0;                                                       \
		CODEBLOCK_##cbtype##_##wtwo (lforam.wsave_two)                     \
		return out; //keeping calculation load almost same??

#define CODEBLOCK_WSUM(wone, wtwo, cbtype)                              \
		uint32_t woutmul = 0xFFFF;                                         \
		CODEBLOCK_##cbtype##_##wone (lforam.wsave_one)                     \
		return out; //keeping calculation load almost same??
// #define CODEBLOCK_WSUM CODEBLOCK_WSWITCH
*/
#define CODEBLOCK_TYPE(waveone, wavetwo, synctype)                          \
		uint32_t nlex_ ## waveone ## wavetwo ## _async_ ## synctype()       \
		{                                                                   \
		CODEBLOCK_START_ ## synctype                                        \
		CODEBLOCK_WSUM(waveone, wavetwo, ASO)                               \
		}                                                                   \
		uint32_t nlex_ ## waveone ## wavetwo ## _asnrf_ ## synctype()       \
		{                                                                   \
		CODEBLOCK_START_ ## synctype                                        \
		CODEBLOCK_PURIFY_IRQPOS                                             \
		lforam.irqstate = 0;                                                \
		NVIC_EnableIRQ(EXTI2_3_IRQn);                                       \
		CODEBLOCK_WSUM(waveone, wavetwo, ASO)                               \
		}                                                                   \
		uint32_t nlex_ ## waveone ## wavetwo ## _rise__ ## synctype()       \
		{                                                                   \
		CODEBLOCK_START_ ## synctype                                        \
		CODEBLOCK_SYNCRISE                                                  \
		CODEBLOCK_WSUM(waveone, wavetwo, ES1)                               \
		}                                                                   \
		uint32_t nlex_ ## waveone ## wavetwo ## _fall__ ## synctype()       \
		{                                                                   \
		CODEBLOCK_START_ ## synctype                                        \
		CODEBLOCK_SYNCFALL                                                  \
		CODEBLOCK_WSUM(waveone, wavetwo, ES1)                               \
		}                                                                   \
		uint32_t nlex_ ## waveone ## wavetwo ## _reset_ ## synctype()       \
		{                                                                   \
		CODEBLOCK_START_ ## synctype                                        \
		CODEBLOCK_SYNCRESET                                                 \
		CODEBLOCK_WSUM(waveone, wavetwo, ES2)                               \
		}                                                                   \

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// END

//waveonetwo
//SIN
//TRI
//SDQ
//SDQ - SAW DOWN SQUARE - first
//QSD - SQUARE SAW RISE - second
//QSR - SQUARE SAW RISE - first
//SRQ - SAW RISE SQUARE - second
//SDW - PWM first
//SRW - PWM second

//sunctype AS NS MS

//knob
CODEBLOCK_TYPE(SIN, TRI, AS)
CODEBLOCK_TYPE(TRI, SDQ, AS)
CODEBLOCK_TYPE(SDQ, QSD, AS)
CODEBLOCK_TYPE(QSR, SRQ, AS)
//midi
CODEBLOCK_TYPE(SIN, TRI, MS)
CODEBLOCK_TYPE(TRI, SDQ, MS)
CODEBLOCK_TYPE(SDQ, QSD, MS)
CODEBLOCK_TYPE(QSR, SRQ, MS)
//notes
CODEBLOCK_TYPE(SIN, TRI, NS)
CODEBLOCK_TYPE(TRI, SDQ, NS)
CODEBLOCK_TYPE(SDQ, QSD, NS)
CODEBLOCK_TYPE(SDW, SRW, NS)



uint32_t (*const nlex2_classic[128])(void) =
{
	nlex_SINTRI_async_AS, nlex_TRISDQ_async_AS, nlex_SDQQSD_async_AS, nlex_QSRSRQ_async_AS, nlex_SINTRI_async_MS, nlex_TRISDQ_async_MS, nlex_SDQQSD_async_MS, nlex_QSRSRQ_async_MS,
	nlex_SINTRI_asnrf_AS, nlex_TRISDQ_asnrf_AS, nlex_SDQQSD_asnrf_AS, nlex_QSRSRQ_asnrf_AS, nlex_SINTRI_asnrf_MS, nlex_TRISDQ_asnrf_MS, nlex_SDQQSD_asnrf_MS, nlex_QSRSRQ_asnrf_MS,
	nlex_SINTRI_rise__AS, nlex_TRISDQ_rise__AS, nlex_SDQQSD_rise__AS, nlex_QSRSRQ_rise__AS, nlex_SINTRI_rise__MS, nlex_TRISDQ_rise__MS, nlex_SDQQSD_rise__MS, nlex_QSRSRQ_rise__MS,
	nlex_SINTRI_reset_AS, nlex_TRISDQ_reset_AS, nlex_SDQQSD_reset_AS, nlex_QSRSRQ_reset_AS, nlex_SINTRI_reset_MS, nlex_TRISDQ_reset_MS, nlex_SDQQSD_reset_MS, nlex_QSRSRQ_reset_MS,

	nlex_SINTRI_async_AS, nlex_TRISDQ_async_AS, nlex_SDQQSD_async_AS, nlex_QSRSRQ_async_AS, nlex_SINTRI_async_MS, nlex_TRISDQ_async_MS, nlex_SDQQSD_async_MS, nlex_QSRSRQ_async_MS,
	nlex_SINTRI_fall__AS, nlex_TRISDQ_fall__AS, nlex_SDQQSD_fall__AS, nlex_QSRSRQ_fall__AS, nlex_SINTRI_fall__MS, nlex_TRISDQ_fall__MS, nlex_SDQQSD_fall__MS, nlex_QSRSRQ_fall__MS,
	nlex_SINTRI_asnrf_AS, nlex_TRISDQ_asnrf_AS, nlex_SDQQSD_asnrf_AS, nlex_QSRSRQ_asnrf_AS, nlex_SINTRI_asnrf_MS, nlex_TRISDQ_asnrf_MS, nlex_SDQQSD_asnrf_MS, nlex_QSRSRQ_asnrf_MS,
	nlex_SINTRI_reset_AS, nlex_TRISDQ_reset_AS, nlex_SDQQSD_reset_AS, nlex_QSRSRQ_reset_AS, nlex_SINTRI_reset_MS, nlex_TRISDQ_reset_MS, nlex_SDQQSD_reset_MS, nlex_QSRSRQ_reset_MS,

	nlex_SINTRI_async_AS, nlex_TRISDQ_async_AS, nlex_SDQQSD_async_AS, nlex_QSRSRQ_async_AS, nlex_SINTRI_async_MS, nlex_TRISDQ_async_MS, nlex_SDQQSD_async_MS, nlex_QSRSRQ_async_MS,
	nlex_SINTRI_fall__AS, nlex_TRISDQ_fall__AS, nlex_SDQQSD_fall__AS, nlex_QSRSRQ_fall__AS, nlex_SINTRI_fall__MS, nlex_TRISDQ_fall__MS, nlex_SDQQSD_fall__MS, nlex_QSRSRQ_fall__MS,
	nlex_SINTRI_rise__AS, nlex_TRISDQ_rise__AS, nlex_SDQQSD_rise__AS, nlex_QSRSRQ_rise__AS, nlex_SINTRI_rise__MS, nlex_TRISDQ_rise__MS, nlex_SDQQSD_rise__MS, nlex_QSRSRQ_rise__MS,
	nlex_SINTRI_reset_AS, nlex_TRISDQ_reset_AS, nlex_SDQQSD_reset_AS, nlex_QSRSRQ_reset_AS, nlex_SINTRI_reset_MS, nlex_TRISDQ_reset_MS, nlex_SDQQSD_reset_MS, nlex_QSRSRQ_reset_MS,

	nlex_SINTRI_async_AS, nlex_TRISDQ_async_AS, nlex_SDQQSD_async_AS, nlex_QSRSRQ_async_AS, nlex_SINTRI_async_MS, nlex_TRISDQ_async_MS, nlex_SDQQSD_async_MS, nlex_QSRSRQ_async_MS,
	nlex_SINTRI_fall__AS, nlex_TRISDQ_fall__AS, nlex_SDQQSD_fall__AS, nlex_QSRSRQ_fall__AS, nlex_SINTRI_fall__MS, nlex_TRISDQ_fall__MS, nlex_SDQQSD_fall__MS, nlex_QSRSRQ_fall__MS,
	nlex_SINTRI_rise__AS, nlex_TRISDQ_rise__AS, nlex_SDQQSD_rise__AS, nlex_QSRSRQ_rise__AS, nlex_SINTRI_rise__MS, nlex_TRISDQ_rise__MS, nlex_SDQQSD_rise__MS, nlex_QSRSRQ_rise__MS,
	nlex_SINTRI_reset_AS, nlex_TRISDQ_reset_AS, nlex_SDQQSD_reset_AS, nlex_QSRSRQ_reset_AS, nlex_SINTRI_reset_MS, nlex_TRISDQ_reset_MS, nlex_SDQQSD_reset_MS, nlex_QSRSRQ_reset_MS
};

uint32_t (*const nlex2_mono_osc[64])(void) =
{
	nlex_SINTRI_async_NS, nlex_TRISDQ_async_NS, nlex_SDQQSD_async_NS, nlex_SDWSRW_async_NS,
	nlex_SINTRI_asnrf_NS, nlex_TRISDQ_asnrf_NS, nlex_SDQQSD_asnrf_NS, nlex_SDWSRW_asnrf_NS,
	nlex_SINTRI_rise__NS, nlex_TRISDQ_rise__NS, nlex_SDQQSD_rise__NS, nlex_SDWSRW_rise__NS,
	nlex_SINTRI_reset_NS, nlex_TRISDQ_reset_NS, nlex_SDQQSD_reset_NS, nlex_SDWSRW_reset_NS,

	nlex_SINTRI_async_NS, nlex_TRISDQ_async_NS, nlex_SDQQSD_async_NS, nlex_SDWSRW_async_NS,
	nlex_SINTRI_fall__NS, nlex_TRISDQ_fall__NS, nlex_SDQQSD_fall__NS, nlex_SDWSRW_fall__NS,
	nlex_SINTRI_asnrf_NS, nlex_TRISDQ_asnrf_NS, nlex_SDQQSD_asnrf_NS, nlex_SDWSRW_asnrf_NS,
	nlex_SINTRI_reset_NS, nlex_TRISDQ_reset_NS, nlex_SDQQSD_reset_NS, nlex_SDWSRW_reset_NS,

	nlex_SINTRI_async_NS, nlex_TRISDQ_async_NS, nlex_SDQQSD_async_NS, nlex_SDWSRW_async_NS,
	nlex_SINTRI_fall__NS, nlex_TRISDQ_fall__NS, nlex_SDQQSD_fall__NS, nlex_SDWSRW_fall__NS,
	nlex_SINTRI_rise__NS, nlex_TRISDQ_rise__NS, nlex_SDQQSD_rise__NS, nlex_SDWSRW_rise__NS,
	nlex_SINTRI_reset_NS, nlex_TRISDQ_reset_NS, nlex_SDQQSD_reset_NS, nlex_SDWSRW_reset_NS,

	nlex_SINTRI_async_NS, nlex_TRISDQ_async_NS, nlex_SDQQSD_async_NS, nlex_SDWSRW_async_NS,
	nlex_SINTRI_fall__NS, nlex_TRISDQ_fall__NS, nlex_SDQQSD_fall__NS, nlex_SDWSRW_fall__NS,
	nlex_SINTRI_rise__NS, nlex_TRISDQ_rise__NS, nlex_SDQQSD_rise__NS, nlex_SDWSRW_rise__NS,
	nlex_SINTRI_reset_NS, nlex_TRISDQ_reset_NS, nlex_SDQQSD_reset_NS, nlex_SDWSRW_reset_NS
};


void nlex2_sr_classic()
{
	uint8_t wsel = lfoshape>>10;
	uint8_t wctype = wsel | (parameters.lfomidisync << 2) | (lforam.irqstate << 3) | (parameters.lfoextsynctype << 5);
	uint32_t value = nlex2_classic[wctype]();
	LFO_OUTPUT = value>>16;
}

void nlex2_sr_monoosc()
{
	uint8_t wsel = lfoshape>>10;
	uint8_t wctype = wsel | (lforam.irqstate << 2) | (parameters.lfoextsynctype << 4);
	uint32_t value = nlex2_mono_osc[wctype]();
	LFO_OUTPUT = value>>16;
}










/*


uint32_t (*const nlex_classic[32])(void) =
{
	nlex_SINTRI_async_AS,
	nlex_SINTRI_rise__AS,
	nlex_SINTRI_fall__AS,
	nlex_SINTRI_reset_AS,
	nlex_TRISDQ_async_AS,
	nlex_TRISDQ_rise__AS,
	nlex_TRISDQ_fall__AS,
	nlex_TRISDQ_reset_AS,
	nlex_SDQQSD_async_AS,
	nlex_SDQQSD_rise__AS,
	nlex_SDQQSD_fall__AS,
	nlex_SDQQSD_reset_AS,
	nlex_QSRSRQ_async_AS,
	nlex_QSRSRQ_rise__AS,
	nlex_QSRSRQ_fall__AS,
	nlex_QSRSRQ_reset_AS,
	nlex_SINTRI_async_MS,
	nlex_SINTRI_rise__MS,
	nlex_SINTRI_fall__MS,
	nlex_SINTRI_reset_MS,
	nlex_TRISDQ_async_MS,
	nlex_TRISDQ_rise__MS,
	nlex_TRISDQ_fall__MS,
	nlex_TRISDQ_reset_MS,
	nlex_SDQQSD_async_MS,
	nlex_SDQQSD_rise__MS,
	nlex_SDQQSD_fall__MS,
	nlex_SDQQSD_reset_MS,
	nlex_QSRSRQ_async_MS,
	nlex_QSRSRQ_rise__MS,
	nlex_QSRSRQ_fall__MS,
	nlex_QSRSRQ_reset_MS
};
uint32_t (*const nlex_mono_osc[16])(void) =
{
	nlex_SINTRI_async_NS,
	nlex_SINTRI_rise__NS,
	nlex_SINTRI_fall__NS,
	nlex_SINTRI_reset_NS,
	nlex_TRISDQ_async_NS,
	nlex_TRISDQ_rise__NS,
	nlex_TRISDQ_fall__NS,
	nlex_TRISDQ_reset_NS,
	nlex_SDQQSD_async_NS,
	nlex_SDQQSD_rise__NS,
	nlex_SDQQSD_fall__NS,
	nlex_SDQQSD_reset_NS,
	nlex_SDWSRW_async_NS,
	nlex_SDWSRW_rise__NS,
	nlex_SDWSRW_fall__NS,
	nlex_SDWSRW_reset_NS
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void nlex_sr_classic()
{
	uint8_t wsel = lfoshape>>10;
	uint8_t wctype;
	if (lforam.irqstate == 3)
		wctype = (parameters.lfomidisync<<4) | (wsel<<2) | lforam.irqstate; //TODO improve this comparision
	else
		wctype = (parameters.lfomidisync<<4) | (wsel<<2) | (lforam.irqstate & (parameters.lfoextsynctype + 1));
	//nl3m_debug[nl3m_dp++] = wctype;
	uint32_t value = nlex_classic[wctype]();
	LFO_OUTPUT = value>>16;

}

void nlex_sr_monoosc()
{
	uint8_t wsel = lfoshape>>10;
	uint8_t wctype;
	if (lforam.irqstate == 3)
		wctype = (wsel<<2) | lforam.irqstate; //TODO improve this comparision
	else
		wctype = (wsel<<2) | (lforam.irqstate & (parameters.lfoextsynctype + 1));
	//nl3m_debug[nl3m_dp++] = wctype;
	uint32_t value = nlex_mono_osc[wctype]();
	LFO_OUTPUT = value>>16;

}
*/
extern volatile uint32_t pp_o2p;
extern volatile int32_t pp_o2pwshift;
void nlex_cr()
{
	lforam.nl3m_pitch = (pp_o2p>>15) + pp_o2pwshift - (0x1800 + 0x3800*parameters.lfokeysync);
}

void nlex_leds(uint32_t frame)
{
	//@TODO: reduce framerate by 256, add pwm for brightness control
	//calc lfo leds
	uint8_t pos = frame & 0x1F;
	uint8_t i;
	if (pos == 0) //
	{
		uint8_t lfo_shape[5] = {0,0,0,0,0};
		//lfo_shape[lfoshape>>10] = (0x3FF - (lfoshape & 0x3FF))>>2;

		lfo_shape[lfoshape>>10] = (0x3FF - (lfoshape & 0x3FF))>>2;
		lfo_shape[(lfoshape>>10) + 1] = (lfoshape & 0x3FF)>>2;
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
			led_set(LED_LFO1SINE<<i, lforam.ledvals[i]>pos?1:0);
		}
	}
}

#warning "phase problem with midi sync on and keysync on"
void nlex_retrig(void) //TODO: fix midisync problem
{
	lforam.phaseacc = parameters.lfosyncstart;
}

void nlcom_cr(void);
void nlcom_async(void);


const neutr_lfo_t nl_ex_mono =
{
		0, //void (*retrig)(void);
		//nl3m_midistart,  //void (*midistart)(void);
		//nl3m_midiclock,  //void (*midiclock)(void);
		nlex_cr,      //void (*cr)(void);
		nlex2_sr_monoosc,     //void (*sr)(void);
		nlex_leds,   //void (*lfoleds)(uint32_t time);
		0,           //void (*noteon)(uint8_t note, uint32_t ts);
		0,           //void (*noteoff)(uint8_t note, uint32_t ts);
		0,           //void (*damper)(uint8_t val, uint32_t ts);
		0,           //void (*panic)(void);
		//nl3m_irq,           //void (*irqsync)(uint8_t type);
		0,            //void (*asyncmain)(void);
		nl_init_classic             //void (*initialize)(uint8_t prevmode);
};

const neutr_lfo_t nl_ex_classic =
{
		nlex_retrig, //void (*retrig)(void);
		//nl3m_midistart,  //void (*midistart)(void);
		//nl3m_midiclock,  //void (*midiclock)(void);
		nlcom_cr,      //void (*cr)(void);
		nlex2_sr_classic,     //void (*sr)(void);
		nlex_leds,   //void (*lfoleds)(uint32_t time);
		0,           //void (*noteon)(uint8_t note, uint32_t ts);
		0,           //void (*noteoff)(uint8_t note, uint32_t ts);
		0,           //void (*damper)(uint8_t val, uint32_t ts);
		0,           //void (*panic)(void);
		//nl3m_irq,           //void (*irqsync)(uint8_t type);
		nlcom_async,            //void (*asyncmain)(void);
		nl_init_classic             //void (*initialize)(uint8_t prevmode);
};
