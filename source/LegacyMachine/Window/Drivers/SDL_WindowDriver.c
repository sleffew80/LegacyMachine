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

#include "../WindowManager.h"
#include "../../Common/SDL_Common.h"
#include "../../Video/VideoManager.h"
#include "../../MainEngine.h"
#include "../../Logging.h"

/**************************************************************************************************
 * Prototypes
 *************************************************************************************************/

/* Static prototypes */

static void SDL_DriverCloseWindow(void);

/**************************************************************************************************
 * SDL Window Internal Functions
 *************************************************************************************************/

/* Get player index from SDL mouse ID. */
static LMC_Player GetPlayerFromMouseID(uint32_t mouse_id)
{
	InputManager* input = GetInputManagerContext();
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (input->mouse->state[i].connected &&
			input->mouse->state[i].identifier == mouse_id)
		{
			return (LMC_Player)i;
		}
	}
	return LMC_PLAYER_1; /* Fallback to player 1 if mouse ID not found. */
}

/* Get player index from SDL joystick instance ID. */
static LMC_Player GetPlayerFromInstanceID(SDL_JoystickID instance_id)
{
	InputManager* input = GetInputManagerContext();

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (input->joypad->state[i].connected &&
			input->joypad->state[i].identifier == instance_id)
		{
			return (LMC_Player)i;
		}
	}

	return LMC_PLAYER_1; /* Fallback to player 1 if instance ID not found. */
}

/* Translate SDL key symbol to the LMC_KeyCode. */
static LMC_KeyCode TranslateKeyCode(uint32_t key_symbol)
{
	InputManager* input = GetInputManagerContext();

	for (int i = 0; i < MAX_KEY_INPUTS; i++)
	{
		if (input->keyboard->state.key_codes[i] == key_symbol)
			return (LMC_KeyCode)i;
	}

	return LMC_KEY_UNKNOWN;
}

/* Translate SDL hat state to internal hat state representation. */
static uint8_t TranslateHatState(uint8_t sdl_state)
{
	uint8_t state = 0;

	if (sdl_state & SDL_HAT_UP)    state |= (1 << LMC_HAT_UP);
	if (sdl_state & SDL_HAT_RIGHT) state |= (1 << LMC_HAT_RIGHT);
	if (sdl_state & SDL_HAT_DOWN)  state |= (1 << LMC_HAT_DOWN);
	if (sdl_state & SDL_HAT_LEFT)  state |= (1 << LMC_HAT_LEFT);

	return state;
}

/* Translate SDL key modifiers to libretro key modifier representation. */
static uint16_t TranslateKeyMod(SDL_Keymod sdl_mod)
{
	uint16_t mod = RETROKMOD_NONE;

	if (sdl_mod & KMOD_SHIFT) mod |= RETROKMOD_SHIFT;
	if (sdl_mod & KMOD_CTRL)  mod |= RETROKMOD_CTRL;
	if (sdl_mod & KMOD_ALT)   mod |= RETROKMOD_ALT;
	if (sdl_mod & KMOD_GUI)   mod |= RETROKMOD_META;
	if (sdl_mod & KMOD_NUM)   mod |= RETROKMOD_NUMLOCK;
	if (sdl_mod & KMOD_CAPS)  mod |= RETROKMOD_CAPSLOCK;

	return mod;
}

