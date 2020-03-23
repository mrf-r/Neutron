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
 * input_proc.c
 *
 *  Created on: Dec 31, 2018
 */

#include "neutron.h"
#include "menu.h"

extern uint32_t timestamp;

// we just keep it same as buttons
#define MIDI_NON    0x4000
#define MIDI_NOFF   0x8000
#define MIDI_DMPON  0x1000
#define MIDI_DMPOFF 0x2000

typedef enum
{
	EVNT_TYPE_NOTEOFF = 0,
	EVNT_TYPE_NOTEON,
	EVNT_TYPE_DAMPER,
	EVNT_TYPE_BTTN,
	EVNT_TYPE_TOTAL
}event_type_en;


typedef struct
{
	uint32_t ts;
	uint16_t type;
	uint16_t event;
}user_action_t;


volatile user_action_t usact[64];
volatile user_action_t last_button;
volatile uint8_t input_buf_w = 0;
volatile uint8_t input_buf_r = 0;


void event_add(event_type_en type, uint16_t event)
{
	usact[input_buf_w].ts = timestamp;
	usact[input_buf_w].type = type;
	usact[input_buf_w].event = event;
	input_buf_w = (input_buf_w + 1)&0x3F;
}

void buttonpress(uint16_t change)
{
	event_add(EVNT_TYPE_BTTN, change);
}

void noteon(note_t note)
{
	event_add(EVNT_TYPE_NOTEON, note.data);
}
void noteoff(note_t note)
{
	event_add(EVNT_TYPE_NOTEOFF, note.data);
}
void damper(uint8_t value)
{
	event_add(EVNT_TYPE_DAMPER, value);
}

void ev_proc_noteoff(uint32_t ts, uint16_t data)
{
	note_t n;
	n.data = data;
	va_noteoff(n);
	if (neutron_lfo->noteoff)
		neutron_lfo->noteoff(n.note, ts);
}
void ev_proc_noteon(uint32_t ts, uint16_t data)
{
	note_t n;
	n.data = data;
	va_noteon(n);
	if (neutron_lfo->noteon)
		neutron_lfo->noteon(n.note, ts);
}
void ev_proc_damper(uint32_t ts, uint16_t data)
{
	va_damper((uint8_t)data);
	if (neutron_lfo->damper)
		neutron_lfo->damper(data, ts);
}
void ev_proc_bttn(uint32_t ts, uint16_t data)
{
	uint8_t i;
	//convert change to state
	//convert bitmap to position
	//process every position
	for (i=0; i<7; i++)
	{
		if ((data>>i)&0x1)
		{
			if (data & (0x0100<<i))
			{
				led_blink(0,0);
				if (page.buttons[i])
				{
					page.buttons[i]();
				}
			}
			else
			{
				if (page.buttons[i+7])
				{
					page.buttons[i+7]();
				}
			}
		}
	}


}

void (*const proc_evnt[])(uint32_t, uint16_t) =
{
		ev_proc_noteoff,
		ev_proc_noteon,
		ev_proc_damper,
		ev_proc_bttn
};

void eventscheck()
{
	while(input_buf_r != input_buf_w) //was if
	{
		user_action_t act = usact[input_buf_r];
		input_buf_r = (input_buf_r + 1)&0x3F;
		if (act.type < EVNT_TYPE_TOTAL)
		{
			proc_evnt[act.type](act.ts, act.event);
		}
	}
}


