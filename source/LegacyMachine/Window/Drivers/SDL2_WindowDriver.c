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

#include "../WindowDriver.h"
#include "../../MainEngine.h"
#include "../../Logging.h"

/**************************************************************************************************
 * SDL2 Window Variables
 *************************************************************************************************/

/* SDL window variables */

static SDL_Window*	window = NULL;
static SDL_Thread*	thread = NULL;
static SDL_mutex*	lock = NULL;
static SDL_cond*	cond = NULL;

/* General window variables */

static char*	window_title = NULL;

/**************************************************************************************************
 * SDL2 Window Structures
 *************************************************************************************************/

/* Window information. */
static WindowInfo window_info = { 0 };

/* Viewport information. */
static ViewportInfo viewport_info = { 0 };

/**************************************************************************************************
 * Prototypes
 *************************************************************************************************/

/* Local prototypes */

static bool SDL2_InitializeWindow(void);
static void SDL2_CloseWindow(void);

/**************************************************************************************************
 * Local Window Functions
 *************************************************************************************************/

/* Resize dimensions to aspect ratio. */
static void ResizeToAspect(double aspect, int src_width, int src_height,
	int* dst_width, int* dst_height, SDL_DisplayMode* mode)
{
	*dst_width = src_width;
	*dst_height = src_height;

	if (aspect <= 0)
		aspect = (double)src_width / src_height;

	if (!(window_info.flags & LMC_CWF_FULLSCREEN))
	{
		if ((float)src_width / src_height < 1)
			*dst_width = *dst_height * aspect;
		else
			*dst_height = *dst_width / aspect;
	}
	else
	{
		*dst_width = mode->w;
		*dst_height = *dst_width / aspect;
		if (*dst_height > mode->h)
		{
			*dst_height = mode->h;
			*dst_width = *dst_height * aspect;
		}
	}
}

/* Calculate windowed dimensions. */
static void CalculateWindowedDimensions(double aspect, int width, int height, SDL_DisplayMode* mode)
{
	if ((window_info.override_width > 0) && (window_info.override_height > 0))
	{
		/* Size window according to dimension overrides */
		window_info.width = window_info.override_width;
		window_info.height = window_info.override_height;
	}
	else
	{
		/* Resize dimensions to aspect ratio. */
		ResizeToAspect(aspect, width * 1, height * 1, &window_info.width, &window_info.height, mode);
	}
	window_info.scale_factor = (window_info.flags >> 2) & 0x07;
	/* If a scale isn't provided, calculate an optimal one based on client screen dimensions. */
	if (!window_info.scale_factor)
	{
		window_info.scale_factor = 1;
		while (width * (window_info.scale_factor + 1) < mode->w && height * (window_info.scale_factor + 1) < mode->h && window_info.scale_factor < 3)
			window_info.scale_factor++;
	}

	window_info.width *= window_info.scale_factor;
	window_info.height *= window_info.scale_factor;

	viewport_info.x = 0;
	viewport_info.y = 0;
	viewport_info.w = window_info.width;
	viewport_info.h = window_info.height;
}

/* Calculate fullscreen dimensions. */
static void CalculateFullscreenDimensions(double aspect, int width, int height, SDL_DisplayMode* mode)
{
	if ((window_info.override_width > 0) && (window_info.override_height > 0))
	{
		/* Resize dimensions to aspect ratio based on overrides. */
		ResizeToAspect(window_info.override_aspect, mode->w, mode->h, &window_info.width, &window_info.height, mode);
		if (!window_info.scale_factor)
			window_info.scale_factor = window_info.height / window_info.height;
	}
	else
	{
		/* Resize dimensions to aspect ratio. */
		ResizeToAspect(aspect, mode->w, mode->h, &window_info.width, &window_info.height, mode);
		if (!window_info.scale_factor)
			window_info.scale_factor = window_info.height / height;
	}

	viewport_info.x = (mode->w - window_info.width) >> 1;
	viewport_info.y = (mode->h - window_info.height) >> 1;
	viewport_info.w = window_info.width;
	viewport_info.h = window_info.height;
}

/**************************************************************************************************
 * SDL2 Window Functions
 *************************************************************************************************/

