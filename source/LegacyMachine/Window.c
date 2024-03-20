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
#ifdef HAVE_SDL2
#include <SDL.h>
#endif

#include "LegacyMachine.h"
#include "MainEngine.h"

/**************************************************************************************************
 * LegacyMachine Window Management
 *************************************************************************************************/

/*!
 * \brief
 * Creates a window for rendering.
 *
 * \param cwf_flags
 * Mask of the possible window creation flags:
 * LMC_CWF_FULLSCREEN, LMC_CWF_VSYNC, LMC_CWF_S1 - LMC_CWF_S5 (scaling factor, none = auto max).
 *
 * \returns
 * True if window was created or false if error.
 *
 * Creates a host window for LegacyMachine. If fullscreen, it uses the desktop
 * resolution and stretches the output resolution with aspect correction, letterboxing or pillarboxing
 * as needed. If windowed, it creates a centered window that is the maximum possible integer multiple of
 * either the resolution configured at LMC_Init() or from the core/content loaded from LMC_LoadCore() and
 * LMC_LoadContent().
 *
 * \see
 * LMC_DeleteWindow(), LMC_ProcessWindow()
 */
bool LMC_CreateWindow(int flags)
{
	WindowFlags creation_flags;
	bool ok;

	creation_flags.value = flags;

	legacy_machine->video->info.fullscreen = creation_flags.fullscreen;
	legacy_machine->video->info.vsync = creation_flags.vsync;
	legacy_machine->video->info.smooth = !creation_flags.nearest;
	legacy_machine->window->params.factor = creation_flags.factor;

	/* Allow single instance. */
	if (legacy_machine->window->params.instances)
	{
		legacy_machine->window->params.instances++;
		return true;
	}

#ifdef HAVE_SDL2
	/* Initialize required SDL sub-systems. */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
		return false;
#endif

	/* fill parameters for window creation and video intialization. */
	legacy_machine->video->filter.enabled = (flags & LMC_CWF_NEAREST) == 0;

#if defined HAVE_MENU
	/* Initialize video for frontend menu. */
	legacy_machine->video->cb_set_geometry_fmt(&legacy_machine->menu->av_info);
	legacy_machine->video->cb_set_pixel_fmt(RETRO_PIXEL_FORMAT_XRGB8888);
	legacy_machine->video->info.frame->pitch = legacy_machine->menu->frame.pitch;

	ok = legacy_machine->window->cb_init();
#else
	ok = true;
#endif

	if (ok)
		legacy_machine->window->params.instances++;
	return ok;
}

/*!
 * \brief
 * Deletes the window and all associated data (Video, Audio, and Input) previoulsy created with LMC_CreateWindow().
 *
 * \see
 * LMC_CreateWindow()
 */
void LMC_DeleteWindow(void)
{
	/* Single instance, delete when 0 is reached. */
	if (!legacy_machine->window->params.instances)
		return;
	legacy_machine->window->params.instances--;
	if (legacy_machine->window->params.instances)
		return;

	/* Close core if one is active and running. */
	if (LMC_IsCoreRunning())
		LMC_CloseCore();

	/* Close the window. */
	legacy_machine->window->cb_deinit();

#ifdef HAVE_SDL2
	/* Quit SDL. */
	SDL_Quit();
#endif
	printf(" ");
}

/*!
 * \brief
 * Processes all events related to a window created with LMC_CreateWindow().
 *
 * \returns
 * True if window is active or false if the user has requested to end the application (by pressing Esc key
 * or clicking the close button)
 *
 * \see
 * LMC_CreateWindow()
 */
bool LMC_ProcessWindow(void)
{
	return legacy_machine->window->cb_process();
}

/*!
 * \brief
 * Checks window state
 *
 * \returns
 * True if window is active and running or false if the user has requested to end the application (by pressing Esc key
 * or clicking the close button)
 *
 * \see
 * LMC_CreateWindow()
 */
bool LMC_IsWindowActive(void)
{
	return legacy_machine->window->params.running;
}

/*!
 * \brief
 * Sets window title.
 *
 * \param title
 * Text with the title to set.
 *
 */
void LMC_SetWindowTitle(const char* title)
{
	legacy_machine->window->cb_set_title(title);
}

/*!
 * \brief
 * Sets dimension overrides to force a specific window size.
 * 
 * \param width
 * Window width to force.
 * 
 * \param title
 * Window height to force.
 * 
 */
void LMC_SetBaseDimensionOverrides(int width, int height)
{
	legacy_machine->window->params.override_width = width;
	legacy_machine->window->params.override_height = height;
	legacy_machine->window->params.override_aspect = (double)width / height;
}

/*!
 * \brief
 * Returns horizontal dimension of window after scaling.
 */
int LMC_GetWindowWidth(void)
{
	return legacy_machine->window->params.width;
}

/*!
 * \brief
 * Returns vertical dimension of window after scaling.
 */
int LMC_GetWindowHeight(void)
{
	return legacy_machine->window->params.height;
}

/*!
 * \brief
 * Returns the number of milliseconds since application start.
 */
uint64_t LMC_GetTicks(void)
{
	return legacy_machine->video->cb_get_ticks();
}

/*!
 * \brief
 * Suspends execution for a fixed time.
 *
 * \param time Number of milliseconds to wait.
 */
void LMC_Delay(uint32_t time)
{
	legacy_machine->video->cb_set_delay(time);
}

/**************************************************************************************************
 * LegacyMachine CRT Effect Management
 *************************************************************************************************/

void LMC_EnableRFBlur(bool mode)
{
	CRTFilter* filter = GetVideoFilter();
	filter->cb_enable_rf(mode);
}

