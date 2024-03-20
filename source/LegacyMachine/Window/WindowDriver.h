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

#ifndef _WINDOW_DRIVER_H
#define _WINDOW_DRIVER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"
#include "../Common/Common.h"

/**************************************************************************************************
 * WindowDriver Structure
 *************************************************************************************************/

typedef struct WindowDriver
{
	bool			(*cb_init)(void);
	bool			(*cb_process)(void);
	void			(*cb_deinit)(void);
	void			(*cb_resize)(const struct retro_game_geometry*);
	void			(*cb_set_title)(const char*);
	WindowInfo		params;
	ViewportInfo	viewport;
	bool			initialized;
}
WindowDriver;

/**************************************************************************************************
 * WindowDriver Array
 *************************************************************************************************/

extern const WindowDriver* window_drivers[];

/**************************************************************************************************
 * WindowDriver Contexts
 *************************************************************************************************/

extern WindowDriver sdl2_window_driver;

/**************************************************************************************************
 * WindowDriver Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

WindowDriver* InitializeWindowDriver(void);
WindowInfo* GetWindowParameterInfo(void);
ViewportInfo* GetViewportInfo(void);

void ResizeToAspect(double aspect, int src_width, int src_height,
	int* dst_width, int* dst_height, int disp_width, int disp_height);
void CalculateWindowedDimensions(double aspect, int width, int height,
	int disp_width, int disp_height);
void CalculateFullscreenDimensions(double aspect, int width, int height,
	int disp_width, int disp_height);

RETRO_END_DECLS

#endif