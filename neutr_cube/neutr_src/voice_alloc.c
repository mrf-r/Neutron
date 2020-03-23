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
 * voice_alloc.c
 *
 *  Created on: 28 nov. 2018 ã.
 * main mode - last note priority
 *
 * paraphonic mode - highest and lowest for osc1 and osc2
 *
 * poly mode - last note
 */

#include "neutron.h"
#include "menu.h"


#define NOTE_HOLD_MAX_MEMORY 10

extern volatile uint8_t retrigrequest;

volatile uint8_t va_last_noteon_velo = 0x7F;
volatile uint8_t va_damperstate = 0;
volatile uint8_t va_gatestate = 0;
volatile uint8_t va_o1key;
volatile uint8_t va_o2key;
volatile uint8_t va_notemem[NOTE_HOLD_MAX_MEMORY];
volatile uint8_t va_notesheld = 0;

void va_m0_noteon(uint8_t note);
void va_m0_noteoff(uint8_t note);
void va_m0m1switch(void);
void va_m1m0switch(void);
void va_m1_noteon(uint8_t note);
void va_m1_noteoff(uint8_t note);
void va_m2_noteon(uint8_t note);
void va_m2_noteoff(uint8_t note);

void va_noteon(note_t note)
{
	va_last_noteon_velo = note.velo;
	switch(parameters.paraphony)
	{
	case 0:
		va_m0_noteon(note.note);
		break;
	case 1:
		va_m1_noteon(note.note);
		break;
	case 2:
		va_m2_noteon(note.note);
		break;
	}
}
void va_noteoff(note_t note)
{
	switch(parameters.paraphony)
	{
	case 0:
		va_m0_noteoff(note.note);
		break;
	case 1:
		va_m1_noteoff(note.note);
		break;
	case 2:
		va_m2_noteoff(note.note);
		break;
	}
}

inline void keysync(void)
{
	if (parameters.lfokeysync)
	{
		if (neutron_lfo->retrig)//lfokeysync
			neutron_lfo->retrig();
	}
}

inline void gateon(void)
{
	if (va_gatestate || va_damperstate)
	{
		if (parameters.gateretrig)
		{
			retrigrequest = 2;
			keysync();
		}
	}
	else
	{
		keysync();
		GATEPORT = GATEON;
	}
	va_gatestate = 1;
}
inline void gateoff(void)
{
	va_gatestate = 0;
	if (!va_damperstate)
		GATEPORT = GATEOFF;
}

void va_damper(uint8_t val)
{
	if (val>0x40)
	{
		va_damperstate = 1;
		GATEPORT = GATEON; // lol
	}
	else
	{
		va_damperstate = 0;
		if (!va_gatestate)
			GATEPORT = GATEOFF;
	}
}

void va_panic()
{
	va_gatestate = 0;
	va_damperstate = 0;
	GATEPORT = GATEOFF;
	//@TODO: clear notes in modes?
	va_notesheld = 0;
	if (neutron_lfo->panic)
		neutron_lfo->panic();
}

void va_setmode(uint8_t mode) //paraphony mode
{
	if (mode == 1)
		va_m0m1switch();
	if (mode == 0)
		va_m1m0switch();
	//@TODO: switch sequence 0-1-2-0-1-2...
}

inline void seto1(uint8_t key, uint8_t glide)
{
	va_o1key = key;
	pp_seto1(key,glide);
}

