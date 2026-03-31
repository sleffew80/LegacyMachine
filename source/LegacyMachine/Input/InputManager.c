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
#include <string.h>

#include "InputManager.h"
#include "../MainEngine.h"

/**************************************************************************************************
 * InputManager Context
 *************************************************************************************************/

static InputManager input_manager = { 0 };

/**************************************************************************************************
 * InputDriver Context Array
 *************************************************************************************************/

const InputDriver* input_drivers[] = {
#if defined HAVE_SDL
	&sdl_input_driver,
#endif
	NULL
};

/**************************************************************************************************
 * InputDriver Context
 *************************************************************************************************/

static InputDriver* input_driver = NULL;

/**************************************************************************************************
 * Input Keymap Array
 *************************************************************************************************/

const uint32_t input_keymap[] = {
	RETROK_FIRST,
	RETROK_BACKSPACE,
	RETROK_TAB,
	RETROK_CLEAR,
	RETROK_RETURN,
	RETROK_PAUSE,
	RETROK_ESCAPE,
	RETROK_SPACE,
	RETROK_EXCLAIM,
	RETROK_QUOTEDBL,
	RETROK_HASH,
	RETROK_DOLLAR,
	RETROK_AMPERSAND,
	RETROK_QUOTE,
	RETROK_LEFTPAREN,
	RETROK_RIGHTPAREN,
	RETROK_ASTERISK,
	RETROK_PLUS,
	RETROK_COMMA,
	RETROK_MINUS,
	RETROK_PERIOD,
	RETROK_SLASH,
	RETROK_0,
	RETROK_1,
	RETROK_2,
	RETROK_3,
	RETROK_4,
	RETROK_5,
	RETROK_6,
	RETROK_7,
	RETROK_8,
	RETROK_9,
	RETROK_COLON,
	RETROK_SEMICOLON,
	RETROK_LESS,
	RETROK_EQUALS,
	RETROK_GREATER,
	RETROK_QUESTION,
	RETROK_AT,
	RETROK_LEFTBRACKET,
	RETROK_BACKSLASH,
	RETROK_RIGHTBRACKET,
	RETROK_CARET,
	RETROK_UNDERSCORE,
	RETROK_BACKQUOTE,
	RETROK_a,
	RETROK_b,
	RETROK_c,
	RETROK_d,
	RETROK_e,
	RETROK_f,
	RETROK_g,
	RETROK_h,
	RETROK_i,
	RETROK_j,
	RETROK_k,
	RETROK_l,
	RETROK_m,
	RETROK_n,
	RETROK_o,
	RETROK_p,
	RETROK_q,
	RETROK_r,
	RETROK_s,
	RETROK_t,
	RETROK_u,
	RETROK_v,
	RETROK_w,
	RETROK_x,
	RETROK_y,
	RETROK_z,
	RETROK_LEFTBRACE,
	RETROK_BAR,
	RETROK_RIGHTBRACE,
	RETROK_TILDE,
	RETROK_DELETE,
	RETROK_KP0,
	RETROK_KP1,
	RETROK_KP2,
	RETROK_KP3,
	RETROK_KP4,
	RETROK_KP5,
	RETROK_KP6,
	RETROK_KP7,
	RETROK_KP8,
	RETROK_KP9,
	RETROK_KP_PERIOD,
	RETROK_KP_DIVIDE,
	RETROK_KP_MULTIPLY,
	RETROK_KP_MINUS,
	RETROK_KP_PLUS,
	RETROK_KP_ENTER,
	RETROK_KP_EQUALS,
	RETROK_UP,
	RETROK_DOWN,
	RETROK_RIGHT,
	RETROK_LEFT,
	RETROK_INSERT,
	RETROK_HOME,
	RETROK_END,
	RETROK_PAGEUP,
	RETROK_PAGEDOWN,
	RETROK_F1,
	RETROK_F2,
	RETROK_F3,
	RETROK_F4,
	RETROK_F5,
	RETROK_F6,
	RETROK_F7,
	RETROK_F8,
	RETROK_F9,
	RETROK_F10,
	RETROK_F11,
	RETROK_F12,
	RETROK_F13,
	RETROK_F14,
	RETROK_F15,
	RETROK_NUMLOCK,
	RETROK_CAPSLOCK,
	RETROK_SCROLLOCK,
	RETROK_RSHIFT,
	RETROK_LSHIFT,
	RETROK_RCTRL,
	RETROK_LCTRL,
	RETROK_RALT,
	RETROK_LALT,
	RETROK_RMETA,
	RETROK_LMETA,
	RETROK_LSUPER,
	RETROK_RSUPER,
	RETROK_MODE,
	RETROK_COMPOSE,
	RETROK_HELP,
	RETROK_PRINT,
	RETROK_SYSREQ,
	RETROK_BREAK,
	RETROK_MENU,
	RETROK_POWER,
	RETROK_EURO,
	RETROK_UNDO,
	RETROK_OEM_102
};

