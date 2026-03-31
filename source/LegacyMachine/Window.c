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
#if defined HAVE_SDL
#include <SDL.h>
#endif

#include <string/stdstring.h>

#include "LegacyMachine.h"
#include "MainEngine.h"
#include "SettingsManager.h"

/**************************************************************************************************
 * LegacyMachine Window Management Functions
 *************************************************************************************************/

/* Creates a window for rendering. */
bool LMC_CreateWindow(int flags)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	SettingsManager* settings = GetSettingsManagerContext();
	DefaultInfo* defaults = GetDefaultInfoContext();
	WindowFlags creation_flags;
	bool result;

	creation_flags.value = flags;

	/* Store default initialization values. */
	defaults->fullscreen = creation_flags.fullscreen;
	defaults->vsync = creation_flags.vsync;
	defaults->smooth = !creation_flags.nearest;

	/* Initialize configuration settings from file (create new Settings.cfg if necessary). */
	if (!InitializeSettings())
	{
		LMC_SetLastError(LMC_ERR_FAIL_CONFIG_INIT);
		return false;
	}

	/* Apply configurable settings. */
	legacy_machine->audio->output.latency = settings->uints.audio_latency;
	legacy_machine->audio->output.minimum_latency = settings->uints.audio_latency;
	legacy_machine->video->output.fullscreen = settings->bools.video_fullscreen;
	legacy_machine->video->output.vsync = settings->bools.video_vsync;
	legacy_machine->video->output.smooth = settings->bools.video_smooth;
	legacy_machine->video->output.type = (VideoOutput)GetOutputEnumFromSetting();

	/* Resolve override dimensions and aspect ratio for adjusted video. Otherwise, if outputting
	   raw video, window dimensions will be determined from supplied raw video geometry. */
	SetBaseOverrideDimensions(legacy_machine->video->output.type);

	/* Assign default window title if one hasn't been provided. */
	if (!legacy_machine->window->params.title)
		LMC_SetWindowTitle("LegacyMachine");

	/* Allow single instance. */
	if (legacy_machine->window->params.instances)
	{
		legacy_machine->window->params.instances++;
		return true;
	}

	/* Determine initial state of crt effect. Always disabled if raw output is active. */
	legacy_machine->video->crt_effect.enabled = (flags & LMC_CWF_NEAREST) == 0;

#if defined HAVE_MENU
	/* Apply configurable settings to menu. */
	legacy_machine->menu->av_info.geometry.aspect_ratio = settings->floats.video_aspect_ratio;

	/* Initialize video for frontend menu. */
	SetVideoPixelFormat(RETRO_PIXEL_FORMAT_XRGB8888);
	SetVideoGeometry(&legacy_machine->menu->av_info.geometry);
	legacy_machine->video->output.raw_frame.pitch = legacy_machine->menu->frame.pitch;

	result = InitializeWindow(WINDOW_DRIVER_FIRST);
#else
	result = true;
#endif

	if (result)
		legacy_machine->window->params.instances++;
	return result;
}

/* Deletes the window and all associated data (Video, Audio, and Input) previously created with
   LMC_CreateWindow(). */
void LMC_DeleteWindow(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();

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
	DeinitializeWindow();

#if defined HAVE_SDL
	/* Quit SDL. */
	SDL_Quit();
#endif
}

/* Processes all events related to a window created with LMC_CreateWindow(). */
bool LMC_ProcessWindow(void)
{
	return ProcessEvents();
}

/* Checks whether the window is active and running. */
bool LMC_IsWindowActive(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	if (!legacy_machine->window)
		return false;

	return legacy_machine->window->running;
}

/* Sets window title. */
void LMC_SetWindowTitle(const char* title)
{
	SetWindowTitle(title);
}

/* Gets the horizontal dimension of window after scaling. */
int LMC_GetWindowWidth(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	return legacy_machine->window->params.width;
}

/* Gets the vertical dimension of window after scaling. */
int LMC_GetWindowHeight(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	return legacy_machine->window->params.height;
}
