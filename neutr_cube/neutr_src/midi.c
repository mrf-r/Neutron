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

* created unsigned long long time_ago;
*/

// neutron stripped and hardened version
#include "stm32f072xb.h"
#include "neutron.h"



//uint32_t* const receiver_not_empty = (uint32_t*)(0x42000000 + (0x9C000 + 0x14)*32 + 0*4);


#define MIDI_IN_STATUS_VALID_TIME_MS 15000
#define MIDI_TIMER_CALL_RATE 48000

const uint32_t MIDI_STATUS_DELAY = MIDI_TIMER_CALL_RATE * MIDI_IN_STATUS_VALID_TIME_MS / 1000;
volatile uint8_t midi_channel_switch = 0;

/////////////////////////////////////////////////
// synth api @TODO:this must be somewhere else
/*
void noteon(note_t note);
void noteoff(note_t note);
void damperon(void);
void damperoff(void);
void midiclock(void);
void midistart(void);
void panic(void);
void aftertouch(uint8_t value);
void modwheel(uint16_t value);
void pitchwheel(int16_t value);


uint8_t midi_channel_switch;
*/
///////////////////////////////////////////////////



void midi_rxd_noa(uint8_t d);

void midi_rxd_noteoff(uint8_t d);
void midi_rxd_noteoff2(uint8_t d);
void midi_rxd_noteon(uint8_t d);
void midi_rxd_noteon2(uint8_t d);
void midi_rxd_controlchange(uint8_t d);
void midi_rxd_controlchange2(uint8_t d);
void midi_rxd_aftertouch(uint8_t d);
void midi_rxd_pitchwheel(uint8_t d);
void midi_rxd_pitchwheel2(uint8_t d);


void sbh_na(void);
void sbh_rt_active_sense(void);

void (*const midi_channel_h[7])(uint8_t d) =   //system byte handle
{
	midi_rxd_noteoff,                                 // note off                    | note number       | velocity
	midi_rxd_noteon,                                  // note on                     | note number       | velocity
	midi_rxd_noa,                                     // polyphonic key pressure     | note number       | pressure
	midi_rxd_controlchange,                           // control change              | controller number | value
	midi_rxd_noa,                                     // program change              | program number    | --
	midi_rxd_aftertouch,                              // aftertouch                  | pressure          | --
	midi_rxd_pitchwheel,                              // pitch wheel change          | LSB               | MSB
};
//

/*
void midiclock(void)
{
	if (neutron_lfo->midiclock)
		neutron_lfo->midiclock();
}
void midistart(void)
{
	if (neutron_lfo->midistart)
		neutron_lfo->midistart();
}
*/

void (*const midi_system_h[16])() =
{
	sbh_na,                                           //start sysex reception        | ID                | ...
	sbh_na,                                           //midi time code               | timecode          | --
	sbh_na,                                           //song position pointer        | LSB               | MSB
	sbh_na,                                           //select song                  | song number       | --
	sbh_na,                                           //
	sbh_na,                                           //
	sbh_na,                                           //tune request                 | --                | --
	sbh_na,                                           //end sysex reception          | --                | --
	midiclock,                                        //main sync
	sbh_na,                                           //don't know what it used for
	midistart,                                        //start
	sbh_na,                                           //continue
	sbh_na,                                           //stop
	sbh_na,                                           //
	sbh_rt_active_sense,                              //270mS delay after last message, 330mS - connection error
	sbh_na                                            //reset
};

void (*midi_rx_data)(uint8_t d) = midi_rxd_noa; //data handler
volatile uint32_t status_timer;
volatile uint8_t midi_temp;

volatile uint8_t txbuf[MIDI_TX_BUFSIZE];
volatile uint8_t mtxw = 0;
volatile uint8_t mtxr = 0;
volatile uint8_t midimode = 0; //1 = usb mode

