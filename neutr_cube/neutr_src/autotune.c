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

 * autotune.c
 *
 *  Created on: May 13, 2019
 */



#include "neutron.h"
#include "menu.h"
#include "math.h"

//#define ARM_MATH_CM0
// #include "arm_math.h"

typedef double ft_real_t; //you can calculate in double

#define FT_FREQ_PRECISION 32
#define FT_A_VAL 0x3000
#define FT_B_VAL 0xC000
#define FT_MID_VAL 0x9000
const ft_real_t distance = ((FT_B_VAL - FT_A_VAL)*64.0);

#define DAC_O1PITCH dac_output[CV_OSC1COARSE]
#define DAC_O1OCTAVE dac_output[CV_OSC1FINE]

#define DAC_O2PITCH dac_output[CV_OSC2COARSE]
#define DAC_O2OCTAVE dac_output[CV_OSC2FINE]

#define FT_CORE_CLK   48000000.0f
#define FT_SAMPLERATE 3000
#define FT_TIMEOUT     12000
//12000 = 4sec


volatile uint32_t ft_time;
//uint16_t o1freq[FT_FREQ_PRECISION+1];
//uint16_t o2freq[FT_FREQ_PRECISION+1];

uint16_t ft_freq[FT_FREQ_PRECISION+1];



//volatile uint16_t coefs[6];




extern uint16_t dac_output[CV_TOTAL];
extern volatile uint8_t pp_tunemode;










ft_real_t pitch2freq(ft_real_t pitch)
{
	ft_real_t t = (pitch - 69.0)/440.0;
	t= 440.0 * pow(2, t);
	return t;
}

ft_real_t freq2pitch(ft_real_t freq)
{
	ft_real_t t = freq/440.0;
	t = log2(t);
	t = (t * 12.0) + 69;
	return t;
}



void captime()
{
	ft_time = 0;
}

uint8_t timeout()
{
	if (ft_time < FT_TIMEOUT)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}




void ft_delay(uint32_t time_ms)
{
	//3kHz equal to 0.333 mS delay sample
	ft_time = 0;
	uint32_t nexttime = time_ms * FT_SAMPLERATE / 1000;
	if (nexttime < 2)
		nexttime = 2;
	while(ft_time < nexttime)
	{
		;
	}
}





////////////////////////////////////////////////////////////////////////////////////
// oscillator tuning code
// this code is extremely precise and can be used for calibration algorithm only
// if you want to realtime measure frequency, then left pre to 0x3F and do that without dma just using flags


/*
 * precision tuning - from 5000 to 40000 counts
 * timer prescalers:
 * 0x3F - from 18.75 to 150 Hz
 * 0x7  - from 150 to 1200 Hz
 * 0x0  - from 1200 to 9600 Hz
 *
 * if ccr<5000 then psc = next and repeat
 * if uif or >40000 then psc = prev
 *
 */

/*
 * presca      64       8       1
 *
 * period
 *     625   1200    9600   76800
 *    5000    150    1200    9600
 *   40000  18,75     150    1200
 *   65535  11,44   91,55   732,4
 * freq
 *   20000   37,5     300    2400
 *    9600  78,12     625    5000
 *    1200    625    5000   40000
 *     150   5000   40000  320000
 */

#define FT_LOWFREQPRE 0x3F
#define FT_MIDFREQPRE 0x7
#define FT_HIFREQPRE  0x0


inline uint16_t tim1capround()
{
	TIM1->SR &= ~(TIM_SR_CC1IF | TIM_SR_UIF);
	while (!(TIM1->SR & TIM_SR_CC1IF))
	{
		;
	}
	return TIM1->CCR1;
}

inline uint16_t tim17capround()
{
	TIM17->SR &= ~(TIM_SR_CC1IF | TIM_SR_UIF);
	while (!(TIM17->SR & TIM_SR_CC1IF))
	{
		;
	}
	return TIM17->CCR1;
}

