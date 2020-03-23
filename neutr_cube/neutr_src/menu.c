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
 * menu.c
 *
 *  Created on: 19 февр. 2019 г.
 */
#include "neutron.h"
#include "menu.h"


volatile uint8_t tunemode = 0;
volatile int8_t shiftvalue = 0;

volatile uint8_t o1widerange;
volatile uint8_t o2widerange;


const menu_page_t mspage_null =
{
	0, //BTNEN_O1RANGE
	0, //BTNEN_O2RANGE
	0, //BTNEN_OSCSYNC
	0, //BTNEN_PARAPH
	0, //BTNEN_LFOSYNC
	0, //BTNEN_FLTMODE
	0, //BTNEN_FLTKEYTRK
	0, //BTNEN_O1RANGE_OFF
	0, //BTNEN_O2RANGE_OFF
	0, //BTNEN_OSCSYNC_OFF
	0, //BTNEN_PARAPH_OFF
	0, //BTNEN_LFOSYNC_OFF
	0, //BTNEN_FLTMODE_OFF
	0, //BTNEN_FLTKEYTRK_OFF
};

void (*lfoledproc)(uint32_t frame) = 0;

volatile menu_page_t page =
{
	0, //BTNEN_O1RANGE
	0, //BTNEN_O2RANGE
	0, //BTNEN_OSCSYNC
	0, //BTNEN_PARAPH
	0, //BTNEN_LFOSYNC
	0, //BTNEN_FLTMODE
	0, //BTNEN_FLTKEYTRK
	0, //BTNEN_O1RANGE_OFF
	0, //BTNEN_O2RANGE_OFF
	0, //BTNEN_OSCSYNC_OFF
	0, //BTNEN_PARAPH_OFF
	0, //BTNEN_LFOSYNC_OFF
	0, //BTNEN_FLTMODE_OFF
	0, //BTNEN_FLTKEYTRK_OFF
};


void ms_empty(void)
{
	;
}