/**************************************************************************************************
 * InputDriver Functions
 *************************************************************************************************/

 /* Verify that InputDriverID enum values match their corresponding indices in the input_drivers
	array. */
 static void AssertInputDriverEnumValues(void)
{
	for (unsigned i = 0; i < MAX_INPUT_DRIVERS; ++i)
	{
		retro_assert((InputDriverID)i == input_drivers[i]->id);
	}
}

/* Initialize the input driver. */
static InputDriver* InitializeInputDriver(InputDriverID driver_id)
{
#if defined _DEBUG
	/* Verify InputDriverID values when debugging. */
	AssertInputDriverEnumValues();
#endif
	return (InputDriver*)input_drivers[driver_id];
}

/* Get the current input driver context. */
static InputDriver* GetInputDriverContext(void)
{
	return input_driver;
}

/**************************************************************************************************
 * InputManager Functions
 *************************************************************************************************/

/* Get the keyboard key state for a given port and key ID. */
static int16_t KeyboardState(unsigned port, unsigned index, unsigned id)
{
	KeyboardManager* keyboard = GetKeyboardManagerContext();

	if (!keyboard)
		return 0;

	if (id >= RETROK_LAST)
		return 0;

	return keyboard->state.key_state[id] ? 1 : 0;
}

/* Get the mouse input state for a given port and input ID. */
static int16_t MouseState(unsigned port, unsigned index, unsigned id)
{
	MouseManager* mouse = GetMouseManagerContext();

	if (!mouse)
		return 0;

	switch (id)
	{
		case RETRO_DEVICE_ID_MOUSE_X:
			return mouse->state[port].position_x;
		case RETRO_DEVICE_ID_MOUSE_Y:
			return mouse->state[port].position_y;
		case RETRO_DEVICE_ID_MOUSE_LEFT:
			return mouse->state[port].button_left;
		case RETRO_DEVICE_ID_MOUSE_RIGHT:
			return mouse->state[port].button_right;
		case RETRO_DEVICE_ID_MOUSE_WHEELUP:
			return mouse->state[port].wheel_up;
		case RETRO_DEVICE_ID_MOUSE_WHEELDOWN:
			return mouse->state[port].wheel_down;
		case RETRO_DEVICE_ID_MOUSE_MIDDLE:
			return mouse->state[port].button_middle;
		case RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP:
			return mouse->state[port].wheel_right;
		case RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN:
			return mouse->state[port].wheel_left;
		case RETRO_DEVICE_ID_MOUSE_BUTTON_4:
			return mouse->state[port].button_4;
		case RETRO_DEVICE_ID_MOUSE_BUTTON_5:
			return mouse->state[port].button_5;
	}

	return 0;
}