void LMC_ConfigCRTEffect(LMC_CRT type, bool blur)
{
	CRTFilter* filter = GetVideoFilter();
	filter->cb_config_crt(type, blur);
}

void LMC_DisableCRTEffect(void)
{
	CRTFilter* filter = GetVideoFilter();
	filter->cb_deinit_crt();
}

/**************************************************************************************************
 * LegacyMachine Input Management
 *************************************************************************************************/

/*!
 * \brief
 * Returns the state of a given input.
 *
 * \param input
 * Input to check state. It can be one of the following values:
 *	 * LMC_INPUT_UP
 *	 * LMC_INPUT_DOWN
 *	 * LMC_INPUT_LEFT
 *	 * LMC_INPUT_RIGHT
 *	 * LMC_INPUT_A
 *   * LMC_INPUT_B
 *   * LMC_INPUT_X
 *   * LMC_INPUT_Y
 *   * LMC_INPUT_L
 *   * LMC_INPUT_R
 *   * LMC_INPUT_L2
 *   * LMC_INPUT_R2
 *   * LMC_INPUT_L3
 *   * LMC_INPUT_R3
 *   * LMC_INPUT_SELECT
 *	 * LMC_INPUT_START
 *	 * Optionally combine with LMC_INPUT_P1 to LMC_INPUT_P8 to request input for specific player
 *
 * \returns
 * True if that input is pressed or false if not.
 *
 * If a window has been created with LMC_CreateWindow(), it provides basic user input.
 * It mirrors libretro's JOYPAD setup where INPUT_(button name) corresponds to
 * RETRO_DEVICE_ID_JOYPAD_(button name). By default directional buttons are mapped to
 * the keyboard arrows and the action buttons (B, A, X, Y) are mapped to the keys
 * Z, X, A, S. L and R are mapped to the keys Q and W. 
 *
 * \see
 * LMC_CreateWindow(), LMC_DefineInputKey(), LMC_DefineInputButton()
 */
bool LMC_GetInput(LMC_Input input)
{
	const LMC_Player player = (LMC_Player)(input >> 5);
	const uint32_t mask = (legacy_machine->input->joypad->state[player].inputs & (1 << (input & INPUT_MASK)));
	if (mask)
		return true;
	return false;
}

/*!
 * \brief
 * Enables or disables using the keyboard as joypad input for specified player.
 *
 * \param player
 * Player number to enable (PLAYER1 - PLAYER4).
 *
 * \param enable
 * Set true to enable, false to disable.
 */
void LMC_EnableKeyboardAsJoypadInput(LMC_Player player, bool enable)
{
	legacy_machine->input->joypad->state[player].keyboard_enabled = enable;
}

/*!
 * \brief
 * Assigns a joypad index to the specified player.
 *
 * \param player
 * Player number to configure (PLAYER1 - PLAYER4).
 *
 * \param index
 * Joyspad index to assign, 0-based index. -1 = disable.
 */
void LMC_AssignInputJoypad(LMC_Player player, int index)
{
	legacy_machine->input->joypad->cb_assign_player(player, index);
}

/*!
 * \brief
 * Assigns a keyboard input to use as joypad input when keyboard input is enabled.
 *
 * \param player
 * Player number to configure (LMC_PLAYER1 - LMC_PLAYER4).
 *
 * \param input
 * Input to associate to the given key.
 *
 * \param keycode
 * ASCII key value or scancode (Uses SDL keycodes as defined in SDL.h).
 */
void LMC_DefineJoypadInputKey(LMC_Player player, LMC_Input input, uint32_t keycode)
{
	legacy_machine->input->joypad->state[player].key_map[input & INPUT_MASK] = keycode;
}

/*!
 * \brief
 * Assigns a joypad button input to a player.
 *
 * \param player
 * Player number to configure (LMC_PLAYER1 - LMC_PLAYER4).
 *
 * \param input
 * Input to associate to the given button.
 *
 * \param joybutton
 * Button index.
 */
void LMC_DefineJoypadInputButton(LMC_Player player, LMC_Input input, uint8_t joybutton)
{
	legacy_machine->input->joypad->state[player].button_map[input & INPUT_MASK] = joybutton;
}

/*!
 * \brief
 * Assigns a joypad hat input to a player.
 *
 * \param player
 * Player number to configure (LMC_PLAYER1 - LMC_PLAYER4).
 *
 * \param hat_index
 * Index of joypad hat to assign to.
 * 
 * \param input
 * Input to associate to the given hat direction.
 *
 * \param hat_direction
 * Hat direction index.
 */
void LMC_DefineJoypadInputHat(LMC_Player player, int hat_index, LMC_Input input, LMC_HatDirection hat_direction)
{
	legacy_machine->input->joypad->state[player].hat_map[hat_index][(int)hat_direction] = (uint8_t)input;
}

/*!
 * \brief
 * Assigns a joypad axis input to a player.
 *
 * \param player
 * Player number to configure (LMC_PLAYER1 - LMC_PLAYER4).
 *
 * \param axis_index
 * Index of joypad axis to assign to.
 *
 * \param input
 * Input to associate to the given axis direction.
 *
 * \param hat_direction
 * Axis direction index.
 */
void LMC_DefineJoypadInputAxis(LMC_Player player, int axis_index, LMC_Input input, LMC_AxisDirection axis_direction)
{
	legacy_machine->input->joypad->state[player].axis_map[axis_index][(int)axis_direction] = (uint8_t)input;
}

/*!
 * \brief
 * Returns the last pressed input button.
 *
 * \see
 * LMC_GetInput()
 */
int LMC_GetLastInput(void)
{
	int value = legacy_machine->input->last_input;
	legacy_machine->input->last_input = LMC_INPUT_NONE;
	return value;
}