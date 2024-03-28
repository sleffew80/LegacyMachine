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
#include "WindowDriver.h"
#include "../MainEngine.h"

/**************************************************************************************************
 * WindowDriver Context Array
 *************************************************************************************************/

const WindowDriver* window_drivers[] = {
#if defined(HAVE_SDL2) && defined(HAVE_OPENGL)
	&sdl2_gl_window_driver,
#endif
#if defined HAVE_SDL2
	&sdl2_window_driver,
#endif  
	NULL
};

/**************************************************************************************************
 * WindowDriver Initialization
 *************************************************************************************************/

/* Initialize the window driver. */
WindowDriver* InitializeWindowDriver(void)
{
	return (WindowDriver*)window_drivers[0];
}

/* Get window info. */
WindowInfo* GetWindowParameterInfo(void)
{
	return &legacy_machine->window->params;
}

/* Get window viewport info. */
ViewportInfo* GetViewportInfo(void)
{
	return &legacy_machine->window->viewport;
}

/* Resize dimensions to aspect ratio. */
void ResizeToAspect(double aspect, int src_width, int src_height,
	int* dst_width, int* dst_height, int disp_width, int disp_height)
{
	*dst_width = src_width;
	*dst_height = src_height;

	if (aspect <= 0)
		aspect = (double)src_width / src_height;

	if (!legacy_machine->video->info.fullscreen)
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
		ResizeToAspect(aspect, width * 1, height * 1, &window_info->width, &window_info->height, disp_width, disp_height);
	}

	/* If a scale isn't provided, calculate an optimal one based on client screen dimensions. */
	if (!window_info->factor)
	{
		window_info->factor = 1;
		while (width * (window_info->factor + 1) < disp_width && height * (window_info->factor + 1) < disp_height && window_info->factor < 3)
			window_info->factor++;
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

	if ((window_info->override_width > 0) && (window_info->override_height > 0))
	{
		/* Resize dimensions to aspect ratio based on overrides. */
		ResizeToAspect(window_info->override_aspect, disp_width, disp_height, &window_info->width, &window_info->height, disp_width, disp_height);
		if (!window_info->factor)
			window_info->factor = window_info->height / window_info->height;
	}
	else
	{
		/* Resize dimensions to aspect ratio. */
		ResizeToAspect(aspect, disp_width, disp_height, &window_info->width, &window_info->height, disp_width, disp_height);
		if (!window_info->factor)
			window_info->factor = window_info->height / height;
	}

	viewport_info->x = (disp_width - window_info->width) >> 1;
	viewport_info->y = (disp_height - window_info->height) >> 1;
	viewport_info->w = window_info->width;
	viewport_info->h = window_info->height;
}