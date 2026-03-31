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

#include "WindowManager.h"
#include "../MainEngine.h"

/**************************************************************************************************
 * WindowManager Context
 *************************************************************************************************/

static WindowManager window_manager = { 0 };

/**************************************************************************************************
 * WindowDriver Context Array
 *************************************************************************************************/

const WindowDriver* window_drivers[] = {
#if defined HAVE_SDL
	&sdl_window_driver,
#endif
	NULL
};

/**************************************************************************************************
 * WindowDriver Context
 *************************************************************************************************/

static WindowDriver* window_driver = NULL;

/**************************************************************************************************
 * WindowDriver Functions
 *************************************************************************************************/

/* Verify that WindowDriverID enum values match their corresponding indices in the window_drivers
   array. */
static void AssertWindowDriverEnumValues(void)
{
	for (unsigned i = 0; i < MAX_WINDOW_DRIVERS; ++i)
	{
		retro_assert((WindowDriverID)i == window_drivers[i]->id);
	}
}

/* Initialize the window driver. */
static WindowDriver* InitializeWindowDriver(WindowDriverID driver_id)
{
#if defined _DEBUG
	/* Verify WindowDriverID values when debugging. */
	AssertWindowDriverEnumValues();
#endif
	return (WindowDriver*)window_drivers[driver_id];
}

/* Get the current window driver context. */
static WindowDriver* GetWindowDriverContext(void)
{
	return window_driver;
}

/**************************************************************************************************
 * WindowManager Functions
 *************************************************************************************************/

/* Resize dimensions to aspect ratio. */
static void ResizeToAspect(double aspect, int src_width, int src_height,
	int* dst_width, int* dst_height, int disp_width, int disp_height)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	*dst_width = src_width;
	*dst_height = src_height;

	if (aspect <= 0)
		aspect = (double)src_width / src_height;

	if (!legacy_machine->video->output.fullscreen)
	{
		if ((float)src_width / src_height < 1)
			*dst_width = *dst_height * aspect;
		else
			*dst_height = *dst_width / aspect;
	}
	else
	{
		*dst_width = disp_width;
		*dst_height = *dst_width / aspect;
		if (*dst_height > disp_height)
		{
			*dst_height = disp_height;
			*dst_width = *dst_height * aspect;
		}
	}
}

/* Calculate an optimal scale factor based on client screen dimensions. */
static void CalculateWindowedScaleFactor(int width, int height, int disp_width, int disp_height)
{
	WindowInfo* window_info = GetWindowParameterInfo();

	window_info->factor = 1;
	while ((width * (window_info->factor + 1) < disp_width) && 
		(height * (window_info->factor + 1) < disp_height))
		window_info->factor++;
}

/* Get the current window manager context. */
WindowManager* GetWindowManagerContext(void)
{
	return &window_manager;
}

/* Get the current window driver ID. */
WindowDriverID GetWindowDriverID(void)
{
	WindowDriver* driver = GetWindowDriverContext();
	return driver ? driver->id : WINDOW_DRIVER_NONE;
}

/* Initialize the window manager with the specified window driver. */
bool InitializeWindow(WindowDriverID driver_id)
{
	WindowManager* window = GetWindowManagerContext();
	WindowDriver* driver = InitializeWindowDriver(driver_id);

	if (!driver || !driver->cb_init)
	{
		return false;
	}

	window_driver = driver;
	window->initialized = driver->cb_init();

	/* Window is running. */
	window->running = true;

	return window->initialized;
}

/* Deinitialize the window manager and its driver. */
void DeinitializeWindow(void)
{
	WindowManager* window = GetWindowManagerContext();
	WindowDriver* driver = GetWindowDriverContext();

	DeinitializeInput();
	DeinitializeVideo();

	if (driver && driver->cb_deinit)
	{
		driver->cb_deinit();
	}

	window_driver = NULL;

	memset(window, 0, sizeof(WindowManager));
}

/* Process window and associated video and input events.*/
bool ProcessEvents(void)
{
	WindowDriver* driver = GetWindowDriverContext();

	retro_assert(driver);

	return driver->cb_process();
}

/* Get window info. */
WindowInfo* GetWindowParameterInfo(void)
{
	WindowManager* window = GetWindowManagerContext();

	return &window->params;
}

/* Get window viewport info. */
ViewportInfo* GetViewportInfo(void)
{
	WindowManager* window = GetWindowManagerContext();

	return &window->viewport;
}

