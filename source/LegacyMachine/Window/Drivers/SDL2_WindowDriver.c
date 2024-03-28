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
#include "../../Common/SDL2_Common.h"
#include "../../Video/VideoDriver.h"
#include "../../MainEngine.h"
#include "../../Logging.h"

#if defined HAVE_OPENGL
#include "../../Common/GL_Common.h"
#endif

/**************************************************************************************************
 * Prototypes
 *************************************************************************************************/

/* Local prototypes */

static void SDL2_CloseWindow(void);
#if defined HAVE_OPENGL
static void SDL2_GL_CloseWindow(void);
#endif

/**************************************************************************************************
 * SDL2 Local Functions
 *************************************************************************************************/

static bool SDL2_CreateWindow(int flags)
{
	WindowInfo* window = GetWindowParameterInfo();
	VideoInfo* video = GetVideoInfo();
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	uint32_t subsystem_flags = SDL_WasInit(0);
	SDL_DisplayMode mode;

	/* Initialize video subsystem, if necessary. */
	if (subsystem_flags == 0)
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			lmc_trace(LMC_LOG_ERRORS, "[SDL2]: Failed to initialize video subsystem: %s", SDL_GetError());
			LMC_SetLastError(LMC_ERR_FAIL_VIDEO_INIT);
			return false;
		}
	}
	else if ((subsystem_flags & SDL_INIT_VIDEO) == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		{
			lmc_trace(LMC_LOG_ERRORS, "[SDL2]: Failed to initialize video subsystem: %s", SDL_GetError());
			LMC_SetLastError(LMC_ERR_FAIL_VIDEO_INIT);
			return false;
		}
	}

	/* List available displays. */
	lmc_trace(LMC_LOG_VERBOSE, "[SDL2]: Available displays:");
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

	lmc_trace(LMC_LOG_VERBOSE, "[SDL2]: Using display 0: %ix%i @%ihz", 
		mode.w, mode.h, mode.refresh_rate);

	/* Calculate window and screen dimensions. */
	if (!video->fullscreen)
	{
		CalculateWindowedDimensions(video->aspect_ratio, video->frame->width, video->frame->height, mode.w, mode.h);
	}
	else
	{
		flags = SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_BORDERLESS;
#if SDL_VERSION_ATLEAST(2,0,5)
		flags |= SDL_WINDOW_ALWAYS_ON_TOP;
#endif
		CalculateFullscreenDimensions(video->aspect_ratio, video->frame->width, video->frame->height, mode.w, mode.h);
	}

	/* Create window. */
	if (window->title == NULL)
		window->title = strdup(legacy_machine->settings->program_name);
	sdl2_video->window = SDL_CreateWindow(window->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window->width, window->height, flags);
	if (!sdl2_video->window)
	{
		LMC_SetLastError(LMC_ERR_FAIL_WINDOW_INIT);
		return false;
	}
	window->identifier = SDL_GetWindowID(sdl2_video->window);

	return true;
}

static bool SDL2_FinalizeWindow(void)
{
	WindowInfo* window = GetWindowParameterInfo();
	VideoInfo* video = GetVideoInfo();

	if (video->fullscreen)
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
	window->running = true;

	return true;
}

/**************************************************************************************************
 * SDL2 Window Functions
 *************************************************************************************************/

/* Create a window and initialize video and input. */
static bool SDL2_InitializeWindow(void)
{
	WindowInfo* window = GetWindowParameterInfo();
	ViewportInfo* viewport = GetViewportInfo();
	VideoInfo* video = GetVideoInfo();
	int flags = 0;

	/* Create window. */
	if (!SDL2_CreateWindow(flags))
	{
		lmc_trace(LMC_LOG_ERRORS, "[SDL2]: Failed to create window: %s", SDL_GetError());
		SDL2_CloseWindow();
		return false;
	}

	/* Set video viewport dimensions. */
	legacy_machine->video->cb_set_viewport(viewport->x, viewport->y, viewport->w, viewport->h);

	/* Initialize video. */
	if (!legacy_machine->video->cb_init())
	{
		SDL2_CloseWindow();
		return false;
	}

	/* Finalize window initialization and return result. */
	return SDL2_FinalizeWindow();
}

