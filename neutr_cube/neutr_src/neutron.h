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
 * neutron.h
 *
 *  Created on: 30 но€б. 2018 г.
 */

#ifndef NEUTRON_H_
#define NEUTRON_H_


//OPTS
#define POT_MAX_VALUE  0x0F00
#define ADC_FILTER_E   12
#define LEDSPWMBDPTH   5


//STM32F0xx uses 2 Bits for the Priority Levels  DO NOT TOUCH THIS!!!!!!!!!!!!!!
#define IRQ_PRIORITY_LFOSYNC  0x0
#define IRQ_PRIORITY_SR       0x1
#define IRQ_PRIORITY_CR       0x3
#define IRQ_PRIORITY_USB      0x2
#define IRQ_PRIORITY_MIDI     0x2
 //Definitive guide to ARM book definitely sucks
 //as is ARM libraries without detailed explanation

#define TUNE_CF_MAX 0xA666
#define TUNE_CF_MIN 0x4CCC

#define MIDI_TX_BUFSIZE 64

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#include "stm32f072xb.h"
#include "stdint.h"

typedef enum
{
	CV_OSC1FINE = 0,
	CV_OSC2FINE,
	CV_OSC1COARSE,
	CV_OSC2COARSE,
	CV_FILTERCUT,
	CV_ASSIGN,
	CV_OSC1WAVE,
	CV_OSC2WAVE,
	CV_TOTAL
}cvch_en;

typedef enum
{
	POT_OSC1SHAPE = 0,
	POT_OSC2SHAPE,
	POT_LFOSHAPE,
	POT_LFORATE,
	POT_GLIDE,
	POT_TOTAL
}adcch_en;

typedef enum
{
	KNOB_LFORATE = 0,
	KNOB_LFOSHAPE,
	KNOB_PORTA,
	KNOB_TOTAL
}knobselect_en;

typedef enum
{
	BTNEN_O1RANGE = 0,
	BTNEN_O2RANGE,
	BTNEN_OSCSYNC,
	BTNEN_PARAPH,
	BTNEN_LFOSYNC,
	BTNEN_FLTMODE,
	BTNEN_FLTKEYTRK,
	BTNEN_O1RANGE_OFF,
	BTNEN_O2RANGE_OFF,
	BTNEN_OSCSYNC_OFF,
	BTNEN_PARAPH_OFF,
	BTNEN_LFOSYNC_OFF,
	BTNEN_FLTMODE_OFF,
	BTNEN_FLTKEYTRK_OFF,
	BTNEN_TOTAL
}buttons_en;

typedef enum
{
	TUNEM_NORMAL = 0,
	TUNEM_O1OCT,
	TUNEM_O1P,
	TUNEM_O1OFF,
	TUNEM_O2OCT,
	TUNEM_O2P,
	TUNEM_O2OFF,
	TUNEM_FLT,
	TUNEM_AS,
	TUNEM_TOTAL
}tunemode_en;

#define BTN_OSC1RANGE      0x00000001
#define BTN_OSC2RANGE      0x00000002
#define BTN_OSCSYNC        0x00000004
#define BTN_PARAPHONIC     0x00000008
#define BTN_LFOSYNC        0x00000010
#define BTN_FILTERMODE     0x00000020
#define BTN_FILTERKEYTRACK 0x00000040

#define BTN_MIDISEL0       0x00000100
#define BTN_MIDISEL1       0x00000200
#define BTN_MIDISEL2       0x00000400
#define BTN_MIDISEL3       0x00000800
#define BTN_MIDISEL        0x00000F00

//ser out
#define LEDBTN_FLTKEYTRK   0x00000080
#define LED_LFO5RAMPUP     0x00000040
#define LED_LFO4SQUARE     0x00000020
#define LED_LFO3RAMPDWN    0x00000010
#define LED_LFO2TRIANGLE   0x00000008
#define LED_LFO1SINE       0x00000004
#define LEDBTN_LFOSYNC     0x00000002
#define LED_FILTERLP       0x00000001

#define LED_FILTERHP       0x80000000
#define LED_FILTERBP       0x40000000
#define LEDBTN_OSC2RANGE   0x20000000
#define LEDBTN_OSC1RANGE   0x10000000
#define LEDBTN_PARAPHONIC  0x08000000
#define LEDBTN_OSCSYNC     0x04000000
#define LED_OSC2_32        0x02000000
#define LED_OSC2_16        0x01000000

#define LED_OSC2_8         0x00800000
#define LED_OSC1_32        0x00400000
#define LED_OSC1_16        0x00200000
#define LED_OSC1_8         0x00100000
#define OSC2_FULLRANGE     0x00080000
#define OSC1_FULLRANGE     0x00040000
#define FILTERMODE_1       0x00020000
#define FILTERMODE_2       0x00010000

//GPIO
#define DAC_PORT           GPIOC->BSRR
#define DAC_DISABLE        0x00004000
#define DAC_ENABLE         0x40000000