/* Get the digital joypad input state for a given port and input ID. */
static int16_t JoypadState(unsigned port, unsigned index, unsigned id)
{
	JoypadManager* joypad = GetJoypadManagerContext();

	if (port >= MAX_PLAYERS)
		return 0;

	if (id == RETRO_DEVICE_ID_JOYPAD_MASK)
		/* Return masked input. Shift right by 1 to
		   convert LMC_JoypadInput to RETRO_DEVICE
		   input. */
		return (int16_t)(joypad->state[port].digital_inputs >> 1);

	/* Return a specific input state. INPUT_MASK prevents bit shift overflow. */
	return (int16_t)(joypad->state[port].digital_inputs & (1 << ((id + 1) & INPUT_MASK)));
}

/* Get the analog state for a given joypad port, index, and axis ID. */
static int16_t JoypadAnalogState(unsigned port, unsigned index, unsigned id)
{
	JoypadManager* joypad = GetJoypadManagerContext();
	if (port >= MAX_PLAYERS)
		return 0;

	/* Map Libretro analog input to remapped physical axis. */
	int logical_axis = (index * 2) + id; /* 0-3 for left/right X/Y. */

	if (logical_axis >= 4)
		return 0;

	int physical_axis = joypad->state[port].axis_indices[logical_axis];

	if (physical_axis < MAX_AXES)
		return joypad->state[port].analog_values[physical_axis];

	return 0;
}

/* Get the current input manager context. */
InputManager* GetInputManagerContext(void)
{
	return &input_manager;
}

/* Get the current keyboard manager context. */
KeyboardManager* GetKeyboardManagerContext(void)
{
	InputManager* input = GetInputManagerContext();

	return input->keyboard;
}

/* Get the current mouse manager context. */
MouseManager* GetMouseManagerContext(void)
{
	InputManager* input = GetInputManagerContext();

	return input->mouse;
}

/* Get the current joypad manager context. */
JoypadManager* GetJoypadManagerContext(void)
{
	InputManager* input = GetInputManagerContext();

	return input->joypad;
}

/* Get the current input driver ID. */
InputDriverID GetInputDriverID(void)
{
	InputDriver* driver = GetInputDriverContext();
	return driver ? driver->id : INPUT_DRIVER_NONE;
}

/* Initialize the input manager with the specified input driver. */
bool InitializeInput(InputDriverID driver_id)
{
	InputManager* input = GetInputManagerContext();
	InputDriver* driver = InitializeInputDriver(driver_id);

	input->joypad = (JoypadManager*)calloc(1, sizeof(JoypadManager));
	input->keyboard = (KeyboardManager*)calloc(1, sizeof(KeyboardManager));
	input->mouse = (MouseManager*)calloc(1, sizeof(MouseManager));
	if (!input->joypad || !input->keyboard || !input->mouse)
	{
		return false;
	}

	if (!driver || !driver->cb_init)
	{
		return false;
	}

	input_driver = driver;
	input->initialized = driver->cb_init();

	/* Enable keyboard interface input by default. */
	input->keyboard->interface_input = true;

	/* Initialize default keyboard input mappings. */
	InitializeKeyboardInput();

	return input->initialized;
}

/* Deinitialize the input manager and its driver. */
void DeinitializeInput(void)
{
	InputManager* input = GetInputManagerContext();
	InputDriver* driver = GetInputDriverContext();

	if (driver && driver->cb_deinit)
	{
		driver->cb_deinit();
	}

	input_driver = NULL;

	free(input->joypad);
	input->joypad = NULL;
	free(input->keyboard);
	input->keyboard = NULL;
	free(input->mouse);
	input->mouse = NULL;

	memset(input, 0, sizeof(InputManager));
}

