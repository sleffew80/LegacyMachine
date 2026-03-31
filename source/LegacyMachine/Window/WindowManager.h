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

#ifndef _WINDOW_MANAGER_H
#define _WINDOW_MANAGER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"
#include "../Common/Common.h"

/**************************************************************************************************
 * Window Definitions
 *************************************************************************************************/

#define WINDOW_DRIVER_FIRST (WindowDriverID)0

/**************************************************************************************************
 * WindowDriver Enumeration
 *************************************************************************************************/

typedef enum
{
#if defined HAVE_SDL
	WINDOW_DRIVER_SDL,
#endif
	WINDOW_DRIVER_NONE,
	MAX_WINDOW_DRIVERS = WINDOW_DRIVER_NONE
}
WindowDriverID;

/**************************************************************************************************
 * WindowDriver Structure
 *************************************************************************************************/

typedef struct WindowDriver
{
	/* Initialize window driver. */
	bool			(*cb_init)(void);
	/* Process window and input events. */
	bool			(*cb_process)(void);
	/* Deinitialize window driver. */
	void			(*cb_deinit)(void);
	/* Resize window to new geometry. */
	void			(*cb_resize)(const struct retro_game_geometry*);
	/* Set the window title. */
	void			(*cb_set_title)(const char*);
	/* Set cursor visibility. */
	void			(*cb_show_cursor)(bool);
	/* Set the frame delay in milliseconds. */
	void			(*cb_set_delay)(uint32_t);
	/* Window driver ID. */
	WindowDriverID	id;
}
WindowDriver;

/**************************************************************************************************
 * WindowManager Structure
 *************************************************************************************************/

typedef struct WindowManager
{
	WindowInfo		params;				/* Window parameter info. */
	ViewportInfo	viewport;			/* Window viewport info. */
	bool			running;			/* Is window currently running. */
	bool			initialized;		/* Whether the window manager is initialized. */
}
WindowManager;

/**************************************************************************************************
 * WindowDriver Array
 *************************************************************************************************/

extern const WindowDriver* window_drivers[];

/**************************************************************************************************
 * WindowDriver Contexts
 *************************************************************************************************/

extern WindowDriver sdl_window_driver;

/**************************************************************************************************
 * Window Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

/* Window Management Prototypes */

WindowManager* GetWindowManagerContext(void);
WindowDriverID GetWindowDriverID(void);

bool InitializeWindow(WindowDriverID driver_id);
void DeinitializeWindow(void);

bool ProcessEvents(void);

WindowInfo* GetWindowParameterInfo(void);
ViewportInfo* GetViewportInfo(void);

void SetBaseOverrideDimensions(LMC_VideoOutput type);
void SetWindowGeometry(const struct retro_game_geometry*);
void SetWindowTitle(const char*);
void SetCursorVisibility(bool show);

void CalculateWindowedDimensions(double aspect, int width, int height,
	int disp_width, int disp_height);
void CalculateFullscreenDimensions(double aspect, int width, int height,
	int disp_width, int disp_height);

RETRO_END_DECLS

#endif