#define DACDIR_PORT        GPIOB->BSRR
#define DACDIR_PITCH_1     0x68000000
#define DACDIR_PITCH_2     0x48002000
#define DACDIR_OCT_1       0x28004000
#define DACDIR_OCT_2       0x08006000
#define DACDIR_FILTER      0x60000800
#define DACDIR_ASSIGN      0x40002800
#define DACDIR_WAVE_1      0x20004800
#define DACDIR_WAVE_2      0x00006800

#define OSCWAVEPORT        GPIOB->BSRR
#define OSC1WAVE1          0x00600000
#define OSC1WAVE2          0x00400020
#define OSC1WAVE3          0x00200040
#define OSC1WAVE4          0x00000060
#define OSC2WAVE1          0x01800000
#define OSC2WAVE2          0x01000080
#define OSC2WAVE3          0x00800100
#define OSC2WAVE4          0x00000180

#define SPI165_PORT        GPIOA->BSRR
#define SPI165_EN          0x00008000
#define SPI165_DIS         0x80000000
#define SPI595_PORT        GPIOB->BSRR
#define SPI595_EN          0x10000000
#define SPI595_DIS         0x00001000

#define OUTPUTPORT         GPIOB->BSRR
#define OUTPUTENABLE       0x00000001
#define OUTPUTDISABLE      0x00010000
#define OSCSYNCPORT        GPIOB->BSRR
#define OSCSYNCOFF         0x00000002
#define OSCSYNCON          0x00020000
#define O1MODPORT          GPIOA->BSRR
#define O1MODEN            0x00800000
#define O1MODDIS           0x00000080
#define O2MODPORT          GPIOC->BSRR
#define O2MODEN            0x80000000
#define O2MODDIS           0x00008000
#define GATEPORT           GPIOC->BSRR
#define GATEON             0x00002000
#define GATEOFF            0x20000000

#define LFO_TRIG           0x00000004

#define LFO_OUTPUT DAC1->DHR12L2

//
typedef union
{
	uint16_t data;
	struct
	{
		uint8_t note;
		uint8_t velo;
	};
}note_t;

typedef struct
{
	uint32_t inc;
	uint16_t shift;
	uint16_t recp;
}aatbl_t;

typedef struct
{
	uint16_t divr;
	uint32_t maxmod;
}divrtbl_t;

typedef struct
{
	void (*retrig)(void);
	//void (*midistart)(void);
	//void (*midiclock)(void);
	void (*cr)(void);
	void (*sr)(void);
	void (*lfoleds)(uint32_t time);
	void (*noteon)(uint8_t note, uint32_t ts);
	void (*noteoff)(uint8_t note, uint32_t ts);
	void (*damper)(uint8_t val, uint32_t ts);
	void (*panic)(void);
	//void (*irqsync)(uint8_t type, uint16_t sstimej);
	void (*asyncmain)(void);
	void (*initialize)(uint8_t prevmode);
}neutr_lfo_t;
//tables
extern const uint16_t sine[257];
extern const uint32_t dacdir[8];
extern const uint32_t o1w[5];
extern const uint32_t o2w[5];
extern const divrtbl_t syncdivr[32];
extern const aatbl_t lfoinc[275];
extern const uint8_t sqrttbl[256];
extern const uint32_t glinc[276];
extern const aatbl_t oscinc[257];

//hal init
void peripherial_init(void);
void neutr_main(void);
void global_debug(uint16_t source);
extern volatile uint32_t debug_sr_maxtime;
extern volatile uint32_t debug_cr_maxtime;
#define DEB_CR_START   0x10
#define DEB_CR_PPP     0x1C
#define DEB_CR_END     0x1F
#define DEB_SR_START   0x20
#define DEB_SR_HWESS   0x21
#define DEB_SR_END     0x2F
#define DEB_EI_BANG    0xFF

//midi
//void midi_check_buffer(void);

//input proc
void buttonpress(uint16_t change);
void noteon(note_t note);
void noteoff(note_t note);
void damper(uint8_t value);
void eventscheck(void);

//neutron lfo
extern volatile neutr_lfo_t* neutron_lfo;
void lfo_retrig(void);
void midiclock(void);
void midistart(void);

//voice alloc
void va_noteon(note_t note);
void va_noteoff(note_t note);
void va_panic(void);
void va_damper(uint8_t val);
void va_setmode(uint8_t mode);

//proc pitch
void pp_pitchwheel(int16_t value);
void pp_seto1(uint8_t note, uint8_t glide);
void pp_seto2(uint8_t note, uint8_t glide);
void cr_pitchprocess(void);
void pp_octaveo1(uint8_t octave);
void pp_octaveo2(uint8_t octave);
void pp_setdac(uint8_t mode);

//autotune
void ft_o1tune(void);
void ft_o2tune(void);
void ft_delay(uint32_t time_ms);

//nvm
void nvm_initialize(void);
uint8_t load_parameters(void);
uint8_t save_parameters(void);

#endif /* NEUTRON_H_ */
