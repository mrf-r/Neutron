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
* idk when this was created
*/
#include "main.h"
#include "stm32f072xb.h"
#include "stm32f0xx_hal_flash.h"

#include "menu.h"

//save to last page - page 63
// #define SAVEADDRESS ((uint32_t)0x0801F800)

//page 57
#define SAVEADDRESS ((uint32_t)0x0801C800)
#define VALIDKEY 0x1337AFCD

volatile __aligned(4) parameters_t parameters;

void nvm_initialize()
{
	parameters.o1tunecf = 0x8A00;
	parameters.o1tuneoctcf = 0xA400;
	parameters.o1tuneoff = 0x5100;
	parameters.o2tunecf = 0x8A00;
	parameters.o2tuneoctcf = 0xA400;
	parameters.o2tuneoff = 0x5100;
	parameters.o1oct = 1;
	parameters.o2oct = 1;
	parameters.o1pwrange = 2;
	parameters.o2pwrange = 2;
	parameters.o1pitch = 12;
	parameters.o2pitch = 19;
	parameters.o1widerange = 1;
	parameters.o2widerange = 1;
	parameters.o1moddis = 0;
	parameters.o2moddis = 0;
	parameters.o1glide = 0;
	parameters.o2glide = 0;
	parameters.osync = 0;
	parameters.paraphony = 0;
	parameters.flttunecf = 0x8500;
	parameters.filtertype = 0;
	parameters.filterkeytrack = 0;
	parameters.astunecf = 0x8500;
	parameters.astuneoff = 0x0200;
	parameters.asmode = 2;
	parameters.gateretrig = 0;
	parameters.lfokeysync = 0;
	parameters.lfoextsynctype = 0;
	parameters.lfomidisync = 0;
	parameters.lfosyncstart = 0;
	//parameters.lfokeytrack = 0;
	parameters.lfomode = 0;

	parameters.validity_key = VALIDKEY;

	if (save_parameters())
	{
		//fail
		while(1){;}
	}
}


uint8_t load_parameters()
{
	uint8_t *flash_ptr = (uint8_t*)SAVEADDRESS;
	uint8_t *dest_ptr = (uint8_t*)&parameters;//parameters;
	uint8_t i;

	//check
	parameters_t* pp = (parameters_t*)SAVEADDRESS;
	if (pp->validity_key != VALIDKEY)
	{
		nvm_initialize();
	}
	else
	{
		//just load
		for (i = 0; i<sizeof(parameters_t); i++)
		{
			*dest_ptr++ = *flash_ptr++;
		}
	}


	//do we need to check any? nah..
	return 0;
}


uint8_t save_parameters()
{
	uint32_t dummy;
	uint32_t saveerror = 0;
	//uint32_t addr;
	uint32_t i;
	uint8_t savesize = (sizeof(parameters_t) + 3)/4;
	uint32_t *source_ptr = (uint32_t*)&parameters;
	uint32_t *flash_ptr = (uint32_t*)SAVEADDRESS;

	FLASH_EraseInitTypeDef EraseInitStruct =	{FLASH_TYPEERASE_PAGES,SAVEADDRESS,1};

	//1 - unlock
	HAL_FLASH_Unlock();
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &dummy) != HAL_OK)
	{
		saveerror = 1;
	}

	//2 - write

	for (i = 0; i<savesize; i++)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)flash_ptr, *source_ptr) != HAL_OK)
		{
			saveerror = 2;
		}
		flash_ptr++;
		source_ptr++;
	}

	HAL_FLASH_Lock();

	//3 - check

	source_ptr = (uint32_t*)&parameters;
	flash_ptr = (uint32_t*)SAVEADDRESS;
	for (i = 0; i<savesize; i++)
	{
		if (*flash_ptr++ != *source_ptr++)
		{
			saveerror = 3;
		}
	}

	return saveerror;
}

