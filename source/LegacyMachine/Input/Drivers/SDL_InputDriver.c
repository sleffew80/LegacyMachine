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

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <SDL.h>

#if defined HAVE_THREADS
#include <rthreads/rthreads.h>
#endif

#include "../InputManager.h"
#include "../../MainEngine.h"
#include "../../Logging.h"

/**************************************************************************************************
 * Variables
 *************************************************************************************************/

/* Static variables */

static SDL_Joystick*	sdl_joysticks[MAX_PLAYERS];

static const uint32_t sdl_keyboard[] = {
	0,					/* LMC_KEY_NONE */
	SDLK_BACKSPACE,		/* LMC_KEY_BACKSPACE */
	SDLK_TAB,			/* LMC_KEY_TAB */
	SDLK_CLEAR,			/* LMC_KEY_CLEAR */
	SDLK_RETURN,		/* LMC_KEY_RETURN */
	SDLK_PAUSE,			/* LMC_KEY_PAUSE */
	SDLK_ESCAPE,		/* LMC_KEY_ESCAPE */
	SDLK_SPACE,			/* LMC_KEY_SPACE */
	SDLK_EXCLAIM,		/* LMC_KEY_EXCLAIM */
	SDLK_QUOTEDBL,		/* LMC_KEY_QUOTEDBL */
	SDLK_HASH,			/* LMC_KEY_HASH */
	SDLK_DOLLAR,		/* LMC_KEY_DOLLAR */
	SDLK_AMPERSAND,		/* LMC_KEY_AMPERSAND */
	SDLK_QUOTE,			/* LMC_KEY_QUOTE */
	SDLK_LEFTPAREN,		/* LMC_KEY_LEFTPAREN */
	SDLK_RIGHTPAREN,	/* LMC_KEY_RIGHTPAREN */
	SDLK_ASTERISK,		/* LMC_KEY_ASTERISK */
	SDLK_PLUS,			/* LMC_KEY_PLUS */
	SDLK_COMMA,			/* LMC_KEY_COMMA */
	SDLK_MINUS,			/* LMC_KEY_MINUS */
	SDLK_PERIOD,		/* LMC_KEY_PERIOD */
	SDLK_SLASH,			/* LMC_KEY_SLASH */
	SDLK_0,				/* LMC_KEY_0 */
	SDLK_1,				/* LMC_KEY_1 */
	SDLK_2,				/* LMC_KEY_2 */
	SDLK_3,				/* LMC_KEY_3 */
	SDLK_4,				/* LMC_KEY_4 */
	SDLK_5,				/* LMC_KEY_5 */
	SDLK_6,				/* LMC_KEY_6 */
	SDLK_7,				/* LMC_KEY_7 */
	SDLK_8,				/* LMC_KEY_8 */
	SDLK_9,				/* LMC_KEY_9 */
	SDLK_COLON,			/* LMC_KEY_COLON */
	SDLK_SEMICOLON,		/* LMC_KEY_SEMICOLON */
	SDLK_LESS,			/* LMC_KEY_LESS */
	SDLK_EQUALS,		/* LMC_KEY_EQUALS */
	SDLK_GREATER,		/* LMC_KEY_GREATER */
	SDLK_QUESTION,		/* LMC_KEY_QUESTION */
	SDLK_AT,			/* LMC_KEY_AT */
	SDLK_LEFTBRACKET,	/* LMC_KEY_LEFTBRACKET */
	SDLK_BACKSLASH,		/* LMC_KEY_BACKSLASH */
	SDLK_RIGHTBRACKET,	/* LMC_KEY_RIGHTBRACKET */
	SDLK_CARET,			/* LMC_KEY_CARET */
	SDLK_UNDERSCORE,	/* LMC_KEY_UNDERSCORE */
	SDLK_BACKQUOTE,		/* LMC_KEY_BACKQUOTE */
	SDLK_a,				/* LMC_KEY_A */
	SDLK_b,				/* LMC_KEY_B */
	SDLK_c,				/* LMC_KEY_C */
	SDLK_d,				/* LMC_KEY_D */
	SDLK_e,				/* LMC_KEY_E */
	SDLK_f,				/* LMC_KEY_F */
	SDLK_g,				/* LMC_KEY_G */
	SDLK_h,				/* LMC_KEY_H */
	SDLK_i,				/* LMC_KEY_I */
	SDLK_j,				/* LMC_KEY_J */
	SDLK_k,				/* LMC_KEY_K */
	SDLK_l,				/* LMC_KEY_L */
	SDLK_m,				/* LMC_KEY_M */
	SDLK_n,				/* LMC_KEY_N */
	SDLK_o,				/* LMC_KEY_O */
	SDLK_p,				/* LMC_KEY_P */
	SDLK_q,				/* LMC_KEY_Q */
	SDLK_r,				/* LMC_KEY_R */
	SDLK_s,				/* LMC_KEY_S */
	SDLK_t,				/* LMC_KEY_T */
	SDLK_u,				/* LMC_KEY_U */
	SDLK_v,				/* LMC_KEY_V */
	SDLK_w,				/* LMC_KEY_W */
	SDLK_x,				/* LMC_KEY_X */
	SDLK_y,				/* LMC_KEY_Y */
	SDLK_z,				/* LMC_KEY_Z */
	0,					/* LMC_KEY_LEFTBRACE */
	0,					/* LMC_KEY_BAR */
	0,					/* LMC_KEY_RIGHTBRACE */
	0,					/* LMC_KEY_TILDE */
	SDLK_DELETE,		/* LMC_KEY_DELETE */
	SDLK_KP_0,			/* LMC_KEY_KP0 */
	SDLK_KP_1,			/* LMC_KEY_KP1 */
	SDLK_KP_2,			/* LMC_KEY_KP2 */
	SDLK_KP_3,			/* LMC_KEY_KP3 */
	SDLK_KP_4,			/* LMC_KEY_KP4 */
	SDLK_KP_5,			/* LMC_KEY_KP5 */
	SDLK_KP_6,			/* LMC_KEY_KP6 */
	SDLK_KP_7,			/* LMC_KEY_KP7 */
	SDLK_KP_8,			/* LMC_KEY_KP8 */
	SDLK_KP_9,			/* LMC_KEY_KP9 */
	SDLK_KP_PERIOD,		/* LMC_KEY_KP_PERIOD */
	SDLK_KP_DIVIDE,		/* LMC_KEY_KP_DIVIDE */
	SDLK_KP_MULTIPLY,	/* LMC_KEY_KP_MULTIPLY */
	SDLK_KP_MINUS,		/* LMC_KEY_KP_MINUS */
	SDLK_KP_PLUS,		/* LMC_KEY_KP_PLUS */
	SDLK_KP_ENTER,		/* LMC_KEY_KP_ENTER */
	SDLK_KP_EQUALS,		/* LMC_KEY_KP_EQUALS */
	SDLK_UP,			/* LMC_KEY_UP */
	SDLK_DOWN,			/* LMC_KEY_DOWN */
	SDLK_RIGHT,			/* LMC_KEY_RIGHT */
	SDLK_LEFT,			/* LMC_KEY_LEFT */
	SDLK_INSERT,		/* LMC_KEY_INSERT */
	SDLK_HOME,			/* LMC_KEY_HOME */
	SDLK_END,			/* LMC_KEY_END */
	SDLK_PAGEUP,		/* LMC_KEY_PAGEUP */
	SDLK_PAGEDOWN,		/* LMC_KEY_PAGEDOWN */
	SDLK_F1,			/* LMC_KEY_F1 */
	SDLK_F2,			/* LMC_KEY_F2 */
	SDLK_F3,			/* LMC_KEY_F3 */
	SDLK_F4,			/* LMC_KEY_F4 */
	SDLK_F5,			/* LMC_KEY_F5 */
	SDLK_F6,			/* LMC_KEY_F6 */
	SDLK_F7,			/* LMC_KEY_F7 */
	SDLK_F8,			/* LMC_KEY_F8 */
	SDLK_F9,			/* LMC_KEY_F9 */
	SDLK_F10,			/* LMC_KEY_F10 */
	SDLK_F11,			/* LMC_KEY_F11 */
	SDLK_F12,			/* LMC_KEY_F12 */
	SDLK_F13,			/* LMC_KEY_F13 */
	SDLK_F14,			/* LMC_KEY_F14 */
	SDLK_F15,			/* LMC_KEY_F15 */
	SDLK_NUMLOCKCLEAR,	/* LMC_KEY_NUMLOCK */
	SDLK_CAPSLOCK,		/* LMC_KEY_CAPSLOCK */
	SDLK_SCROLLLOCK,	/* LMC_KEY_SCROLLOCK */
	SDLK_RSHIFT,		/* LMC_KEY_RSHIFT */
	SDLK_LSHIFT,		/* LMC_KEY_LSHIFT */
	SDLK_RCTRL,			/* LMC_KEY_RCTRL */
	SDLK_LCTRL,			/* LMC_KEY_LCTRL */
	SDLK_RALT,			/* LMC_KEY_RALT */
	SDLK_LALT,			/* LMC_KEY_LALT */
	0,					/* LMC_KEY_RMETA */
	0,					/* LMC_KEY_LMETA */
	SDLK_LGUI,			/* LMC_KEY_LSUPER */
	SDLK_RGUI,			/* LMC_KEY_RSUPER */
	SDLK_MODE,			/* LMC_KEY_MODE */
	0,					/* LMC_KEY_COMPOSE */
	SDLK_HELP,			/* LMC_KEY_HELP */
	SDLK_PRINTSCREEN,	/* LMC_KEY_PRINT */
	SDLK_SYSREQ,		/* LMC_KEY_SYSREQ */
	SDLK_PAUSE,			/* LMC_KEY_BREAK */
	SDLK_MENU,			/* LMC_KEY_MENU */
	SDLK_POWER,			/* LMC_KEY_POWER */
	0,					/* LMC_KEY_EURO */
	SDLK_UNDO,			/* LMC_KEY_UNDO */
	0					/* LMC_KEY_OEM_102 */
};