void USART1_IRQHandler(void)
{
	if (USART1->ISR & USART_ISR_RXNE)
	{
		uint8_t rxbyte = USART1->RDR;
		USART1->RQR = USART_RQR_RXFRQ;

		//just wait until tx ready and put thru
		if (!midimode)
		{
			txbuf[mtxw] = rxbyte;
			mtxw++;
			if (mtxw == MIDI_TX_BUFSIZE)
			{
				mtxw = 0;
			}
			USART1->CR1 |= USART_CR1_TXEIE;
		}

		if (rxbyte & 0x80)
		{
			uint8_t type = (rxbyte>>4)&0x7;
			uint8_t channel = rxbyte & 0xF;

			if (type == 0x7)
			{
				midi_system_h[channel]();
			}
			else
			{
				if (channel == midi_channel_switch)
				{
					midi_rx_data = midi_channel_h[type];
				}
				else
				{
					midi_rx_data = midi_rxd_noa;
				}
			}
		}
		else
		{
			midi_rx_data(rxbyte);
		}
		status_timer = MIDI_STATUS_DELAY;
	}
	if (USART1->CR1 & USART_CR1_TXEIE)
	{
		if (USART1->ISR & USART_ISR_TXE)
		{
			USART1->TDR = txbuf[mtxr];
			mtxr++;
			if (mtxr == MIDI_TX_BUFSIZE)
			{
				mtxr = 0;
			}
			if (mtxr == mtxw)
			{
				USART1->CR1 &= ~USART_CR1_TXEIE;
			}
		}
	}
	//try to handle bad situations
	if (USART1->ISR & USART_ISR_ORE) //TODO: check after debug stop!!!
	{
		USART1->ICR = USART_ICR_ORECF;
		//all notes off
		va_panic();

	}
}

void midi_check_buffer() //@TODO: do we need this?
{
	if (!(USART1->CR1 & USART_CR1_TXEIE))
	{
		if (mtxr != mtxw)
		{
			USART1->CR1 |= USART_CR1_TXEIE;
		}
	}
}

void hw_midi_status_check()
{
	//1 - input status valid
	//2 - output status refresh
	//3 - active sense
	if (status_timer)
	{
		status_timer--;
		if (status_timer == 0)
		{
			midi_rx_data = midi_rxd_noa;
			//@TODO: finalize
		}
	}
}




////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
// CC HNDL

void mcc_dat_damperpedal(uint8_t dat)
{
	damper(dat);
	midi_rx_data = midi_rxd_controlchange;
}


//**********************************************************************************************************
// all these will generate 2 events (high and low bytes).
#define HCLR 0x007F
#define LCLR 0x3F80

extern volatile uint16_t pp_modwheel;
//uint16_t mp1_modwh;
void mcc_dat_mw_h(uint8_t dat)
{
	pp_modwheel &= HCLR;
	pp_modwheel |= dat<<7;
	//modwheel(mp1_modwh);
	midi_rx_data = midi_rxd_controlchange;
}
void mcc_dat_mw_l(uint8_t dat)
{
	pp_modwheel &= LCLR;
	pp_modwheel |= dat;
	//modwheel(mp1_modwh);
	midi_rx_data = midi_rxd_controlchange;
}

void mcc_allsoundoff(uint8_t dummy)
{
	va_panic();
	midi_rx_data = midi_rxd_controlchange;
	(void)dummy;
}
void mcc_allnotesoff(uint8_t dummy)
{
	va_panic();
	midi_rx_data = midi_rxd_controlchange;
	(void)dummy;
}

#include "neutr_lfo_common.h" //extern volatile uint16_t nl2ahb_breath;

void mcc_dat_breath_h(uint8_t dat)
{
	lforam.nl2ahb_breath &= HCLR;
	lforam.nl2ahb_breath |= dat<<7;
	//modwheel(mp1_modwh);
	midi_rx_data = midi_rxd_controlchange;
}
void mcc_dat_breath_l(uint8_t dat)
{
	lforam.nl2ahb_breath &= LCLR;
	lforam.nl2ahb_breath |= dat;
	//modwheel(mp1_modwh);
	midi_rx_data = midi_rxd_controlchange;
}
//**********************************************************************************************************
//
typedef enum
{
	MCC__DEFAULT = 0,
	MCC_MODWHL_H,
	MCC_MODWHL_L,
	MCC_DMPRPEDL,
	MCC_ALLSNDOF,
	MCC_ALLNTSOF,
	MCC_BREATH_H,
	MCC_BREATH_L,
}midi_noncc_en;