/* Initialize and create a window for rendering. */
static bool SDL_DriverCreateWindow(int flags)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	WindowManager* window = GetWindowManagerContext();
	RenderInfo* output = GetRenderInfo();
	SDL_VideoInfo* sdl_video = SDL_GetVideoInfoContext();
	uint32_t subsystem_flags = SDL_WasInit(0);
	SDL_DisplayMode mode;

	lmc_trace(LMC_LOG_VERBOSE, "[Window] [SDL]: Initializing window...");
	/* Initialize video subsystem, if necessary. */
	if (subsystem_flags == 0)
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			lmc_trace(LMC_LOG_ERRORS,
				"[Window] [SDL]: Failed to initialize video subsystem: %s", SDL_GetError());
			LMC_SetLastError(LMC_ERR_FAIL_VIDEO_INIT);
			return false;
		}
	}
	else if ((subsystem_flags & SDL_INIT_VIDEO) == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		{
			lmc_trace(LMC_LOG_ERRORS,
				"[Window] [SDL]: Failed to initialize video subsystem: %s", SDL_GetError());
			LMC_SetLastError(LMC_ERR_FAIL_VIDEO_INIT);
			return false;
		}
	}

	/* List available displays. */
	lmc_trace(LMC_LOG_VERBOSE, "[Window] [SDL]: Available displays:");
	if (legacy_machine->log_level >= LMC_LOG_VERBOSE)
	{
		for (unsigned i = 0; i < SDL_GetNumVideoDisplays(); ++i)
		{
			if (SDL_GetCurrentDisplayMode(i, &mode) < 0)
				printf("\tDisplay #%i: unknown\n", i);
			else
				printf("\tDisplay #%i: %ix%i @%ihz\n", i, mode.w, mode.h,
					mode.refresh_rate);
		}
	}

	/* Gets desktop size and maximum window size. */
	SDL_GetDesktopDisplayMode(0, &mode);

	lmc_trace(LMC_LOG_VERBOSE,
		"[Window] [SDL]: Using display 0: %ix%i @%ihz",
		mode.w, mode.h, mode.refresh_rate);

	/* Calculate window and screen dimensions. */
	if (!output->fullscreen)
	{
		CalculateWindowedDimensions(
			output->aspect_ratio,
			output->raw_frame.width,
			output->raw_frame.height,
			mode.w,
			mode.h
		);
	}
	else
	{
		CalculateFullscreenDimensions(
			output->aspect_ratio,
			output->raw_frame.width,
			output->raw_frame.height,
			mode.w,
			mode.h);
	}

	/* Add window title if one hasn't alrady been provided. */
	if (!window->params.title)
		window->params.title = strdup(legacy_machine->settings->program_name);

	/* Create window. */
	sdl_video->window = SDL_CreateWindow(window->params.title, 
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED, 
		window->params.width, 
		window->params.height, 
		flags);

	if (!sdl_video->window)
	{
		LMC_SetLastError(LMC_ERR_FAIL_WINDOW_INIT);
		return false;
	}
	window->params.identifier = SDL_GetWindowID(sdl_video->window);

	return true;
}

/* Finalize window initialization by initializing input and hiding cursor if in fullscreen. */
static bool SDL_DriverFinalizeWindow(void)
{
	WindowManager* window = GetWindowManagerContext();
	RenderInfo* output = GetRenderInfo();

	/* Hide cursor in fullscreen mode. */
	if (output->fullscreen)
		SetCursorVisibility(false);

	/* One time init, avoid being forgotten in Alt+TAB. */
	if (window->initialized == false)
	{
		if (!InitializeInput(INPUT_DRIVER_SDL))
			lmc_trace(LMC_LOG_ERRORS, "[Window][SDL]: Failed to initialize window");
	}

	lmc_trace(LMC_LOG_VERBOSE, "[Window] [SDL]: Window initialized successfully");
	return true;
}

/**************************************************************************************************
 * SDL Window Functions
 *************************************************************************************************/

/* Create a window and initialize video and input. */
static bool SDL_DriverInitializeWindow(void)
{
	WindowManager* window = GetWindowManagerContext();
	VideoManager* video = GetVideoManagerContext();
	ViewportInfo* viewport = GetViewportInfo();
	RenderInfo* output = GetRenderInfo();
	int flags = 0;

	if (output->fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_BORDERLESS;
#if SDL_VERSION_ATLEAST(2,0,5)
		flags |= SDL_WINDOW_ALWAYS_ON_TOP;
#endif
	}

	/* Create window. */
	if (!SDL_DriverCreateWindow(flags))
	{
		lmc_trace(LMC_LOG_ERRORS, "[Window] [SDL]: Failed to create window: %s", SDL_GetError());
		SDL_DriverCloseWindow();
		return false;
	}

	/* Initialize video. */
	if (!InitializeVideo(VIDEO_DRIVER_SDL, viewport))
	{
		SDL_DriverCloseWindow();
		return false;
	}

	/* Finalize window initialization and return result. */
	return SDL_DriverFinalizeWindow();
}

/* Destroy window delegate and free associated video and input data. */
static void SDL_DriverCloseWindow(void)
{
	SDL_VideoInfo* sdl_video = SDL_GetVideoInfoContext();

	lmc_trace(LMC_LOG_VERBOSE, "[Window] [SDL]: Closing window...");
	if (sdl_video->window)
	{
		SDL_DestroyWindow(sdl_video->window);
		sdl_video->window = NULL;
	}
}

