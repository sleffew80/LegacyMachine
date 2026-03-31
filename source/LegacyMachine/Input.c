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
#include <compat/strl.h>

#include "LegacyMachine.h"
#include "MainEngine.h"

/**************************************************************************************************
 * Internal Static Functions
 *************************************************************************************************/

/* Get key code string from the enumeration value. */
static const char* GetKeyCodeStringFromEnum(LMC_KeyCode key)
{
	switch (key)
	{
		case LMC_KEY_NONE:
			return "null";
		case LMC_KEY_BACKSPACE:
			return "backspace";
		case LMC_KEY_TAB:
			return "tab";
		case LMC_KEY_CLEAR:
			return "clear";
		case LMC_KEY_RETURN:
			return "enter";
		case LMC_KEY_PAUSE:
			return "pause";
		case LMC_KEY_ESCAPE:
			return "escape";
		case LMC_KEY_SPACE:
			return "space";
		case LMC_KEY_EXCLAIM:
			return "exclamation";
		case LMC_KEY_QUOTEDBL:
			return "double_quote";
		case LMC_KEY_HASH:
			return "hash";
		case LMC_KEY_DOLLAR:
			return "dollar";
		case LMC_KEY_AMPERSAND:
			return "ampersand";
		case LMC_KEY_QUOTE:
			return "quote";
		case LMC_KEY_LEFTPAREN:
			return "left_parenthesis";
		case LMC_KEY_RIGHTPAREN:
			return "right_parenthesis";
		case LMC_KEY_ASTERISK:
			return "asterisk";
		case LMC_KEY_PLUS:
			return "plus";
		case LMC_KEY_COMMA:
			return "comma";
		case LMC_KEY_MINUS:
			return "minus";
		case LMC_KEY_PERIOD:
			return "period";
		case LMC_KEY_SLASH:
			return "slash";
		case LMC_KEY_0:
			return "0";
		case LMC_KEY_1:
			return "1";
		case LMC_KEY_2:
			return "2";
		case LMC_KEY_3:
			return "3";
		case LMC_KEY_4:
			return "4";
		case LMC_KEY_5:
			return "5";
		case LMC_KEY_6:
			return "6";
		case LMC_KEY_7:
			return "7";
		case LMC_KEY_8:
			return "8";
		case LMC_KEY_9:
			return "9";
		case LMC_KEY_COLON:
			return "colon";
		case LMC_KEY_SEMICOLON:
			return "semicolon";
		case LMC_KEY_LESS:
			return "less_than";
		case LMC_KEY_EQUALS:
			return "equals";
		case LMC_KEY_GREATER:
			return "greater_than";
		case LMC_KEY_QUESTION:
			return "question";
		case LMC_KEY_AT:
			return "at";
		case LMC_KEY_LEFTBRACKET:
			return "left_bracket";
		case LMC_KEY_BACKSLASH:
			return "backslash";
		case LMC_KEY_RIGHTBRACKET:
			return "right_bracket";
		case LMC_KEY_CARET:
			return "caret";
		case LMC_KEY_UNDERSCORE:
			return "underscore";
		case LMC_KEY_BACKQUOTE:
			return "backquote";
		case LMC_KEY_A:
			return "a";
		case LMC_KEY_B:
			return "b";
		case LMC_KEY_C:
			return "c";
		case LMC_KEY_D:
			return "d";
		case LMC_KEY_E:
			return "e";
		case LMC_KEY_F:
			return "f";
		case LMC_KEY_G:
			return "g";
		case LMC_KEY_H:
			return "h";
		case LMC_KEY_I:
			return "i";
		case LMC_KEY_J:
			return "j";
		case LMC_KEY_K:
			return "k";
		case LMC_KEY_L:
			return "l";
		case LMC_KEY_M:
			return "m";
		case LMC_KEY_N:
			return "n";
		case LMC_KEY_O:
			return "o";
		case LMC_KEY_P:
			return "p";
		case LMC_KEY_Q:
			return "q";
		case LMC_KEY_R:
			return "r";
		case LMC_KEY_S:
			return "s";
		case LMC_KEY_T:
			return "t";
		case LMC_KEY_U:
			return "u";
		case LMC_KEY_V:
			return "v";
		case LMC_KEY_W:
			return "w";
		case LMC_KEY_X:
			return "x";
		case LMC_KEY_Y:
			return "y";
		case LMC_KEY_Z:
			return "z";
		case LMC_KEY_LEFTBRACE:
			return "left_brace";
		case LMC_KEY_BAR:
			return "bar";
		case LMC_KEY_RIGHTBRACE:
			return "right_brace";
		case LMC_KEY_TILDE:
			return "tilde";
		case LMC_KEY_DELETE:
			return "delete";
		case LMC_KEY_KP0:
			return "keypad_0";
		case LMC_KEY_KP1:
			return "keypad_1";
		case LMC_KEY_KP2:
			return "keypad_2";
		case LMC_KEY_KP3:
			return "keypad_3";
		case LMC_KEY_KP4:
			return "keypad_4";
		case LMC_KEY_KP5:
			return "keypad_5";
		case LMC_KEY_KP6:
			return "keypad_6";
		case LMC_KEY_KP7:
			return "keypad_7";
		case LMC_KEY_KP8:
			return "keypad_8";
		case LMC_KEY_KP9:
			return "keypad_9";
		case LMC_KEY_KP_PERIOD:
			return "keypad_period";
		case LMC_KEY_KP_DIVIDE:
			return "keypad_divide";
		case LMC_KEY_KP_MULTIPLY:
			return "keypad_multiply";
		case LMC_KEY_KP_MINUS:
			return "keypad_minus";
		case LMC_KEY_KP_PLUS:
			return "keypad_plus";
		case LMC_KEY_KP_ENTER:
			return "keypad_enter";
		case LMC_KEY_KP_EQUALS:
			return "keypad_equals";
		case LMC_KEY_UP:
			return "up";
		case LMC_KEY_DOWN:
			return "down";
		case LMC_KEY_RIGHT:
			return "right";
		case LMC_KEY_LEFT:
			return "left";
		case LMC_KEY_INSERT:
			return "insert";
		case LMC_KEY_HOME:
			return "home";
		case LMC_KEY_END:
			return "end";
		case LMC_KEY_PAGEUP:
			return "page_up";
		case LMC_KEY_PAGEDOWN:
			return "page_down";
		case LMC_KEY_F1:
			return "f1";
		case LMC_KEY_F2:
			return "f2";
		case LMC_KEY_F3:
			return "f3";
		case LMC_KEY_F4:
			return "f4";
		case LMC_KEY_F5:
			return "f5";
		case LMC_KEY_F6:
			return "f6";
		case LMC_KEY_F7:
			return "f7";
		case LMC_KEY_F8:
			return "f8";
		case LMC_KEY_F9:
			return "f9";
		case LMC_KEY_F10:
			return "f10";
		case LMC_KEY_F11:
			return "f11";
		case LMC_KEY_F12:
			return "f12";
		case LMC_KEY_F13:
			return "f13";
		case LMC_KEY_F14:
			return "f14";
		case LMC_KEY_F15:
			return "f15";
		case LMC_KEY_NUMLOCK:
			return "num_lock";
		case LMC_KEY_CAPSLOCK:
			return "caps_lock";
		case LMC_KEY_SCROLLOCK:
			return "scroll_lock";
		case LMC_KEY_RSHIFT:
			return "right_shift";
		case LMC_KEY_LSHIFT:
			return "left_shift";
		case LMC_KEY_RCTRL:
			return "right_control";
		case LMC_KEY_LCTRL:
			return "left_control";
		case LMC_KEY_RALT:
			return "right_alt";
		case LMC_KEY_LALT:
			return "left_alt";
		case LMC_KEY_RMETA:
			return "right_meta";
		case LMC_KEY_LMETA:
			return "left_meta";
		case LMC_KEY_LSUPER:
			return "left_super";
		case LMC_KEY_RSUPER:
			return "right_super";
		case LMC_KEY_MODE:
			return "mode";
		case LMC_KEY_COMPOSE:
			return "compose";
		case LMC_KEY_HELP:
			return "help";
		case LMC_KEY_PRINT:
			return "print";
		case LMC_KEY_SYSREQ:
			return "system_request";
		case LMC_KEY_BREAK:
			return "break";
		case LMC_KEY_MENU:
			return "menu";
		case LMC_KEY_POWER:
			return "power";
		case LMC_KEY_EURO:
			return "euro";
		case LMC_KEY_UNDO:
			return "undo";
		case LMC_KEY_OEM_102:
			return "oem_102";
		default:
			return "null";
	}
}