/* Resolve override dimensions and aspect ratio for adjusted video based on output type. */
void SetBaseOverrideDimensions(LMC_VideoOutput type)
{
	WindowManager* window = GetWindowManagerContext();

	if (type != LMC_VIDEO_OUTPUT_RAW)
	{
		switch (type)
		{
			case LMC_VIDEO_OUTPUT_ADJUSTED_3_2:
			{
				window->params.override_width = ADJ_3_2_HRES;
				window->params.override_height = ADJ_VRES;
				window->params.override_aspect = (float)ADJ_3_2_HRES / (float)ADJ_VRES;
				break;
			}
			case LMC_VIDEO_OUTPUT_ADJUSTED_4_3:
			{
				window->params.override_width = ADJ_4_3_HRES;
				window->params.override_height = ADJ_VRES;
				window->params.override_aspect = (float)ADJ_4_3_HRES / (float)ADJ_VRES;
				break;
			}
			case LMC_VIDEO_OUTPUT_ADJUSTED_5_4:
			{
				window->params.override_width = ADJ_5_4_HRES;
				window->params.override_height = ADJ_VRES;
				window->params.override_aspect = (float)ADJ_5_4_HRES / (float)ADJ_VRES;
				break;
			}
			case LMC_VIDEO_OUTPUT_ADJUSTED_16_9:
			{
				window->params.override_width = ADJ_16_9_HRES;
				window->params.override_height = ADJ_VRES;
				window->params.override_aspect = (float)ADJ_16_9_HRES / (float)ADJ_VRES;
				break;
			}
			default:
			{
				window->params.override_width = 0;
				window->params.override_height = 0;
				window->params.override_aspect = 0.0f;
				break;
			}
		}
	}
}

/* Set the window's dimensions via retro_game_geometry structure. */
void SetWindowGeometry(const struct retro_game_geometry* geometry)
{
	WindowDriver* driver = GetWindowDriverContext();

	if (driver && driver->cb_resize)
	{
		driver->cb_resize(geometry);
	}
}

/* Set the window's title. */
void SetWindowTitle(const char* title)
{
	WindowDriver* driver = GetWindowDriverContext();

	if (driver && driver->cb_set_title)
	{
		driver->cb_set_title(title);
	}
}

/* Set the window's cursor visibility. */
void SetCursorVisibility(bool show)
{
	WindowDriver* driver = GetWindowDriverContext();
	if (driver && driver->cb_show_cursor)
	{
		driver->cb_show_cursor(show);
	}
}

/* Calculate windowed dimensions. */
void CalculateWindowedDimensions(double aspect, int width, int height,
	int disp_width, int disp_height)
{
	WindowInfo* window_info = GetWindowParameterInfo();
	ViewportInfo* viewport_info = GetViewportInfo();

	if ((window_info->override_width > 0) && (window_info->override_height > 0))
	{
		/* Size window according to dimension overrides */
		window_info->width = window_info->override_width;
		window_info->height = window_info->override_height;
	}
	else
	{
		/* Resize dimensions to aspect ratio. */
		ResizeToAspect(aspect, width * 1, height * 1, 
			&window_info->width, &window_info->height, 
			disp_width, disp_height);
	}

	/* If a scale isn't provided, calculate an optimal one based on client screen dimensions. */
	if (!window_info->factor)
	{
		CalculateWindowedScaleFactor(window_info->width, window_info->height, 
			disp_width, disp_height);
	}

	window_info->width *= window_info->factor;
	window_info->height *= window_info->factor;

	viewport_info->x = 0;
	viewport_info->y = 0;
	viewport_info->w = window_info->width;
	viewport_info->h = window_info->height;
}

/* Calculate fullscreen dimensions. */
void CalculateFullscreenDimensions(double aspect, int width, int height,
	int disp_width, int disp_height)
{
	WindowInfo* window_info = GetWindowParameterInfo();
	ViewportInfo* viewport_info = GetViewportInfo();
	int out_frame_width = 0;
	int out_frame_height = 0;
	window_info->width = disp_width;
	window_info->height = disp_height;

	if ((window_info->override_width > 0) && (window_info->override_height > 0))
	{
		/* Resize dimensions to aspect ratio based on overrides. */
		ResizeToAspect(window_info->override_aspect, width, height, 
			&out_frame_width, &out_frame_height, 
			disp_width, disp_height);
	}
	else
	{
		/* Resize dimensions to aspect ratio. */
		ResizeToAspect(aspect, width, height, 
			&out_frame_width, &out_frame_height, 
			disp_width, disp_height);
	}

	viewport_info->x = (disp_width - out_frame_width) >> 1;
	viewport_info->y = (disp_height - out_frame_height) >> 1;
	viewport_info->w = out_frame_width;
	viewport_info->h = out_frame_height;
}