/* Create a window and initialize video and input. */
static bool SDL2_InitializeWindow(void)
{
	SDL_DisplayMode mode;
	SDL_Surface* surface = NULL;
	int flags;
	char quality[2] = { 0 };
	Uint32 format = 0;
	void* pixels;
	int pitch;

	/* Gets desktop size and maximum window size. */
	SDL_GetDesktopDisplayMode(0, &mode);

	if (!(window_info.flags & LMC_CWF_FULLSCREEN))
	{
		flags = 0;

		CalculateWindowedDimensions(legacy_machine->video->frame->aspect_ratio, legacy_machine->video->frame->base_width, legacy_machine->video->frame->base_height, &mode);
	}
	else
	{
		flags = SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_BORDERLESS;
#if SDL_VERSION_ATLEAST(2,0,5)
		flags |= SDL_WINDOW_ALWAYS_ON_TOP;
#endif
		CalculateFullscreenDimensions(legacy_machine->video->frame->aspect_ratio, legacy_machine->video->frame->base_width, legacy_machine->video->frame->base_height, &mode);
	}

	/* Set video viewport dimensions. */
	legacy_machine->video->cb_set_viewport(viewport_info.x, viewport_info.y, viewport_info.w, viewport_info.h);

	/* Create window. */
	if (window_title == NULL)
		window_title = strdup(legacy_machine->settings->program_name);
	window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_info.width, window_info.height, flags);
	if (!window)
	{
		LMC_SetLastError(LMC_ERR_FAIL_WINDOW_INIT);
		lmc_trace(LMC_LOG_ERRORS, "Failed to create window: %s", SDL_GetError());
		SDL2_CloseWindow();
		return false;
	}
	window_info.identifier = SDL_GetWindowID(window);

	/* Initialize video. */
	if (!legacy_machine->video->cb_init())
	{
		SDL2_CloseWindow();
		return false;
	}

	if (window_info.flags & LMC_CWF_FULLSCREEN)
		SDL_ShowCursor(SDL_DISABLE);

	/* One time init, avoid being forgotten in Alt+TAB. */
	if (legacy_machine->window->initialized == false)
	{
		legacy_machine->input->cb_init();
		legacy_machine->input->initialized = true;
	}

	/* Window is initialized. */
	legacy_machine->window->initialized = true;

	/* Window is running. */
	window_info.running = true;

	return true;
}

/* Destroy window delegate and free associated video and input data. */
static void SDL2_CloseWindow(void)
{
	if (legacy_machine->video)
	{
		legacy_machine->video->cb_deinit();
	}

	if (window)
	{
		SDL_DestroyWindow(window);
		window = NULL;
	}
}

/* Update window dimensions to aspect. */
static void SDL2_ResizeToAspect(const struct retro_game_geometry* geometry)
{
	if (window)
	{
		SDL_DisplayMode mode;

		/* Get desktop size and maximum window size. */
		SDL_GetDesktopDisplayMode(0, &mode);

		/* Calculate screen dimensions. */
		if (!(window_info.flags & LMC_CWF_FULLSCREEN))
		{
			CalculateWindowedDimensions(geometry->aspect_ratio, 
				geometry->base_width, geometry->base_height, &mode);
		}
		else
		{
			CalculateFullscreenDimensions(geometry->aspect_ratio, 
				geometry->base_width, geometry->base_height, &mode);
		}

		/* Update viewport for video. */
		legacy_machine->video->cb_set_viewport(viewport_info.x, 
			viewport_info.y, viewport_info.w, viewport_info.h);

		/* Update Window size. */
		SDL_SetWindowSize(window, window_info.width, window_info.height);
	}
}

/* Sets window title. */
static void SDL2_SetWindowTitle(const char* title)
{
	if (window != NULL)
		SDL_SetWindowTitle(window, title);
	if (window_title != NULL)
	{
		free(window_title);
		window_title = NULL;
	}
	if (title != NULL)
		window_title = strdup(title);
}

