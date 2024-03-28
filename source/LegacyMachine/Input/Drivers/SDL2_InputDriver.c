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
#include <SDL.h>

#include "../InputDriver.h"
#include "../../MainEngine.h"
#include "../../Logging.h"

SDL_Joystick* joystick[MAX_PLAYERS];

/**************************************************************************************************
 * SDL2 Input Structures
 *************************************************************************************************/

//static JoypadDriver joypad = { 0 };

/**************************************************************************************************
 * Prototypes
 *************************************************************************************************/

/* Local prototypes */

static void SDL2_InitializeJoypad(void);
static void SDL2_DeinitializeJoypad(LMC_Player player);
static void SDL2_ConnectJoypad(LMC_Player player);
static void SDL2_DisconnectJoypad(LMC_Player player);

/**************************************************************************************************
 * SDL Joypad Functions
 *************************************************************************************************/

/* Joypad initialization. */
static bool SDL2_InitializeJoypad(void)
{
	unsigned num_joysticks = 0;
	uint32_t subsystem_flags = SDL_WasInit(0);
	unsigned i;

	/* Initialize input subsystem, if necessary. */
	if (subsystem_flags == 0)
	{
		if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
		{
			lmc_trace(LMC_LOG_ERRORS, "[SDL2]: Failed to initialize input subsystem: %s", SDL_GetError());
			LMC_SetLastError(LMC_ERR_FAIL_INPUT_INIT);
			return false;
		}
	}
	else if ((subsystem_flags & SDL_INIT_JOYSTICK) == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
		{
			lmc_trace(LMC_LOG_ERRORS, "[SDL2]: Failed to initialize input subsystem: %s", SDL_GetError());
			LMC_SetLastError(LMC_ERR_FAIL_INPUT_INIT);
			return false;
		}
	}

	num_joysticks = SDL_NumJoysticks();
	if (num_joysticks > MAX_PLAYERS)
		num_joysticks = MAX_PLAYERS;

	for (i = 0; i < num_joysticks; i++)
		SDL2_ConnectJoypad(i);

	SDL_JoystickEventState(SDL_ENABLE);

	return true;
}

/* Joypad deinitialization. */
static void SDL2_DeinitializeJoypad(LMC_Player player)
{
	JoypadInputState* joypad_state = GetJoypadInputState(player);

	memset(&joypad_state->button_map, 0, MAX_INPUTS);
	joypad_state->name = NULL;
	joypad_state->inputs = 0;
	joypad_state->product = 0;
	joypad_state->vendor = 0;
	joypad_state->buttons = 0;
	joypad_state->axes = 0;
	joypad_state->hats = 0;
	joypad_state->identifier = 0;
	joypad_state->connected = false;
	joystick[player] = NULL;
}

/* Get the human readable text name of a given player's joypad. */
static const char* SDL2_GetJoypadName(LMC_Player player)
{
	if ((player >= MAX_PLAYERS) || (joystick[player] == NULL))
		return NULL;
	return SDL_JoystickNameForIndex(player);
}

/* Initialize a given player's joypad when connected. */
static void SDL2_ConnectJoypad(LMC_Player player)
{
	JoypadInputState* joypad_state = GetJoypadInputState(player);
	bool ok = false;

	joystick[player] = SDL_JoystickOpen(player);
	ok = joystick[player] != NULL;

	if (!ok)
	{
		lmc_trace(LMC_LOG_ERRORS, "Couldn't open joystick #%u: %s\n", player, SDL_GetError());

		if (joystick[player])
			SDL_JoystickClose(joystick[player]);

		joystick[player] = NULL;

		return;
	}

	joypad_state->vendor = SDL_JoystickGetDeviceVendor(player);
	joypad_state->product = SDL_JoystickGetDeviceProduct(player);
	joypad_state->name = SDL2_GetJoypadName(player);

	joypad_state->buttons = SDL_JoystickNumButtons(joystick[player]);
	joypad_state->axes = SDL_JoystickNumAxes(joystick[player]);
	joypad_state->hats = SDL_JoystickNumHats(joystick[player]);

	lmc_trace(LMC_LOG_VERBOSE, "Joypad initialized (name: %s, vendor id: %i, product id: %i, buttons: %i, hats: %i, axes: %i)",
		joypad_state->name, joypad_state->vendor, joypad_state->product, joypad_state->buttons, joypad_state->hats, joypad_state->axes);

	joypad_state->connected = true;
}