/* Initialize keyboard input mappings from settings. */
void InitializeKeyboardInput(void)
{
	SettingsManager* settings = GetSettingsManagerContext();
	InputManager* input = GetInputManagerContext();

	input->keyboard->joypad_input = settings->bools.input_keyboard_enable_joypad_input;

	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_UP,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_up_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_DOWN,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_down_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_LEFT,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_left_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_RIGHT,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_right_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_B,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_b_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_A,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_a_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_Y,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_y_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_X,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_x_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_L,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_l_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_R,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_r_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_L2,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_l2_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_R2,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_r2_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_L3,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_l3_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_R3,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_r3_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_L_ANALOG_UP,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_l_analog_up_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_L_ANALOG_DOWN,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_l_analog_down_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_L_ANALOG_LEFT,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_l_analog_left_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_L_ANALOG_RIGHT,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_l_analog_right_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_R_ANALOG_UP,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_r_analog_up_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_R_ANALOG_DOWN,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_r_analog_down_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_R_ANALOG_LEFT,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_r_analog_left_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_R_ANALOG_RIGHT,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_r_analog_right_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_SELECT,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_select_key), false);
	LMC_DefineJoypadKeyInput(LMC_JOYPAD_INPUT_START,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_player1_start_key), false);
#if defined HAVE_MENU
	LMC_DefineKeyboardKeyInput(LMC_KEYBOARD_INPUT_HOME,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_home_key), false);
#endif
	LMC_DefineKeyboardKeyInput(LMC_KEYBOARD_INPUT_QUIT,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_quit_key), false);
	LMC_DefineKeyboardKeyInput(LMC_KEYBOARD_INPUT_SAVE,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_state_save_key), false);
	LMC_DefineKeyboardKeyInput(LMC_KEYBOARD_INPUT_LOAD,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_state_load_key), false);
	LMC_DefineKeyboardKeyInput(LMC_KEYBOARD_INPUT_CRT,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_toggle_crt_effect_key), false);
	LMC_DefineKeyboardKeyInput(LMC_KEYBOARD_INPUT_FULLSCREEN,
		GetKeyCodeEnumFromString(settings->strings.input_keyboard_toggle_fullscreen_key), false);
}

/* Set the callback used for auto-configuring joypad input mappings. */
void SetJoypadAutoConfigurationCallback(LMC_AutoConfigureJoypadCallback callback)
{
	InputManager* input = GetInputManagerContext();

	input->cb_joypad_auto_config = callback;
}

/* Invoke the joypad auto-configuration callback for the given player. */
void AutoConfigureJoypad(LMC_Player player, char* name, uint32_t vendor, uint32_t product)
{
	InputManager* input = GetInputManagerContext();

	if (input->cb_joypad_auto_config)
		input->cb_joypad_auto_config(player, name, vendor, product);
}

/* Initialize a given player's joypad when connected. */
void ConnectJoypad(LMC_Player player)
{
	InputDriver* driver = GetInputDriverContext();
	InputManager* input = GetInputManagerContext();

	retro_assert(driver);
	retro_assert(input->initialized);

	driver->joypad->cb_connect(player);
}

/* Deinitialize a given player's joypad when disconnected. */
void DisconnectJoypad(LMC_Player player)
{
	InputDriver* driver = GetInputDriverContext();
	InputManager* input = GetInputManagerContext();

	retro_assert(driver);
	retro_assert(input->initialized);

	driver->joypad->cb_disconnect(player);
}

/* Poll input devices. */
void PollInput(void)
{
	InputDriver* driver = GetInputDriverContext();
	InputManager* input = GetInputManagerContext();

	retro_assert(driver);
	retro_assert(input->initialized);

	if (driver->cb_poll)
		driver->cb_poll();
}

/* Get the input state for a given port, device, index, and ID. */
int16_t InputState(unsigned port, unsigned device, unsigned index, unsigned id)
{
	switch (device)
	{
	case RETRO_DEVICE_JOYPAD:
		return JoypadState(port, index, id);
		break;
	case RETRO_DEVICE_MOUSE:
		return MouseState(port, index, id);
		break;
	case RETRO_DEVICE_KEYBOARD:
		return KeyboardState(port, index, id);
		break;
	case RETRO_DEVICE_ANALOG:
		return JoypadAnalogState(port, index, id);
		break;
	}
	return 0;
}