/* Get key code setting string from enumeration value. */
static const char* GetKeyCodeSettingFromEnum(LMC_KeyCode key)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	if (key >= MAX_KEYBOARD_INPUTS || key < LMC_KEY_NONE)
	{
		return "null";
	}

	if (legacy_machine->input->keyboard->state.key_codes[key] == 0)
	{
		lmc_trace(LMC_LOG_VERBOSE,
			"[Input]: The %s key is not supported on this platform or with this driver. Setting to null instead.",
			GetKeyCodeStringFromEnum(key));
		return "null";
	}

	return GetKeyCodeStringFromEnum(key);
}

/**************************************************************************************************
 * LegacyMachine Input Management
 *************************************************************************************************/

/* Gets the state of a given input. */
bool LMC_GetJoypadInput(LMC_JoypadInput input)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	const LMC_Player player = (LMC_Player)(input >> 5);

	const uint32_t mask =
		(legacy_machine->input->joypad->state[player].digital_inputs & (1 << (input & INPUT_MASK)));

	if (mask)
		return true;
	return false;
}

/* Defines a physical keyboard key for a given keyboard input action. */
void LMC_DefineKeyboardKeyInput(LMC_KeyboardInput input, LMC_KeyCode keycode, bool update_settings)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	SettingsManager* settings = GetSettingsManagerContext();

	legacy_machine->input->keyboard->state.key_map[input] =
		legacy_machine->input->keyboard->state.key_codes[keycode];

	if (settings->initialized)
	{
		if (update_settings)
		{
			switch (input)
			{
				case LMC_KEYBOARD_INPUT_QUIT:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_quit_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_KEYBOARD_INPUT_HOME:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_home_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_KEYBOARD_INPUT_SAVE:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_state_save_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_KEYBOARD_INPUT_LOAD:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_state_load_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_KEYBOARD_INPUT_CRT:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_toggle_crt_effect_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_KEYBOARD_INPUT_FULLSCREEN:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_toggle_fullscreen_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				default:
				{
					lmc_trace(LMC_LOG_VERBOSE,
						"[Input]: Invalid keyboard input action specified. No setting was updated.");
					break;
				}
			}
		}
	}
}

