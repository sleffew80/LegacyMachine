/*
* LegacyMachine - A libRetro implementation for creating simple lo-fi
* frontends intended to simulate the look and feel of the classic
* video gaming consoles, computers, and arcade machines being emulated.
*
* Copyright (C) 2022-2024 Steven Leffew
* All rights reserved
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
* */

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "InputDriver.h"
#include "../MainEngine.h"

/**************************************************************************************************
 * InputDriver Context Array
 *************************************************************************************************/

const InputDriver* input_drivers[] = {
#if defined HAVE_SDL2
	&sdl2_input_driver,
#endif  
	NULL
};

/**************************************************************************************************
 * InputDriver Initialization
 *************************************************************************************************/

/* Initialize the input driver. */
InputDriver* InitializeInputDriver(void)
{
	return (InputDriver*)input_drivers[0];
}

/* Get the state of a given player's joypad. */
JoypadInputState* GetJoypadInputState(LMC_Player player)
{
	return &legacy_machine->input->joypad->state[player];
}

/* Marks input as pressed. */
void SetInput(LMC_Player player, LMC_Input input)
{
	legacy_machine->input->joypad->state->inputs |= (1 << input);
	legacy_machine->input->last_input = input;
}

/* Marks input as unpressed. */
void ClearInput(LMC_Player player, LMC_Input input)
{
	legacy_machine->input->joypad->state->inputs &= ~(1 << input);
}