/**************************************************************************************************
 * Prototypes
 *************************************************************************************************/

/* Static prototypes */

static void SDL_DriverConnectMouse(LMC_Player player);
static void SDL_DriverDisconnectMouse(LMC_Player player);
static void SDL_DriverConnectJoypad(LMC_Player player);
static void SDL_DriverDisconnectJoypad(LMC_Player player);

/**************************************************************************************************
 * SDL Keyboard Functions
 *************************************************************************************************/

/* Keyboard initialization. */
static void SDL_DriverInitializeKeyboard(void)
{
	KeyboardManager* keyboard = GetKeyboardManagerContext();
	int num_keys = 0;

	SDL_GetKeyboardState(&num_keys);

	keyboard->state.info.name = "Keyboard";
	keyboard->state.info.keys = num_keys;
	keyboard->state.key_codes = &sdl_keyboard;
}

/**************************************************************************************************
 * SDL Mouse Functions
 *************************************************************************************************/

/* Mouse initialization. */
static void SDL_DriverInitializeMouse(void)
{
	SDL_DriverConnectMouse(LMC_PLAYER_1);
}

static void SDL_DriverDeinitializeMouse(LMC_Player player)
{
	MouseManager* mouse = GetMouseManagerContext();

	mouse->state[player].info.name = NULL;
	mouse->state[player].position_x = 0;
	mouse->state[player].position_y = 0;
	mouse->state[player].position_abs_x = 0;
	mouse->state[player].position_abs_y = 0;
	mouse->state[player].button_left = 0;
	mouse->state[player].button_middle = 0;
	mouse->state[player].button_right = 0;
	mouse->state[player].button_4 = 0;
	mouse->state[player].button_5 = 0;
	mouse->state[player].wheel_up = 0;
	mouse->state[player].wheel_down = 0;
	mouse->state[player].wheel_left = 0;
	mouse->state[player].wheel_right = 0;
	mouse->state[player].identifier = 0;
	mouse->state[player].connected = false;
}