/* Assigns a logical axis index to a physical joypad axis. */
void LMC_AssignJoypadAxisIndex(LMC_Player player, LMC_AxisIndex logical_axis, uint8_t physical_axis)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	legacy_machine->input->joypad->state[player].axis_indices[logical_axis] = physical_axis;
}

/* Defines a keyboard key for a given joypad input action. */
void LMC_DefineJoypadKeyInput(LMC_JoypadInput input, LMC_KeyCode keycode, bool update_settings)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	SettingsManager* settings = GetSettingsManagerContext();

	legacy_machine->input->joypad->state[0].key_map[input & INPUT_MASK] =
		legacy_machine->input->keyboard->state.key_codes[keycode];

	if (settings->initialized)
	{
		if (update_settings)
		{
			switch (input)
			{
				case LMC_JOYPAD_INPUT_B:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_b_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_Y:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_y_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_SELECT:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_select_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_START:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_start_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_UP:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_up_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_DOWN:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_down_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_LEFT:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_left_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_RIGHT:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_right_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_A:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_a_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_X:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_x_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_L:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_l_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_R:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_r_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_L2:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_l2_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_R2:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_r2_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_L3:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_l3_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_R3:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_r3_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_L_ANALOG_UP:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_l_analog_up_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_L_ANALOG_DOWN:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_l_analog_down_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_L_ANALOG_LEFT:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_l_analog_left_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_L_ANALOG_RIGHT:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_l_analog_right_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_R_ANALOG_UP:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_r_analog_up_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_R_ANALOG_DOWN:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_r_analog_down_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_R_ANALOG_LEFT:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_r_analog_left_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				case LMC_JOYPAD_INPUT_R_ANALOG_RIGHT:
				{
					SetStringSetting(settings,
						settings->strings.input_keyboard_player1_r_analog_right_key,
						GetKeyCodeSettingFromEnum(keycode));
					break;
				}
				default:
				{
					lmc_trace(LMC_LOG_VERBOSE,
						"[Input]: Invalid joypad input action specified. No setting was updated.");
					break;
				}
			}
		}
	}
}

/* Defines a joypad button for a given joypad input action. */
void LMC_DefineJoypadButtonInput(LMC_Player player, LMC_JoypadInput input, uint8_t joybutton)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	legacy_machine->input->joypad->state[player].button_map[input & INPUT_MASK] = joybutton;
}

/* Defines a joypad hat direction for a given joypad input action. */
void LMC_DefineJoypadHatInput(LMC_Player player, int hat_index, LMC_JoypadInput input,
	LMC_HatDirection hat_direction)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	legacy_machine->input->joypad->state[player].hat_map[hat_index][(int)hat_direction] =
		(uint8_t)input;
}

/* Defines a joypad axis direction for a given joypad input action. */
void LMC_DefineJoypadAxisInput(LMC_Player player, int axis_index, LMC_JoypadInput input,
	LMC_AxisDirection axis_direction)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	legacy_machine->input->joypad->state[player].axis_map[axis_index][(int)axis_direction] =
		(uint8_t)input;
}

/* Get the last pressed input button. */
int LMC_GetLastInput(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	int value = legacy_machine->input->last_input;
	legacy_machine->input->last_input = LMC_JOYPAD_INPUT_NONE;
	return value;
}
