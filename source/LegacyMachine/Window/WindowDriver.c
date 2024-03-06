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

/**************************************************************************************************
 * WindowDriver Context Array
 *************************************************************************************************/

const WindowDriver* window_drivers[] = {
#ifdef HAVE_SDL2
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