static void SDL_DriverConnectMouse(LMC_Player player)
{
	MouseManager* mouse = GetMouseManagerContext();

	mouse->state[player].info.name = "Mouse";
	mouse->state[player].connected = true;
}

static void SDL_DriverDisconnectMouse(LMC_Player player)
{
	SDL_DriverDeinitializeMouse(player);
}

/* Poll mouse position and state. */
static void SDL_DriverPollMouse(void)
{
	InputManager* input = GetInputManagerContext();

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		SDL_GetRelativeMouseState(&input->mouse->state[i].position_x, 
			&input->mouse->state[i].position_y);
		SDL_GetMouseState(&input->mouse->state[i].position_abs_x, 
			&input->mouse->state[i].position_abs_y);
	}
}

/**************************************************************************************************
 * SDL Joypad Functions
 *************************************************************************************************/

/* Joypad initialization. */
static bool SDL_DriverInitializeJoypad(void)
{
	unsigned num_joysticks = 0;
	uint32_t subsystem_flags = SDL_WasInit(0);
	unsigned i;

	/* Initialize input subsystem, if necessary. */
	if (subsystem_flags == 0)
	{
		if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
		{
			lmc_trace(LMC_LOG_ERRORS, 
				"[Input] [SDL]: Failed to initialize input subsystem: %s", SDL_GetError());
			LMC_SetLastError(LMC_ERR_FAIL_INPUT_INIT);
			return false;
		}
	}
	else if ((subsystem_flags & SDL_INIT_JOYSTICK) == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
		{
			lmc_trace(LMC_LOG_ERRORS, 
				"[Input] [SDL]: Failed to initialize input subsystem: %s", SDL_GetError());
			LMC_SetLastError(LMC_ERR_FAIL_INPUT_INIT);
			return false;
		}
	}

	num_joysticks = SDL_NumJoysticks();
	if (num_joysticks > MAX_PLAYERS)
		num_joysticks = MAX_PLAYERS;

	for (i = 0; i < num_joysticks; i++)
		SDL_DriverConnectJoypad(i);

	SDL_JoystickEventState(SDL_ENABLE);

	return true;
}

