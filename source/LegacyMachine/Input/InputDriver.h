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

#ifndef _INPUT_DRIVER_H
#define _INPUT_DRIVER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"

/**************************************************************************************************
 * Definitions
 *************************************************************************************************/
#define MAX_PLAYERS		   8	/* Number of unique players. */
#define MAX_INPUTS		  32	/* Number of inputs per player. */
#define MAX_HATS		   2	/* Number of hats per player. */
#define MAX_AXES		   8	/* Number of axes per player. */
#define INPUT_MASK	(MAX_INPUTS - 1)

/**************************************************************************************************
 * JoypadInputState Structure
 *************************************************************************************************/
typedef struct JoypadInputState
{
	uint32_t key_map[MAX_INPUTS];
	uint32_t inputs;
	int32_t product;
	int32_t vendor;
	uint8_t button_map[MAX_INPUTS];
	uint8_t hat_map[MAX_HATS][MAX_HAT_INPUTS];
	uint8_t axis_map[MAX_AXES][MAX_AXIS_INPUTS];
	uint8_t buttons;
	uint8_t axes;
	uint8_t hats;
	uint8_t identifier;
	const char* name;
	bool keyboard_enabled;
	bool connected;
}
JoypadInputState;

/**************************************************************************************************
 * JoypadDriver Structure
 *************************************************************************************************/
typedef struct JoypadDriver
{
	void			 (*cb_init)(void);
	void			 (*cb_process_key)(LMC_Player, int32_t, uint8_t);
	void			 (*cb_process_button)(LMC_Player, uint8_t, uint8_t);
	void			 (*cb_process_hat)(LMC_Player, uint8_t, uint8_t);
	void			 (*cb_process_axis)(LMC_Player, uint8_t, int);
	void			 (*cb_poll)(void);
	int16_t			 (*cb_get_state)(void);
	void			 (*cb_assign_player)(LMC_Player, int);
	void			 (*cb_connect)(LMC_Player);
	void			 (*cb_disconnect)(LMC_Player);
	void			 (*cb_deinit)(void);
	JoypadInputState state[MAX_PLAYERS];
	bool			 initialized;
}
JoypadDriver;

/**************************************************************************************************
 * InputDriver Structure
 *************************************************************************************************/
typedef struct InputDriver
{
	JoypadDriver* joypad;
	void		(*cb_init)(void);
	void		(*cb_poll)(void);
	int16_t		(*cb_get_state)(unsigned, unsigned, unsigned, unsigned);
	void		(*cb_deinit)(void);
	void		(*cb_auto_config)(void);
	int			last_input;
	int			last_key;
	bool		initialized;
}
InputDriver;

/**************************************************************************************************
 * InputDriver Array
 *************************************************************************************************/

extern const InputDriver* input_drivers[];

/**************************************************************************************************
 * InputDriver Contexts
 *************************************************************************************************/

extern InputDriver sdl2_input_driver;

/**************************************************************************************************
 * InputDriver Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

InputDriver* InitializeInputDriver(void);
void SetInput(LMC_Player player, LMC_Input input);
void ClearInput(LMC_Player player, LMC_Input input);

RETRO_END_DECLS

#endif