void ft_o1setpre()
{
	uint16_t freq, g1,g2;
	TIM1->PSC = 0x3F;
	TIM1->EGR = TIM_EGR_UG;
	TIM1->SR &= ~(TIM_SR_UIF | TIM_SR_CC1IF);
	tim1capround();
	g1 = tim1capround();
	g2 = tim1capround();
	freq = g2 - g1;
	if (freq < 5000)
	{
		if (freq < 625)
		{
			//swith max freq mode
			TIM1->PSC = FT_HIFREQPRE;
		}
		else
		{
			//switch mid freq mode
			TIM1->PSC = FT_MIDFREQPRE;
		}
	}
	TIM1->EGR = TIM_EGR_UG;
	tim1capround();
}

void ft_o2setpre()
{
	uint16_t freq, g1,g2;
	TIM17->PSC = 0x3F;
	TIM17->EGR = TIM_EGR_UG;
	TIM17->SR &= ~(TIM_SR_UIF | TIM_SR_CC1IF);
	tim17capround();
	g1 = tim17capround();
	g2 = tim17capround();
	freq = g2 - g1;
	if (freq < 5000)
	{
		if (freq < 625)
		{
			//swith max freq mode
			TIM17->PSC = FT_HIFREQPRE;
		}
		else
		{
			//switch mid freq mode
			TIM17->PSC = FT_MIDFREQPRE;
		}
	}
	TIM17->EGR = TIM_EGR_UG;
	tim17capround();
}


ft_real_t ft_o1capture()
{
	uint8_t i;
	uint32_t cap = 0;
	ft_real_t result;

	ft_o1setpre();
	DMA1_Channel6->CCR &= ~DMA_CCR_EN;
	DMA1->IFCR = DMA_IFCR_CTCIF6;
	DMA1_Channel6->CNDTR = FT_FREQ_PRECISION+1;
	DMA1_Channel6->CCR |= DMA_CCR_EN;
	captime();
	while(!(DMA1->ISR & DMA_ISR_TCIF6))
	{
		if (timeout())
		{
			return 0.0f;
		}
	}
	//that's it!
	for(i = 0; i<FT_FREQ_PRECISION; i++)
	{
		cap += (uint16_t)(ft_freq[i+1] - ft_freq[i]);
	}

	result = (FT_CORE_CLK * FT_FREQ_PRECISION) / (cap * (TIM1->PSC + 1));
	return result;
}


ft_real_t ft_o2capture()
{
	uint8_t i;
	uint32_t cap = 0;
	ft_real_t result;

	ft_o2setpre();
	DMA1_Channel7->CCR &= ~DMA_CCR_EN;
	DMA1->IFCR = DMA_IFCR_CTCIF7;
	DMA1_Channel7->CNDTR = FT_FREQ_PRECISION+1;
	DMA1_Channel7->CCR |= DMA_CCR_EN;
	captime();
	while(!(DMA1->ISR & DMA_ISR_TCIF7))
	{
		if (timeout())
		{
			return 0.0f;
		}
	}
	//that's it!
	for(i = 0; i<FT_FREQ_PRECISION; i++)
	{
		cap += (uint16_t)(ft_freq[i+1] - ft_freq[i]);
	}

	result = (FT_CORE_CLK * FT_FREQ_PRECISION) / (cap * (TIM17->PSC + 1));
	return result;
}