/* Joypad deinitialization. */
static void SDL_DriverDeinitializeJoypad(LMC_Player player)
{
	JoypadManager* joypad = GetJoypadManagerContext();

	memset(&joypad->state[player].button_map, 0, sizeof(joypad->state[player].button_map));
	memset(&joypad->state[player].hat_map, 0, sizeof(joypad->state[player].hat_map));
	memset(&joypad->state[player].axis_map, 0, sizeof(joypad->state[player].axis_map));
	memset(&joypad->state[player].axis_indices, 0, sizeof(joypad->state[player].axis_indices));
	memset(&joypad->state[player].axis_types, 0, sizeof(joypad->state[player].axis_types));
	memset(&joypad->state[player].analog_values, 0, sizeof(joypad->state[player].analog_values));
	joypad->state[player].info.name = NULL;
	joypad->state[player].info.product = 0;
	joypad->state[player].info.vendor = 0;
	joypad->state[player].info.buttons = 0;
	joypad->state[player].info.axes = 0;
	joypad->state[player].info.hats = 0;
	joypad->state[player].analog_deadzone = 0;
	joypad->state[player].analog_threshold = 0;
	joypad->state[player].digital_inputs = 0;
	joypad->state[player].identifier = 0;
	joypad->state[player].connected = false;

	sdl_joysticks[player] = NULL;
}

/* Initialize a given player's joypad when connected. */
static void SDL_DriverConnectJoypad(LMC_Player player)
{
	JoypadManager* joypad = GetJoypadManagerContext();

	bool ok = false;

	sdl_joysticks[player] = SDL_JoystickOpen(player);
	ok = sdl_joysticks[player] != NULL;

	if (!ok)
	{
		lmc_trace(LMC_LOG_ERRORS, 
			"[Input] [SDL]: Couldn't open joystick #%u: %s\n", player, SDL_GetError());

		if (sdl_joysticks[player])
			SDL_JoystickClose(sdl_joysticks[player]);

		sdl_joysticks[player] = NULL;

		return;
	}

	joypad->state[player].info.vendor = SDL_JoystickGetDeviceVendor(player);
	joypad->state[player].info.product = SDL_JoystickGetDeviceProduct(player);
	joypad->state[player].info.name = SDL_JoystickNameForIndex(player);;

	joypad->state[player].info.buttons = SDL_JoystickNumButtons(sdl_joysticks[player]);
	joypad->state[player].info.axes = SDL_JoystickNumAxes(sdl_joysticks[player]);
	joypad->state[player].info.hats = SDL_JoystickNumHats(sdl_joysticks[player]);

	/* Default analog stick mapping (standard dual - stick layout). */
	joypad->state[player].axis_indices[0] = 0;  /* Left stick X → axis 0. */
	joypad->state[player].axis_indices[1] = 1;  /* Left stick Y → axis 1. */
	joypad->state[player].axis_indices[2] = 2;  /* Right stick X → axis 2. */
	joypad->state[player].axis_indices[3] = 3;  /* Right stick Y → axis 3. */

	/* Initialize axis deadzone and threshold to reasonable (power of 2) defaults. */
	joypad->state[player].analog_deadzone = 8192;   /* 25% for smooth analog. */
	joypad->state[player].analog_threshold = 16384; /* 50% for button emulation */

	lmc_trace(LMC_LOG_VERBOSE, 
		"[Input] [SDL]: Joypad initialized (name: %s, vendor id: %i, product id: %i, buttons: %i, hats: %i, axes: %i)",
		joypad->state[player].info.name, joypad->state[player].info.vendor, joypad->state[player].info.product,
		joypad->state[player].info.buttons, joypad->state[player].info.hats, joypad->state[player].info.axes);

	AutoConfigureJoypad(player, 
		joypad->state[player].info.name, 
		joypad->state[player].info.vendor, 
		joypad->state[player].info.product);

	joypad->state[player].identifier = SDL_JoystickInstanceID(sdl_joysticks[player]);
	joypad->state[player].connected = true;
}

