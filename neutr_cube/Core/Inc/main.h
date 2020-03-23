/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* USER CODE BEGIN Includes */
#define USBD_AUDIO_FREQ 48000
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define GATE_OUT_Pin GPIO_PIN_13
#define GATE_OUT_GPIO_Port GPIOC
#define DACM_EN_Pin GPIO_PIN_14
#define DACM_EN_GPIO_Port GPIOC
#define OSC2_MODEN_Pin GPIO_PIN_15
#define OSC2_MODEN_GPIO_Port GPIOC
#define ADC_SHAPE1_Pin GPIO_PIN_0
#define ADC_SHAPE1_GPIO_Port GPIOA
#define ADC_SHAPE2_Pin GPIO_PIN_1
#define ADC_SHAPE2_GPIO_Port GPIOA
#define ADC_LFOSHAPE_Pin GPIO_PIN_2
#define ADC_LFOSHAPE_GPIO_Port GPIOA
#define ADC_LFOFREQ_Pin GPIO_PIN_3
#define ADC_LFOFREQ_GPIO_Port GPIOA
#define DAC_M_Pin GPIO_PIN_4
#define DAC_M_GPIO_Port GPIOA
#define DAC_LFO_Pin GPIO_PIN_5
#define DAC_LFO_GPIO_Port GPIOA
#define ADC_GLIDE_Pin GPIO_PIN_6
#define ADC_GLIDE_GPIO_Port GPIOA
#define OSC1_MODEN_Pin GPIO_PIN_7
#define OSC1_MODEN_GPIO_Port GPIOA
#define OUTPUT_EN_Pin GPIO_PIN_0
#define OUTPUT_EN_GPIO_Port GPIOB
#define OSCSYNC_DIS_Pin GPIO_PIN_1
#define OSCSYNC_DIS_GPIO_Port GPIOB
#define LFOTRIG_IN_Pin GPIO_PIN_2
#define LFOTRIG_IN_GPIO_Port GPIOB
#define OUT595_CLK_Pin GPIO_PIN_10
#define OUT595_CLK_GPIO_Port GPIOB
#define DACM_SEL2_Pin GPIO_PIN_11
#define DACM_SEL2_GPIO_Port GPIOB
#define OUT595_LATCH_Pin GPIO_PIN_12
#define OUT595_LATCH_GPIO_Port GPIOB
#define DACM_SEL0_Pin GPIO_PIN_13
#define DACM_SEL0_GPIO_Port GPIOB
#define DACM_SEL1_Pin GPIO_PIN_14
#define DACM_SEL1_GPIO_Port GPIOB
#define OUT595_MOSI_Pin GPIO_PIN_15
#define OUT595_MOSI_GPIO_Port GPIOB
#define OSC1TUNE_IN_Pin GPIO_PIN_8
#define OSC1TUNE_IN_GPIO_Port GPIOA
#define IN165_LATCH_Pin GPIO_PIN_15
#define IN165_LATCH_GPIO_Port GPIOA
#define IN165_CLK_Pin GPIO_PIN_3
#define IN165_CLK_GPIO_Port GPIOB
#define IN165_MISO_Pin GPIO_PIN_4
#define IN165_MISO_GPIO_Port GPIOB
#define OSC1WFSEL0_Pin GPIO_PIN_5
#define OSC1WFSEL0_GPIO_Port GPIOB
#define OSC1WFSEL1_Pin GPIO_PIN_6
#define OSC1WFSEL1_GPIO_Port GPIOB
#define OSC2WFSEL0_Pin GPIO_PIN_7
#define OSC2WFSEL0_GPIO_Port GPIOB
#define OSC2WFSEL1_Pin GPIO_PIN_8
#define OSC2WFSEL1_GPIO_Port GPIOB
#define OSC2TUNE_IN_Pin GPIO_PIN_9
#define OSC2TUNE_IN_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