void ft_o1tune()
{
	ft_real_t pitch_a, pitch_b;
	pp_tunemode = 0xFF;
	//disable notes

	//disable mod
	O1MODPORT = O1MODDIS;


	//set octave mid freq
	DAC_O1OCTAVE = FT_MID_VAL;
	//find note
	DAC_O1PITCH = FT_A_VAL;
	ft_delay(50);
	ft_o1setpre();
	pitch_a = freq2pitch(ft_o1capture());


	DAC_O1PITCH = FT_B_VAL;
	ft_delay(50);
	ft_o1setpre();
	pitch_b = freq2pitch(ft_o1capture());

	parameters.o1tunecf = (uint16_t)lround(distance/(pitch_b - pitch_a));
	//coefs[1] = (uint16_t)lround(distance/(pitch_b - pitch_a));

	//set note mid freq
	DAC_O1PITCH = FT_MID_VAL;

	//find octave
	DAC_O1OCTAVE = FT_A_VAL;
	ft_delay(50);
	ft_o1setpre();
	pitch_a = freq2pitch(ft_o1capture());

	DAC_O1OCTAVE = FT_B_VAL;
	ft_delay(50);
	ft_o1setpre();
	pitch_b = freq2pitch(ft_o1capture());


	pitch_b = distance/(pitch_b - pitch_a); //TODO: zero check

	parameters.o1tuneoctcf = (uint16_t)lround(pitch_b);


	//set octave and note
	DAC_O1OCTAVE = (0x1800*parameters.o1tuneoctcf)>>15; //1st octave
	DAC_O1PITCH = (0x8A00*parameters.o1tunecf)>>15; //69 note A440
	ft_delay(50);
	//find offset
	ft_o1setpre();
	pitch_a = freq2pitch(ft_o1capture());

	parameters.o1tuneoff = (uint16_t)lround(pitch_b*(69.0 - pitch_a)/64.0);

	pp_tunemode = 0;
}


void ft_o2tune()
{
	ft_real_t pitch_a, pitch_b;
	pp_tunemode = 0xFF;
	//disable notes

	//disable mod
	OSCSYNCPORT = OSCSYNCOFF;
	O2MODPORT = O2MODDIS;


	//set octave mid freq
	DAC_O2OCTAVE = FT_MID_VAL;
	//find note
	DAC_O2PITCH = FT_A_VAL;
	ft_o2setpre();
	pitch_a = freq2pitch(ft_o2capture());

	DAC_O2PITCH = FT_B_VAL;
	ft_o2setpre();
	pitch_b = freq2pitch(ft_o2capture());

	parameters.o2tunecf = (uint16_t)lround(distance/(pitch_b - pitch_a));





	//set note mid freq
	DAC_O2PITCH = FT_MID_VAL;

	//find octave
	DAC_O2OCTAVE = FT_A_VAL;
	ft_o2setpre();
	pitch_a = freq2pitch(ft_o2capture());

	DAC_O2OCTAVE = FT_B_VAL;
	ft_o2setpre();
	pitch_b = freq2pitch(ft_o2capture());

	pitch_b = distance/(pitch_b - pitch_a);

	parameters.o2tuneoctcf = (uint16_t)lround(pitch_b);


	//set octave and note
	DAC_O2OCTAVE = (0x1800*parameters.o2tuneoctcf)>>15; //1st octave
	DAC_O2PITCH = (0x8A00*parameters.o2tunecf)>>15; //69 note A440
	//find offset
	ft_o2setpre();
	pitch_a = freq2pitch(ft_o2capture());

	parameters.o2tuneoff = (uint16_t)lround(pitch_b*(69.0 - pitch_a)/64.0);

	pp_tunemode = 0;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
// FILTER ?





uint32_t filter_timer[FT_FREQ_PRECISION + 1];
uint8_t ftwp = 0;

ft_real_t ft_filtercapture()
{
	uint8_t i;
	uint32_t cap = 0;
	ft_real_t result;

	ftwp = 0;
	NVIC_EnableIRQ(EXTI2_3_IRQn);
	captime();
	while(ftwp<(FT_FREQ_PRECISION + 1))
	{
		if (timeout())
		{
			return 0.0f;
		}
	}
	//that's it!
	for(i = 0; i<FT_FREQ_PRECISION; i++)
	{
		cap += filter_timer[i+1] - filter_timer[i];
	}

	result = (FT_CORE_CLK / cap) * FT_FREQ_PRECISION;
	return result;
}
/*
extern volatile uint8_t lfo_sync_blink;

void EXTI2_3_IRQHandler_was()
{
	filter_timer[ftwp] = TIM2->CNT;
	ftwp++;
	lfo_sync_blink = 1;
	if (ftwp == (FT_FREQ_PRECISION + 1))
	{
		NVIC_DisableIRQ(EXTI2_3_IRQn);
	}
	EXTI->PR |= EXTI_PR_PIF2;
}


*/










