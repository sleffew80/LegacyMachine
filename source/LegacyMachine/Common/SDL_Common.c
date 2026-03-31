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
#include "SDL_Common.h"

/**************************************************************************************************
 * SDL Common Contexts
 *************************************************************************************************/

static SDL_VideoInfo sdl_video_info = { 0 };

/**************************************************************************************************
 * SDL Common Functions
 *************************************************************************************************/

/* Get the current SDL video info context. */
SDL_VideoInfo* SDL_GetVideoInfoContext(void)
{
	return &sdl_video_info;
}