#if defined HAVE_OPENGL
/* Create an OpenGL window and initialize video and input. */
static bool SDL2_GL_InitializeWindow(void)
{
	SystemManager* system = GetSystemManagerContext();
	WindowInfo* window = GetWindowParameterInfo();
	ViewportInfo* viewport = GetViewportInfo();
	VideoInfo* video = GetVideoInfo();
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	GL_VideoInfo* gl_video = GL_GetVideoInfoContext();
	int flags = SDL_WINDOW_OPENGL;

	gl_video->shader = calloc(sizeof(GL_ShaderInfo), 1);

	/* TODO: Implement OpenGL version/type detection. Assuming modern CORE for now. */

	/* Initialize hardware rendering. */
	legacy_machine->video->hw_api.version_major = 4;
	legacy_machine->video->hw_api.version_minor = 6;
	legacy_machine->video->hw_context = RETRO_HW_CONTEXT_OPENGL_CORE;

	InitializeHardwareRenderCallback();

	/* Set OpenGL context attributes. */
	if (system->cb_hw_render.context_type == RETRO_HW_CONTEXT_OPENGL_CORE || system->cb_hw_render.version_major >= 3)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, system->cb_hw_render.version_major);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, system->cb_hw_render.version_minor);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	}

	/* Set SDL OpenGL context profile. */
	switch (system->cb_hw_render.context_type)
	{
	case RETRO_HW_CONTEXT_OPENGL_CORE:
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		break;
	case RETRO_HW_CONTEXT_OPENGLES2:
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		break;
	case RETRO_HW_CONTEXT_OPENGL:
		if (system->cb_hw_render.version_major >= 3)
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
		break;
	default:
		LMC_SetLastError(LMC_ERR_FAIL_WINDOW_INIT);
		lmc_trace(LMC_LOG_ERRORS,
			"[SDL2 GL]: Unsupported hardware context: %s",
			GetHardwareContextTypeString(system->cb_hw_render.context_type));
		SDL2_GL_CloseWindow();
		return false;
	}

	/* Create window. */
	if (!SDL2_CreateWindow(flags))
	{
		lmc_trace(LMC_LOG_ERRORS, "[SDL2]: Failed to create window: %s", SDL_GetError());
		SDL2_GL_CloseWindow();
		return false;
	}

	lmc_trace(LMC_LOG_VERBOSE, "[SDL2 GL]: Creating OpenGL context");

	/* Create context */
	sdl2_video->context = SDL_GL_CreateContext(sdl2_video->window);

	SDL_GL_MakeCurrent(sdl2_video->window, sdl2_video->context);

	if (!sdl2_video->context)
	{
		LMC_SetLastError(LMC_ERR_FAIL_WINDOW_INIT);
		lmc_trace(LMC_LOG_ERRORS, "[SDL2 GL]: Failed to create OpenGL context: %s", SDL_GetError());
		SDL2_GL_CloseWindow();
		return false;
	}

	/* Initialize OpenGL via glad. */
	if (system->cb_hw_render.context_type == RETRO_HW_CONTEXT_OPENGLES2) {
		if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress))
		{
			LMC_SetLastError(LMC_ERR_FAIL_WINDOW_INIT);
			lmc_trace(LMC_LOG_ERRORS, "[SDL2 GL]: Failed to initialize glad");
			SDL2_GL_CloseWindow();
			return false;
		}
	}
	else {
		if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
		{
			LMC_SetLastError(LMC_ERR_FAIL_WINDOW_INIT);
			lmc_trace(LMC_LOG_ERRORS, "[SDL2 GL]: Failed to initialize glad");
			SDL2_GL_CloseWindow();
			return false;
		}
	}

	lmc_trace(LMC_LOG_VERBOSE, "[SDL2 GL]: OpenGL version: %s", glGetString(GL_VERSION));
	lmc_trace(LMC_LOG_VERBOSE, "[SDL2 GL]: OpenGL shader language version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	/* Initialize video. */
	if (!legacy_machine->video->cb_init())
	{
		SDL2_GL_CloseWindow();
		return false;
	}

	/* Finalize window initialization and return result. */
	return SDL2_FinalizeWindow();
}
#endif

