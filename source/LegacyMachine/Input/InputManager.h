/*
* LegacyMachine - A Libretro implementation for creating simple lo-fi
* frontends intended to simulate the look and feel of the classic
* video gaming consoles, computers, and arcade machines being emulated.
*
* Copyright (C) 2022-2026 Steven Leffew
* All rights reserved
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
* */

#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"
#include "../StateList.h"
#include "../Common/Common.h"

/**************************************************************************************************
 * Input Definitions
 *************************************************************************************************/

#define INPUT_DRIVER_FIRST (InputDriverID)0

/**************************************************************************************************
 * InputDriver Enumeration
 *************************************************************************************************/

typedef enum
{
#if defined HAVE_SDL
	INPUT_DRIVER_SDL,
#endif
	INPUT_DRIVER_NONE,
	MAX_INPUT_DRIVERS = INPUT_DRIVER_NONE
}
InputDriverID;

/**************************************************************************************************
 * JoypadDriver Structure
 *************************************************************************************************/

typedef struct JoypadDriver
{
	bool	(*cb_init)(void);				/* Initialize joypad driver. */
	void	(*cb_connect)(LMC_Player);		/* Connect a player's joypad device. */
	void	(*cb_disconnect)(LMC_Player);	/* Disconnect a player's joypad device. */
	void	(*cb_deinit)(void);				/* Deinitialize joypad driver. */
}
JoypadDriver;

/**************************************************************************************************
 * KeyboardManager Structure
 *************************************************************************************************/

typedef struct KeyboardManager
{
	KeyboardInputState	state;			/* Current keyboard input state. */
	bool				joypad_input;	/* Whether keyboard is mapped to joypad input. */
	bool				interface_input;/* Whether keyboard is mapped to interface input. */
}
KeyboardManager;

/**************************************************************************************************
 * MouseManager Structure
 *************************************************************************************************/

typedef struct MouseManager
{
	MouseInputState	state[MAX_PLAYERS];		/* Per-player mouse input state. */
}
MouseManager;

/**************************************************************************************************
 * JoypadManager Structure
 *************************************************************************************************/

typedef struct JoypadManager
{
	JoypadInputState	state[MAX_PLAYERS];	/* Per-player joypad input state. */
	bool				active;				/* Whether joypad input is active. */
}
JoypadManager;

/**************************************************************************************************
 * InputDriver Structure
 *************************************************************************************************/

typedef struct InputDriver
{
	bool			(*cb_init)(void);	/* Initialize input driver. */
	void			(*cb_poll)(void);	/* Poll all input devices. */
	void			(*cb_deinit)(void);	/* Deinitialize input driver. */
	JoypadDriver*	joypad;				/* Pointer to joypad sub-driver. */
	InputDriverID	id;					/* Input driver ID. */
}
InputDriver;

/**************************************************************************************************
 * InputManager Structure
 *************************************************************************************************/

typedef struct InputManager
{
	JoypadManager*		joypad;			/* Pointer to joypad manager. */
	KeyboardManager*	keyboard;		/* Pointer to keyboard manager. */
	MouseManager*		mouse;			/* Pointer to mouse manager. */

	/* Callback for joypad auto-configuration. */
	void				(*cb_joypad_auto_config)(LMC_Player, char*, uint32_t, uint32_t);

	int					last_input;		/* Last processed input value. */
	bool				initialized;	/* Whether the input manager is initialized. */
}
InputManager;

/**************************************************************************************************
 * Input Keymap Array
 *************************************************************************************************/

extern const uint32_t input_keymap[];

/**************************************************************************************************
 * InputDriver Array
 *************************************************************************************************/

extern const InputDriver* input_drivers[];

/**************************************************************************************************
 * InputDriver Contexts
 *************************************************************************************************/

extern InputDriver sdl_input_driver;

/**************************************************************************************************
 * Input Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

/* Input Management Prototypes */

InputManager* GetInputManagerContext(void);

KeyboardManager* GetKeyboardManagerContext(void);
MouseManager* GetMouseManagerContext(void);
JoypadManager* GetJoypadManagerContext(void);

InputDriverID GetInputDriverID(void);

bool InitializeInput(InputDriverID driver_id);
void DeinitializeInput(void);

void InitializeKeyboardInput(void);

void SetJoypadAutoConfigurationCallback(LMC_AutoConfigureJoypadCallback callback);
void AutoConfigureJoypad(LMC_Player player, char* name, uint32_t vendor, uint32_t product);

void ConnectJoypad(LMC_Player player);
void DisconnectJoypad(LMC_Player player);

void PollInput(void);

int16_t InputState(unsigned port, unsigned device, unsigned index, unsigned id);

void ProcessKeyboardKeyInput(uint32_t key, bool state, uint16_t modifiers);
void ProcessMouseMovement(LMC_Player player, int32_t delta_x, int32_t delta_y,
	int32_t absolute_x, int32_t absolute_y);
void ProcessMouseButtonInput(LMC_Player player, uint8_t button, bool state);
void ProcessMouseWheelInput(LMC_Player player, int32_t wheel_x, int32_t wheel_y);
void ProcessJoypadKeyInput(uint32_t key, bool state);
void ProcessJoypadButtonInput(LMC_Player player, uint8_t button, bool state);
void ProcessJoypadHatInput(LMC_Player player, uint8_t hat, uint8_t state);
void ProcessJoypadAxisInput(LMC_Player player, uint8_t axis, int16_t value);

void SetJoypadInput(LMC_Player player, LMC_JoypadInput input);
void ClearJoypadInput(LMC_Player player, LMC_JoypadInput input);
void SetKeyInput(uint32_t retrok);
void ClearKeyInput(uint32_t retrok);

RETRO_END_DECLS

#endif