inline void seto2(uint8_t key, uint8_t glide)
{
	va_o2key = key;
	pp_seto2(key,glide);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * osc1 - always low
 * osc2 - always high
 * filter - always same as osc1
 * cv out - switched
 *
 * mode 0 : no paraphony
 * 0-1: sync gate on
 * 1-n: glide
 * 1-0: gate off
 *
 * mode 1 : glide always on
 * 0-1: both same key
 * 1-2: o2 higher, o1 lower, may swap
 * 2-n: highest and lowest
 * 2-1: nothing changes
 * 1-0: gate off
 *
 * mode 2 :
 * 0-1: do nothing
 * 1-2: start glide and gate on and sync
 * 2-n: same highest and lowest
 * 2-1: gate off
 * 1-0: do nothing
 *
 */



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MODE 0 - monophonic mode. last note priority, 10 notes memory



void va_m0_noteon(uint8_t note)
{
	uint8_t i;

	if (va_notesheld < NOTE_HOLD_MAX_MEMORY)
	{
		va_notesheld++;
	}

	for (i = va_notesheld; i>0; i--)        //scan from bottom to top
	{
		va_notemem[i] = va_notemem[i - 1]; //shift the whole used part of buffer down (from the priority pov)
	}
	va_notemem[0] = note;                     //and put new note at the top

	if (va_notesheld == 1)
		i = 0;
	else
		i = 1;

	seto1(note, i);
	seto2(note, i);
	gateon();
}

void va_m0_noteoff(uint8_t note)  // nt_off_played(note_t note)
{
	uint8_t i;

	for (i = 0; i<va_notesheld; i++)            //scan from top to bot
	{
		if (va_notemem[i] == note)   //if we found that note, than just release it...
		{
			va_notesheld--;                     //decrement
			for (;i<va_notesheld;i++)
			{
				va_notemem[i] = va_notemem[i+1]; //...by shifting up other notes
			}
			//now we must exit, cause i == notes held
		}
	}

	if (va_o1key != va_notemem[0])
	{
		seto1(va_notemem[0], 1);
		seto2(va_notemem[0], 1);
		gateon();
	}
	if (!va_notesheld)
	{
		gateoff();
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void va_m0m1switch()
{
	uint8_t i;
	uint8_t nup = 0, ndwn = 127;
	if(va_notesheld)
	{
		for (i = 0; i<va_notesheld; i++)
		{
			if (va_notemem[i] > nup)
				nup = va_notemem[i];
			if (va_notemem[i]<ndwn)
				ndwn = va_notemem[i];
		}
		seto1(ndwn, 1);
		seto2(nup, 1);
	}
}
void va_m1m0switch()
{
	if(va_notesheld)
	{
		seto1(va_notemem[0],1);
		seto2(va_notemem[0],1);
	}
}

void va_m1_noteon(uint8_t note)
{
	uint8_t i;

	if (va_notesheld < NOTE_HOLD_MAX_MEMORY)
	{
		va_notesheld++;
	}

	for (i = va_notesheld; i>0; i--)        //scan from bottom to top
	{
		va_notemem[i] = va_notemem[i - 1]; //shift the whole used part of buffer down (from the priority pov)
	}
	va_notemem[0] = note;                     //and put new note at the top

	if (va_notesheld == 1)
	{
		seto1(note, 0);
		seto2(note, 0);
	}
	else
	{
		if (va_o1key > note)
			seto1(note, 1);
		if (va_o2key < note)
			seto2(note, 1);
	}
	gateon();
}

void va_m1_noteoff(uint8_t note)  // nt_off_played(note_t note)
{
	uint8_t i;
	//uint8_t tmp = va_notemem[0];

	for (i = 0; i<va_notesheld; i++)            //scan from top to bot
	{
		if (va_notemem[i] == note)   //if we found that note, than just release it...
		{
			va_notesheld--;                     //decrement
			for (;i<va_notesheld;i++)
			{
				va_notemem[i] = va_notemem[i+1]; //...by shifting up other notes
			}
			//now we must exit, cause i == notes held
		}
	}

	//independent
	if (va_notesheld)
	{
		if (note == va_o1key)
		{
			//scan for lowest
			uint8_t nf = 127;
			for (i = 0; i<va_notesheld; i++)
			{
				if (va_notemem[i] < nf)
					nf = va_notemem[i];
			}
			seto1(nf,1);
			gateon();
		}
		if (note == va_o2key)
		{
			//scan for highest
			uint8_t nf = 0;
			for (i = 0; i<va_notesheld; i++)
			{
				if (va_notemem[i] > nf)
					nf = va_notemem[i];
			}
			seto2(nf,1);
			gateon();
		}
	}
	else
	{
		gateoff();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//m2 - mrf paraphonic mode - gateon only on second voice
uint8_t va_m2_first;
//uint8_t va_m2_second;
void va_m2_noteon(uint8_t note)
{
	uint8_t i;

	if (va_notesheld < NOTE_HOLD_MAX_MEMORY)
	{
		va_notesheld++;
	}

	for (i = va_notesheld; i>0; i--)        //scan from bottom to top
	{
		va_notemem[i] = va_notemem[i - 1]; //shift the whole used part of buffer down (from the priority pov)
	}
	va_notemem[0] = note;                     //and put new note at the top


	switch (va_notesheld)
	{
	case 1:
		va_m2_first = note;
		//seto1(note, 0);
		//seto2(note, 0);
		break;
	case 2:
		//va_m2_second = note;
		if (va_m2_first > note)
		{
			seto1(note,0);
			seto2(va_m2_first,0);
		}
		else
		{
			seto1(va_m2_first,0);
			seto2(note,0);
		}
		//if (va_o1key > note)
		//	seto1(note, 0);
		//if (va_o2key < note)
		//	seto2(note, 0);
		gateon();
		break;
	default:
		if (va_o1key > note)
			seto1(note, 1);
		if (va_o2key < note)
			seto2(note, 1);
		gateon();
		break;
	}
}

void va_m2_noteoff(uint8_t note)  // nt_off_played(note_t note)
{
	uint8_t i;
	//uint8_t tmp = va_notemem[0];

	for (i = 0; i<va_notesheld; i++)            //scan from top to bot
	{
		if (va_notemem[i] == note)   //if we found that note, than just release it...
		{
			va_notesheld--;                     //decrement
			for (;i<va_notesheld;i++)
			{
				va_notemem[i] = va_notemem[i+1]; //...by shifting up other notes
			}
			//now we must exit, cause i == notes held
		}
	}

	//independent

	if (va_notesheld > 1)
	{
		if (note == va_o1key)
		{
			//scan for lowest
			uint8_t nf = 127;
			for (i = 0; i<va_notesheld; i++)
			{
				if (va_notemem[i] < nf)
					nf = va_notemem[i];
			}
			seto1(nf,1);
			gateon();
		}
		if (note == va_o2key)
		{
			//scan for highest
			uint8_t nf = 0;
			for (i = 0; i<va_notesheld; i++)
			{
				if (va_notemem[i] > nf)
					nf = va_notemem[i];
			}
			seto2(nf,1);
			gateon();
		}
	}
	else
	{
		if (note == va_o1key)
			va_m2_first = va_o2key;
		else
			va_m2_first = va_o1key;
		gateoff();
	}
}