/* Update window dimensions. */
static void SDL_DriverSetWindowSize(const struct retro_game_geometry* geometry)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	SDL_VideoInfo* sdl_video = SDL_GetVideoInfoContext();
	WindowInfo* window = GetWindowParameterInfo();
	ViewportInfo* viewport = GetViewportInfo();
	RenderInfo* output = GetRenderInfo();

	if (sdl_video->window)
	{
		SDL_DisplayMode mode;

		/* Get desktop size and maximum window size. */
		SDL_GetDesktopDisplayMode(0, &mode);

		/* Calculate screen dimensions. */
		if (!output->fullscreen)
		{
			CalculateWindowedDimensions(geometry->aspect_ratio,
				geometry->base_width, geometry->base_height, mode.w, mode.h);
		}
		else
		{
			CalculateFullscreenDimensions(geometry->aspect_ratio,
				geometry->base_width, geometry->base_height, mode.w, mode.h);
		}

		/* Update viewport for video. */
		SetVideoViewport(viewport);

		/* Update Window size. */
		SDL_SetWindowSize(sdl_video->window, window->width, window->height);
	}
}

/* Sets window title. */
static void SDL_DriverSetWindowTitle(const char* title)
{
	SDL_VideoInfo* sdl_video = SDL_GetVideoInfoContext();
	WindowInfo* window = GetWindowParameterInfo();

	if (sdl_video->window != NULL)
		SDL_SetWindowTitle(sdl_video->window, title);
	if (window->title != NULL)
	{
		free(window->title);
		window->title = NULL;
	}
	if (title != NULL)
		window->title = strdup(title);
}

/* Set the window's cursor visibility. */
static void SDL_DriverSetCursorVisibility(bool show)
{
	SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
}