/* Process keyboard key input. Used for processing keyboard input for libretro cores. */
void ProcessKeyboardKeyInput(uint32_t key, bool state, uint16_t modifiers)
{
	KeyboardManager* keyboard = GetKeyboardManagerContext();
	SystemManager* system = GetSystemManagerContext();
	unsigned i;

	if (!keyboard || !keyboard->state.key_codes)
		return;

	for (i = 1; i < MAX_KEY_INPUTS; i++)
	{
		if (keyboard->state.key_codes[i] == key)
		{
			uint32_t input = input_keymap[i];
			if (input > RETROK_FIRST && input < RETROK_LAST)
			{
				if (state)
					SetKeyInput(input);
				else
					ClearKeyInput(input);

				if (system->cb_keyboard.callback)
					system->cb_keyboard.callback(state != 0, input, (uint32_t)key, modifiers);
			}
			return;
		}
	}
}

/* Process mouse movement input, updating relative and absolute position. */
void ProcessMouseMovement(LMC_Player player, int32_t delta_x, int32_t delta_y,
	int32_t absolute_x, int32_t absolute_y)
{
	MouseManager* mouse = GetMouseManagerContext();

	if (!mouse)
		return;

	mouse->state[player].position_x += (int16_t)delta_x;
	mouse->state[player].position_y += (int16_t)delta_y;
	mouse->state[player].position_abs_x = (int16_t)absolute_x;
	mouse->state[player].position_abs_y = (int16_t)absolute_y;
}

/* Process a mouse button press or release event. */
void ProcessMouseButtonInput(LMC_Player player, uint8_t button, bool state)
{
	MouseManager* mouse = GetMouseManagerContext();

	if (!mouse)
		return;

	switch (button)
	{
	case MOUSE_INPUT_LEFT:
		mouse->state[player].button_left = state ? 1 : 0;
		break;
	case MOUSE_INPUT_MIDDLE:
		mouse->state[player].button_middle = state ? 1 : 0;
		break;
	case MOUSE_INPUT_RIGHT:
		mouse->state[player].button_right = state ? 1 : 0;
		break;
	case MOUSE_INPUT_X1:
		mouse->state[player].button_4 = state ? 1 : 0;
		break;
	case MOUSE_INPUT_X2:
		mouse->state[player].button_5 = state ? 1 : 0;
		break;
	}
}

/* Process mouse wheel scroll input. */
void ProcessMouseWheelInput(LMC_Player player, int32_t wheel_x, int32_t wheel_y)
{
	MouseManager* mouse = GetMouseManagerContext();

	mouse->state[player].wheel_up = wheel_y < 0;
	mouse->state[player].wheel_down = wheel_y > 0;
	mouse->state[player].wheel_left = wheel_x < 0;
	mouse->state[player].wheel_right = wheel_x > 0;
}

/* Process keyboard key input as joypad input. */
void ProcessJoypadKeyInput(uint32_t key, bool state)
{
	JoypadManager* joypad = GetJoypadManagerContext();
	LMC_JoypadInput input = LMC_JOYPAD_INPUT_NONE;

	/* Find joypad input mapped to this key */
	for (int i = LMC_JOYPAD_INPUT_B; i < MAX_INPUTS && input == LMC_JOYPAD_INPUT_NONE; i++)
	{
		if (joypad->state[LMC_PLAYER_1].key_map[i] == key)
		{
			input = (LMC_JoypadInput)i;
		}
	}

	/* Update. */
	if (input != LMC_JOYPAD_INPUT_NONE)
	{
		if (state)
			SetJoypadInput(LMC_PLAYER_1, input);
		else
			ClearJoypadInput(LMC_PLAYER_1, input);
	}
}

/* Process joypad button input. */
void ProcessJoypadButtonInput(LMC_Player player, uint8_t button, bool state)
{
	InputDriver* driver = GetInputDriverContext();
	JoypadManager* joypad = GetJoypadManagerContext();
	LMC_JoypadInput input = LMC_JOYPAD_INPUT_NONE;
	int i;

	/* Search input. */
	for (i = LMC_JOYPAD_INPUT_B; i < MAX_INPUTS && input == LMC_JOYPAD_INPUT_NONE; i++)
	{
		if (joypad->state[player].button_map[i] == button)
			input = (LMC_JoypadInput)i;
	}

	/* Update. */
	if (input != LMC_JOYPAD_INPUT_NONE)
	{
		if (state)
			SetJoypadInput(player, input);
		else
			ClearJoypadInput(player, input);
	}
}