/* Process window and associated video and input events. */
static bool SDL2_ProcessEvents(void)
{
	SDL_Event evt;
	SDL_KeyboardEvent* keybevt;
	SDL_JoyButtonEvent* joybuttonevt;
	SDL_JoyHatEvent* joyhatevt;
	SDL_JoyAxisEvent* joyaxisevt;
	int input = 0;
	int c;

	if (!window_info.running)
		return false;

	/* dispatch message queue */
	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
		case SDL_QUIT:
			window_info.running = false;
			break;

		case SDL_WINDOWEVENT:
			switch (evt.window.event) {
			case SDL_WINDOWEVENT_CLOSE:
				window_info.running = false;
				break;
			}
			break;

		case SDL_KEYDOWN:
			keybevt = (SDL_KeyboardEvent*)&evt;
			if (keybevt->repeat != 0)
				break;

			/* Special inputs */
			if (legacy_machine->input->joypad->state[LMC_PLAYER1].keyboard_enabled == true)
			{
				if (keybevt->keysym.sym == legacy_machine->input->joypad->state[LMC_PLAYER1].key_map[LMC_INPUT_QUIT])
					window_info.running = false;
				else if (keybevt->keysym.sym == legacy_machine->input->joypad->state[LMC_PLAYER1].key_map[LMC_INPUT_CRT])
				{
					legacy_machine->video->filter->cb_toggle_crt();
				}
				else if (keybevt->keysym.sym == SDLK_RETURN && keybevt->keysym.mod & KMOD_ALT)
				{
					SDL2_CloseWindow();
					legacy_machine->window->params->flags ^= LMC_CWF_FULLSCREEN;
					SDL2_InitializeWindow();
				}
			}
			/* Regular user input */
			for (c = LMC_PLAYER1; c < MAX_PLAYERS; c++)
			{
				if (legacy_machine->input->joypad->state[c].keyboard_enabled == true)
					legacy_machine->input->joypad->cb_process_key((LMC_Player)c, keybevt->keysym.sym, keybevt->state);
			}
			break;

		case SDL_KEYUP:
			keybevt = (SDL_KeyboardEvent*)&evt;
			for (c = LMC_PLAYER1; c < MAX_PLAYERS; c++)
			{
				if (legacy_machine->input->joypad->state[c].keyboard_enabled == true)
					legacy_machine->input->joypad->cb_process_key((LMC_Player)c, keybevt->keysym.sym, keybevt->state);
			}
			break;

		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			joybuttonevt = (SDL_JoyButtonEvent*)&evt;
			for (c = LMC_PLAYER1; c < MAX_PLAYERS; c++)
			{
				if (legacy_machine->input->joypad->state[c].connected == true && legacy_machine->input->joypad->state[c].identifier == joybuttonevt->which)
					legacy_machine->input->joypad->cb_process_button((LMC_Player)c, joybuttonevt->button, joybuttonevt->state);
			}
			break;
		case SDL_JOYHATMOTION:
			joyhatevt = (SDL_JoyHatEvent*)&evt;
			for (c = LMC_PLAYER1; c < MAX_PLAYERS; c++)
			{
				if (legacy_machine->input->joypad->state[c].connected == true && legacy_machine->input->joypad->state[c].identifier == joyhatevt->which)
					legacy_machine->input->joypad->cb_process_hat((LMC_Player)c, joyhatevt->hat, joyhatevt->value);
			}
			break;
		case SDL_JOYAXISMOTION:
			joyaxisevt = (SDL_JoyAxisEvent*)&evt;
			for (c = LMC_PLAYER1; c < MAX_PLAYERS; c++)
			{
				if (legacy_machine->input->joypad->state[c].connected == true && legacy_machine->input->joypad->state[c].identifier == joyaxisevt->which)
					legacy_machine->input->joypad->cb_process_axis((LMC_Player)c, joyaxisevt->axis, joyaxisevt->value);
			}
			break;
		case SDL_JOYDEVICEADDED:
			legacy_machine->input->joypad->cb_connect(evt.jdevice.which);
			break;
		case SDL_JOYDEVICEREMOVED:
			legacy_machine->input->joypad->cb_disconnect(evt.jdevice.which);
			break;
		}
	}

	/* delete */
	if (!window_info.running)
		LMC_DeleteWindow();

	return LMC_IsWindowActive();
}

/**************************************************************************************************
 * SDL2 Window Driver
 *************************************************************************************************/

WindowDriver sdl2_window_driver = {
	SDL2_InitializeWindow,
	SDL2_ProcessEvents,
	SDL2_CloseWindow,
	SDL2_ResizeToAspect,
	SDL2_SetWindowTitle,
	&window_info,
	&viewport_info,
	false
};