void (*const ccn[])(uint8_t dat) =      // KEEP IN SYNC THEESE TABLES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
{
		midi_rxd_controlchange2,   // MCC__DEFAULT
		mcc_dat_mw_h,              // MCC_MODWHL_H
		mcc_dat_mw_l,              // MCC_MODWHL_L
		mcc_dat_damperpedal,       // MCC_DMPRPEDL
		mcc_allsoundoff,           // MCC_ALLSNDOF
		mcc_allnotesoff,           // MCC_ALLNTSOF
		mcc_dat_breath_h,
		mcc_dat_breath_l,
};

const uint8_t cc_table[128] =
{
		MCC__DEFAULT, MCC_MODWHL_H, MCC_BREATH_H, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //   0  0x00
		MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //   8  0x08
		MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //  16  0x10
		MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //  24  0x18

		MCC__DEFAULT, MCC_MODWHL_L, MCC_BREATH_L, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //  32  0x20
		MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //  40  0x28
		MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //  48  0x30
		MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //  56  0x38

		MCC_DMPRPEDL, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //  64  0x40
		MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //  72  0x48
		MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //  80  0x50
		MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //  88  0x58

		MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   //  96  0x60
		MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   // 104  0x68
		MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   // 112  0x70
		MCC_ALLSNDOF, MCC__DEFAULT, MCC__DEFAULT, MCC_ALLNTSOF, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT, MCC__DEFAULT,   // 120  0x78
};





//**********************************************************************************************************
//system handlers
void sbh_na()
{
	;
}


void sbh_rt_active_sense()                          //0xFE
{
	;
}




//**********************************************************************************************************
//databytes handlers
void midi_rxd_noteoff(uint8_t d)                //number
{
	midi_temp = d;
	midi_rx_data = midi_rxd_noteoff2;
}
void midi_rxd_noteoff2(uint8_t d)               //velocity
{
	note_t note;
	note.note = midi_temp;
	note.velo = d;
	noteoff(note);
	midi_rx_data = midi_rxd_noteoff;
}

void midi_rxd_noteon(uint8_t d)                 //number
{
	midi_temp = d;
	midi_rx_data = midi_rxd_noteon2;
}
void midi_rxd_noteon2(uint8_t d)                //velocity
{
	note_t note;
	note.note = midi_temp;
	note.velo = d;
	if (d)
		noteon(note);
	else
		noteoff(note);
	midi_rx_data = midi_rxd_noteon;
}

void midi_rxd_controlchange(uint8_t d)          //cc number
{
	midi_temp = d;
	midi_rx_data = ccn[cc_table[d]];
}
void midi_rxd_controlchange2(uint8_t d)         //cc value
{
	//do nothing
	midi_rx_data = midi_rxd_controlchange;
	(void)d;
}

extern volatile uint8_t pp_aftertouch;

void midi_rxd_aftertouch(uint8_t d)             //pressure
{
	pp_aftertouch = d;
}
//

void midi_rxd_pitchwheel(uint8_t d)             //lsb
{
	midi_temp = d;
	midi_rx_data = midi_rxd_pitchwheel2;
}
void midi_rxd_pitchwheel2(uint8_t d)            //msb
{
	int16_t pw = midi_temp;
	pw |= d<<7;
	//pp_pitchwheel = pw - 0x2000;
	pp_pitchwheel(pw - 0x2000);
	midi_rx_data = midi_rxd_pitchwheel;
}

void midi_rxd_noa(uint8_t d)
{
	(void)d;
}


//oh. all fun stuff deleted