/* Destroy window delegate and free associated video and input data. */
static void SDL2_CloseWindow(void)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();

	if (legacy_machine->video)
	{
		legacy_machine->video->cb_deinit();
	}

	if (sdl2_video->window)
	{
		SDL_DestroyWindow(sdl2_video->window);
		sdl2_video->window = NULL;
	}
}

#if defined HAVE_OPENGL
/* Destroy OpenGL window delegate and free associated video and input data. */
static void SDL2_GL_CloseWindow(void)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();

	if (legacy_machine->video)
	{
		legacy_machine->video->cb_deinit();
	}

	if (sdl2_video->context)
	{
		SDL_GL_MakeCurrent(sdl2_video->window, sdl2_video->context);
		SDL_GL_DeleteContext(sdl2_video->context);

		sdl2_video->context = NULL;
	}

	if (sdl2_video->window)
	{
		SDL_DestroyWindow(sdl2_video->window);
		sdl2_video->window = NULL;
	}
}
#endif

/* Update window dimensions. */
static void SDL2_SetWindowSize(const struct retro_game_geometry* geometry)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	WindowInfo* window = GetWindowParameterInfo();
	ViewportInfo* viewport = GetViewportInfo();
	VideoInfo* video = GetVideoInfo();

	if (sdl2_video->window)
	{
		SDL_DisplayMode mode;

		/* Get desktop size and maximum window size. */
		SDL_GetDesktopDisplayMode(0, &mode);

		/* Calculate screen dimensions. */
		if (!video->fullscreen)
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
		legacy_machine->video->cb_set_viewport(viewport->x, 
			viewport->y, viewport->w, viewport->h);

		/* Update Window size. */
		SDL_SetWindowSize(sdl2_video->window, window->width, window->height);
	}
}

/* Sets window title. */
static void SDL2_SetWindowTitle(const char* title)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	WindowInfo* window = GetWindowParameterInfo();

	if (sdl2_video->window != NULL)
		SDL_SetWindowTitle(sdl2_video->window, title);
	if (window->title != NULL)
	{
		free(window->title);
		window->title = NULL;
	}
	if (title != NULL)
		window->title = strdup(title);
}

/* Process window and associated video and input events. */
static bool SDL2_ProcessEvents(void)
{
	WindowInfo* window = GetWindowParameterInfo();
	VideoInfo* video = GetVideoInfo();
	CRTFilter* filter = GetVideoFilter();
	SDL_Event evt;
	SDL_KeyboardEvent* keybevt;
	SDL_JoyButtonEvent* joybuttonevt;
	SDL_JoyHatEvent* joyhatevt;
	SDL_JoyAxisEvent* joyaxisevt;
	int input = 0;
	int c;

	if (!window->running)
		return false;

	/* Dispatch message queue. */
	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
		case SDL_QUIT:
			window->running = false;
			break;

		case SDL_WINDOWEVENT:
			switch (evt.window.event) {
			case SDL_WINDOWEVENT_CLOSE:
				window->running = false;
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
					window->running = false;
				else if (keybevt->keysym.sym == legacy_machine->input->joypad->state[LMC_PLAYER1].key_map[LMC_INPUT_CRT])
				{
					filter->cb_toggle_crt();
				}
				else if (keybevt->keysym.sym == SDLK_RETURN && keybevt->keysym.mod & KMOD_ALT)
				{
					legacy_machine->window->cb_deinit();
					/* Toggle fullscreen. */
					video->fullscreen ^= 1;
					legacy_machine->window->cb_init();
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

	/* Delete */
	if (!window->running)
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
	SDL2_SetWindowSize,
	SDL2_SetWindowTitle,
	0,
	0,
	false
};

#if defined HAVE_OPENGL
WindowDriver sdl2_gl_window_driver = {
	SDL2_GL_InitializeWindow,
	SDL2_ProcessEvents,
	SDL2_GL_CloseWindow,
	SDL2_SetWindowSize,
	SDL2_SetWindowTitle,
	0,
	0,
	false
};
#endif