/* Deinitialize a given player's joypad when disconnected. */
static void SDL2_DisconnectJoypad(LMC_Player player)
{
	JoypadInputState* joypad_state = GetJoypadInputState(player);

	if (joystick[player])
	{
		SDL_JoystickClose(joystick[player]);
	}
	SDL2_DeinitializeJoypad(player);
	joypad_state->connected = false;
}

/* Assign input to a player's joypad. */
static void SDL2_AssignInputJoypad(LMC_Player player, int index)
{
	JoypadInputState* joypad_state = GetJoypadInputState(player);

	if (joystick[player] != NULL)
	{
		SDL_JoystickClose(joystick[player]);
		joystick[player] = NULL;
	}
	if (index >= 0)
	{
		joystick[player] = SDL_JoystickOpen(index);
		joypad_state->identifier = SDL_JoystickInstanceID(joystick[player]);
	}
}

/* Process keyboard input. */
static void SDL2_ProcessJoypadKeycodeInput(LMC_Player player, int32_t keycode, uint8_t state)
{
	JoypadInputState* joypad_state = GetJoypadInputState(player);
	LMC_Input input = LMC_INPUT_NONE;
	int i;

	/* Search input. */
	for (i = LMC_INPUT_B; i < MAX_INPUTS && input == LMC_INPUT_NONE; i++)
	{
		if (joypad_state->key_map[i] == (SDL_Keycode)keycode)
			input = (LMC_Input)i;
	}

	/* Update. */
	if (input != LMC_INPUT_NONE)
	{
		if (state == SDL_PRESSED)
			SetInput(player, input);
		else
			ClearInput(player, input);
	}
}

/* Process joypad button input. */
static void SDL2_ProcessJoypadButtonInput(LMC_Player player, uint8_t button, uint8_t state)
{
	JoypadInputState* joypad_state = GetJoypadInputState(player);
	LMC_Input input = LMC_INPUT_NONE;
	int i;

	/* Search input. */
	for (i = LMC_INPUT_B; i < MAX_INPUTS && input == LMC_INPUT_NONE; i++)
	{
		if (joypad_state->button_map[i] == button)
			input = (LMC_Input)i;
	}

	/* Update. */
	if (input != LMC_INPUT_NONE)
	{
		if (state == SDL_PRESSED)
			SetInput(player, input);
		else
			ClearInput(player, input);
	}
}

/* Process joypad hat input. */
static void SDL2_ProcessJoypadHatInput(LMC_Player player, uint8_t hat, uint8_t value)
{
	JoypadInputState* joypad_state = GetJoypadInputState(player);

	if (hat < MAX_HATS)
	{
		ClearInput(player, joypad_state->hat_map[hat][LMC_HAT_UP]);
		ClearInput(player, joypad_state->hat_map[hat][LMC_HAT_RIGHT]);
		ClearInput(player, joypad_state->hat_map[hat][LMC_HAT_DOWN]);
		ClearInput(player, joypad_state->hat_map[hat][LMC_HAT_LEFT]);
		if (value != SDL_HAT_CENTERED)
		{
			if (value == SDL_HAT_UP)
				SetInput(player, joypad_state->hat_map[hat][LMC_HAT_UP]);
			else if (value == SDL_HAT_RIGHT)
				SetInput(player, joypad_state->hat_map[hat][LMC_HAT_RIGHT]);
			else if (value == SDL_HAT_DOWN)
				SetInput(player, joypad_state->hat_map[hat][LMC_HAT_DOWN]);
			else if (value == SDL_HAT_LEFT)
				SetInput(player, joypad_state->hat_map[hat][LMC_HAT_LEFT]);
		}
	}
}