/* Deinitialize a given player's joypad when disconnected. */
static void SDL_DriverDisconnectJoypad(LMC_Player player)
{
	JoypadManager* joypad = GetJoypadManagerContext();

	if (sdl_joysticks[player])
	{
		SDL_JoystickClose(sdl_joysticks[player]);
	}
	SDL_DriverDeinitializeJoypad(player);
}

/* Close all joypads. */
static void SDL_DriverCloseJoypad(void)
{
	unsigned i;
	for (i = 0; i < MAX_PLAYERS; i++)
		SDL_DriverDisconnectJoypad(i);
}

/* Poll joypad axes and update analog values for all active players. */
static void SDL_DriverPollJoypad(void)
{
	JoypadManager* joypad = GetJoypadManagerContext();

	/* Refresh the joypad state. */
	SDL_JoystickUpdate();

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (!sdl_joysticks[i]) continue;

		int num_axes = SDL_JoystickNumAxes(sdl_joysticks[i]);
		for (int axis = 0; axis < num_axes && axis < MAX_AXES; axis++)
		{
			int16_t value = SDL_JoystickGetAxis(sdl_joysticks[i], axis);
			int16_t deadzone = joypad->state[i].analog_deadzone;
			if (value < -deadzone || value > deadzone)
				joypad->state[i].analog_values[axis] = value;
			else
				joypad->state[i].analog_values[axis] = 0;  /* Reset to center */
		}
	}
}

/* Input initialization. */
static bool SDL_DriverInitializeInput(void)
{
	lmc_trace(LMC_LOG_VERBOSE, "[Input] [SDL]: Initializing input...");

	SDL_DriverInitializeKeyboard();
	SDL_DriverInitializeMouse();

	bool result = SDL_DriverInitializeJoypad();

	lmc_trace(LMC_LOG_VERBOSE, "[Input] [SDL]: Input initialized successfully");

	return result;
}

/* Poll all input. */
static void SDL_DriverPollInput(void)
{
	SDL_DriverPollMouse();
	SDL_DriverPollJoypad();
}

/* Close all input. */
static void SDL_DriverCloseInput(void)
{
	lmc_trace(LMC_LOG_VERBOSE, "[Input] [SDL]: Closing input...");

	SDL_DriverCloseJoypad();
}

/**************************************************************************************************
 * SDL Input Driver
 *************************************************************************************************/

JoypadDriver sdl_joypad_driver = {
	SDL_DriverInitializeJoypad,	/* cb_init */
	SDL_DriverConnectJoypad,		/* cb_connect */
	SDL_DriverDisconnectJoypad,	/* cb_disconnect */
	SDL_DriverCloseJoypad,		/* cb_deinit */
};

InputDriver sdl_input_driver = {
	SDL_DriverInitializeInput,	/* cb_init */
	SDL_DriverPollInput,			/* cb_poll */
	SDL_DriverCloseInput,		/* cb_deinit */
	&sdl_joypad_driver,
	INPUT_DRIVER_SDL
};