/* Process window and associated video and input events. */
static bool SDL_DriverProcessEvents(void)
{
	WindowManager* window = GetWindowManagerContext();
	InputManager* input = GetInputManagerContext();
	RenderInfo* output = GetRenderInfo();
	CRTEffect* crt_effect = GetCRTEffect();
	SDL_Event event;
	SDL_KeyboardEvent* keyboard_event;
	SDL_JoyButtonEvent* joypad_button_event;
	SDL_JoyHatEvent* joypad_hat_event;
	SDL_JoyAxisEvent* joypad_axis_event;

	if (!window->running)
		return false;

	/* Dispatch message queue. */
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
			{
				window->running = false;
				break;
			}
			case SDL_WINDOWEVENT:
			{
				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_CLOSE:
						window->running = false;
						break;
				}
				break;
			}
			case SDL_KEYDOWN:
			{
				keyboard_event = (SDL_KeyboardEvent*)&event;
				if (keyboard_event->repeat != 0)
					break;

				/* Emergency core exit: Ctrl+Alt+Home closes the core when keyboard
				   interface input is suppressed and no other path back exists. */
				if (!input->keyboard->interface_input && LMC_IsCoreRunning())
				{
					if (keyboard_event->keysym.sym == SDLK_HOME &&
						(keyboard_event->keysym.mod & KMOD_CTRL) &&
						(keyboard_event->keysym.mod & KMOD_ALT))
					{
						LMC_CloseCore();
						break;
					}
				}

				if (input->keyboard->interface_input)
				{
					/* Special inputs */
					if (keyboard_event->keysym.sym == input->keyboard->state.key_map[LMC_KEYBOARD_INPUT_QUIT])
					{
						window->running = false;
					}
	#if defined HAVE_MENU
					else if (keyboard_event->keysym.sym == input->keyboard->state.key_map[LMC_KEYBOARD_INPUT_HOME])
					{
						if (LMC_IsCoreRunning())
							LMC_CloseCore();
					}
	#endif
					else if (keyboard_event->keysym.sym == input->keyboard->state.key_map[LMC_KEYBOARD_INPUT_SAVE])
					{
						if (LMC_IsCoreRunning())
							LMC_SaveState(LMC_SLOT_QUICK);
					}
					else if (keyboard_event->keysym.sym == input->keyboard->state.key_map[LMC_KEYBOARD_INPUT_LOAD])
					{
						if (LMC_IsCoreRunning())
							LMC_LoadState(LMC_SLOT_QUICK);
					}
					else if (keyboard_event->keysym.sym == input->keyboard->state.key_map[LMC_KEYBOARD_INPUT_CRT])
					{
						crt_effect->cb_toggle_crt();
					}
					else if (keyboard_event->keysym.sym == input->keyboard->state.key_map[LMC_KEYBOARD_INPUT_FULLSCREEN])
					{
						/* Get current driver ID, deinitialize window, toggle fullscreen, and reinitialize window
						   with same driver and new settings. */
						WindowDriverID driver_id = GetWindowDriverID();
						DeinitializeWindow();
						/* Toggle fullscreen. */
						output->fullscreen ^= 1;
						InitializeWindow(driver_id);
					}
				}

				/* Keyboard input for joypad. */
				if (input->keyboard->joypad_input)
				{
					ProcessJoypadKeyInput((uint32_t)keyboard_event->keysym.sym, keyboard_event->state == SDL_PRESSED);
				}

				/* Keyboard input for core. */
				if (LMC_IsCoreRunning())
				{
					ProcessKeyboardKeyInput((uint32_t)keyboard_event->keysym.sym, keyboard_event->state == SDL_PRESSED,
						TranslateKeyMod(keyboard_event->keysym.mod));
				}
				break;
			}
			case SDL_KEYUP:
			{
				keyboard_event = (SDL_KeyboardEvent*)&event;

				/* Keyboard input for joypad. */
				if (input->keyboard->joypad_input)
				{
					ProcessJoypadKeyInput((uint32_t)keyboard_event->keysym.sym, keyboard_event->state == SDL_PRESSED);
				}

				/* Keyboard input for core. */
				if (LMC_IsCoreRunning())
				{
					ProcessKeyboardKeyInput((uint32_t)keyboard_event->keysym.sym, keyboard_event->state == SDL_PRESSED,
						TranslateKeyMod(keyboard_event->keysym.mod));
				}
				break;
			}
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
			{
				joypad_button_event = (SDL_JoyButtonEvent*)&event;
				LMC_Player player = GetPlayerFromInstanceID(joypad_button_event->which);
				ProcessJoypadButtonInput(player, joypad_button_event->button, joypad_button_event->state == SDL_PRESSED);
				break;
			}
			case SDL_JOYHATMOTION:
			{
				joypad_hat_event = (SDL_JoyHatEvent*)&event;
				LMC_Player player = GetPlayerFromInstanceID(joypad_hat_event->which);
				uint8_t state = TranslateHatState(joypad_hat_event->value);
				ProcessJoypadHatInput(player, joypad_hat_event->hat, state);
				break;
			}
			case SDL_JOYAXISMOTION:
			{
				joypad_axis_event = (SDL_JoyAxisEvent*)&event;
				LMC_Player player = GetPlayerFromInstanceID(joypad_axis_event->which);
				ProcessJoypadAxisInput(player, joypad_axis_event->axis, joypad_axis_event->value);
				break;
			}
			case SDL_JOYDEVICEADDED:
			{
				/* event.jdevice.which is a device index for ADDED events, not an instance ID. */
				if (event.jdevice.which < MAX_PLAYERS)
					ConnectJoypad((LMC_Player)event.jdevice.which);
				break;
			}
			case SDL_JOYDEVICEREMOVED:
			{
				LMC_Player player = GetPlayerFromInstanceID(event.jdevice.which);
				DisconnectJoypad(player);
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				ProcessMouseButtonInput(LMC_PLAYER_1, event.button.button, event.button.state == SDL_PRESSED);
				break;
			}
			case SDL_MOUSEWHEEL:
			{
				ProcessMouseWheelInput(LMC_PLAYER_1, event.wheel.x, event.wheel.y);
				break;
			}
		}
	}

	/* Delete */
	if (!window->running)
		LMC_DeleteWindow();

	return LMC_IsWindowActive();
}

/**************************************************************************************************
 * SDL Window Driver
 *************************************************************************************************/

WindowDriver sdl_window_driver = {
	SDL_DriverInitializeWindow,		/*cb_init*/
	SDL_DriverProcessEvents,		/*cb_process*/
	SDL_DriverCloseWindow,			/*cb_deinit*/
	SDL_DriverSetWindowSize,		/*cb_resize*/
	SDL_DriverSetWindowTitle,		/*cb_set_title*/
	SDL_DriverSetCursorVisibility,	/*cb_show_cursor*/
	SDL_Delay,						/*cb_set_delay*/
	WINDOW_DRIVER_SDL
};