/* Process joystic axis input */
static void SDL2_ProcessJoypadAxisInput(LMC_Player player, uint8_t axis, int value)
{
	JoypadInputState* joypad_state = GetJoypadInputState(player);

	if (axis < MAX_AXES)
	{
		ClearInput(player, joypad_state->axis_map[axis][LMC_AXIS_POS]);
		ClearInput(player, joypad_state->axis_map[axis][LMC_AXIS_NEG]);
		if (value > 1000)
			SetInput(player, joypad_state->axis_map[axis][LMC_AXIS_POS]);
		else if (value < -1000)
			SetInput(player, joypad_state->axis_map[axis][LMC_AXIS_NEG]);
	}
}

/* Poll all joypad input. */
static void SDL2_PollJoypadInput(void)
{
	/* All input polling is currently handled in the event handler. */
}

/* Get joypad's state on a given port. */
static int16_t SDL2_JoypadState(unsigned port, unsigned device, unsigned index, unsigned id)
{
	JoypadInputState* joypad_state = GetJoypadInputState((LMC_Player)port);

	if (port >= MAX_PLAYERS)
		return 0;

	return (int16_t)(joypad_state->inputs & (1 << ((id + 1) & INPUT_MASK)));
}

/* Close all joypads. */
static void SDL2_CloseJoypad(void)
{
	unsigned i;
	for (i = 0; i < MAX_PLAYERS; i++)
		SDL2_DisconnectJoypad(i);
}

/* Input initialization. */
static void SDL2_InitializeInput(void)
{
	/* Enable keyboard input for PLAYER 1 by default. */
	LMC_EnableKeyboardAsJoypadInput(LMC_PLAYER1, true);

	/* Default keyboard input mappings for PLAYER 1 */
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_UP, SDLK_UP);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_DOWN, SDLK_DOWN);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_LEFT, SDLK_LEFT);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_RIGHT, SDLK_RIGHT);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_B, SDLK_z);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_A, SDLK_x);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_Y, SDLK_a);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_X, SDLK_s);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_L, SDLK_q);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_R, SDLK_w);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_SELECT, SDLK_RSHIFT);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_START, SDLK_RETURN);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_QUIT, SDLK_ESCAPE);
	LMC_DefineJoypadInputKey(LMC_PLAYER1, LMC_INPUT_CRT, SDLK_BACKSPACE);

	SDL2_InitializeJoypad();
}

/* Poll all input. */
static void SDL2_PollInput(void)
{
	/* All input polling is currently handled in the event handler. */
}

/* Get the input state on a given port. */
static int16_t SDL2_InputState(unsigned port, unsigned device, unsigned index, unsigned id)
{
	switch (device)
	{
	case RETRO_DEVICE_JOYPAD:
		return SDL2_JoypadState(port, device, index, id);
		break;
	}
	return 0;
}

/* Close all input. */
static void SDL2_CloseInput(void)
{
	SDL2_CloseJoypad();
}

/**************************************************************************************************
 * SDL2 Input Driver
 *************************************************************************************************/

JoypadDriver sdl2_joypad_driver = {
	SDL2_InitializeJoypad,
	SDL2_ProcessJoypadKeycodeInput,
	SDL2_ProcessJoypadButtonInput,
	SDL2_ProcessJoypadHatInput,
	SDL2_ProcessJoypadAxisInput,
	SDL2_PollJoypadInput,
	SDL2_JoypadState,
	SDL2_AssignInputJoypad,
	SDL2_ConnectJoypad,
	SDL2_DisconnectJoypad,
	SDL2_CloseJoypad,
	0,
	false
};

InputDriver sdl2_input_driver = {
	&sdl2_joypad_driver,
	SDL2_InitializeInput,
	SDL2_PollInput,
	SDL2_InputState,
	SDL2_CloseInput,
	NULL,
	0,
	0,
	false
};