/* Process joypad hat input. */
void ProcessJoypadHatInput(LMC_Player player, uint8_t hat, uint8_t state)
{
	JoypadManager* joypad = GetJoypadManagerContext();

	if (player >= MAX_PLAYERS || hat >= MAX_HATS)
		return;

	/* Clear all four directions first */
	ClearJoypadInput(player, joypad->state[player].hat_map[hat][LMC_HAT_UP]);
	ClearJoypadInput(player, joypad->state[player].hat_map[hat][LMC_HAT_RIGHT]);
	ClearJoypadInput(player, joypad->state[player].hat_map[hat][LMC_HAT_DOWN]);
	ClearJoypadInput(player, joypad->state[player].hat_map[hat][LMC_HAT_LEFT]);

	/* Set whichever directions are active - diagonals supported. */
	if (state & (1 << LMC_HAT_UP))
		SetJoypadInput(player, joypad->state[player].hat_map[hat][LMC_HAT_UP]);
	if (state & (1 << LMC_HAT_RIGHT))
		SetJoypadInput(player, joypad->state[player].hat_map[hat][LMC_HAT_RIGHT]);
	if (state & (1 << LMC_HAT_DOWN))
		SetJoypadInput(player, joypad->state[player].hat_map[hat][LMC_HAT_DOWN]);
	if (state & (1 << LMC_HAT_LEFT))
		SetJoypadInput(player, joypad->state[player].hat_map[hat][LMC_HAT_LEFT]);
}

/* Process joypad axis input (For non-analog input actions assigned to a physical joypad's analog
   axis directions). */
void ProcessJoypadAxisInput(LMC_Player player, uint8_t axis, int16_t value)
{
	JoypadManager* joypad = GetJoypadManagerContext();

	if (player >= MAX_PLAYERS || axis >= MAX_AXES)
		return;

	/* Clear both directions */
	ClearJoypadInput(player, joypad->state[player].axis_map[axis][LMC_AXIS_POS]);
	ClearJoypadInput(player, joypad->state[player].axis_map[axis][LMC_AXIS_NEG]);

	/* Threshold is used to prevent accidental input from slight axis movement or drift.
	   Only set input if value exceeds threshold in either direction. */
	int16_t threshold = joypad->state[player].analog_threshold;

	if (value > threshold)
		SetJoypadInput(player, joypad->state[player].axis_map[axis][LMC_AXIS_POS]);
	else if (value < -threshold)
		SetJoypadInput(player, joypad->state[player].axis_map[axis][LMC_AXIS_NEG]);
}

/* Marks joypad input as pressed. */
void SetJoypadInput(LMC_Player player, LMC_JoypadInput input)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	legacy_machine->input->joypad->state[player].digital_inputs |= (1 << input);
	legacy_machine->input->last_input = input;
}

/* Marks joypad input as unpressed. */
void ClearJoypadInput(LMC_Player player, LMC_JoypadInput input)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	legacy_machine->input->joypad->state[player].digital_inputs &= ~(1 << input);
}

/* Marks a key as pressed. Used for libretro core keyboard input. */
void SetKeyInput(uint32_t input)
{
	KeyboardManager* keyboard = GetKeyboardManagerContext();

	if (keyboard && input < RETROK_LAST)
		keyboard->state.key_state[input] = true;
}

/* Marks a key as released. Used for libretro core keyboard input. */
void ClearKeyInput(uint32_t input)
{
	KeyboardManager* keyboard = GetKeyboardManagerContext();

	if (keyboard && input < RETROK_LAST)
		keyboard->state.key_state[input] = false;
}

