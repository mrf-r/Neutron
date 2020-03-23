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
 * neutr_exticalibration.c
 *
 *  Created on: Jul 31, 2019
 */


//delete this file.

#include "neutron.h"

void exticalib(void)
{

	//initialize timer
	RCC->APB1ENR &= ~RCC_APB1ENR_TIM2EN;
	RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	//config tim 2 ch4 pwm out
	TIM2->ARR = 0x1000;
	TIM2->CCR4 = 0x1000;
	TIM2->CCMR2 = TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;
	TIM2->CCER = TIM_CCER_CC4E;
	TIM2->EGR = TIM_EGR_UG;
	TIM2->SR &= ~TIM_SR_UIF;
	TIM2->CNT = 0xFFFFFF00;

	//config gpioB11 altmode
	GPIOB->AFR[1] |= 0x00002000;
	GPIOB->MODER &= ~GPIO_MODER_MODER11;//affect main pitch a little, sorry
	GPIOB->MODER |= GPIO_MODER_MODER11_1; //reconfigure to alt

	//configure PB02 external interrupt
	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI11_PB;
	EXTI->IMR |= EXTI_IMR_IM11;
	EXTI->RTSR |= EXTI_RTSR_RT11;
	//EXTI->FTSR |= EXTI_FTSR_FT11;

	EXTI->PR |= EXTI_PR_PIF11;
	//

	NVIC_SetPriority(EXTI4_15_IRQn,IRQ_PRIORITY_LFOSYNC);
	NVIC_EnableIRQ(EXTI4_15_IRQn);


	TIM2->CR1 |= TIM_CR1_CEN;
	while (!(TIM2->SR & TIM_SR_UIF))
	{
		;//wait
	}
	//ft_delay(100);

	//reset all
	GPIOB->AFR[1] &= ~0x0000F000;
	GPIOB->MODER &= ~GPIO_MODER_MODER11;
	GPIOB->MODER |= GPIO_MODER_MODER11_0; //reconfigure to output

	RCC->APB1ENR &= ~RCC_APB1ENR_TIM2EN;
	RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	SYSCFG->EXTICR[2] &= ~SYSCFG_EXTICR3_EXTI11_PB;
	EXTI->IMR &= ~EXTI_IMR_IM11;
	EXTI->RTSR &= ~EXTI_RTSR_RT11;
	EXTI->FTSR &= ~EXTI_FTSR_FT11;

	TIM2->CR1 = TIM_CR1_CEN;

}
extern volatile uint32_t exti_timcomp;

void not_EXTI4_15_IRQHandler()
{
	int32_t tpos = TIM2->CNT;
	exti_timcomp = tpos;

	EXTI->PR |= EXTI_PR_PIF11;

	NVIC_DisableIRQ(EXTI4_15_IRQn);

	global_debug(DEB_EI_BANG);
}





