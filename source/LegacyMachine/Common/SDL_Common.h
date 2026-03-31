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

#ifndef _SDL__COMMON_H
#define _SDL__COMMON_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <SDL.h>

#include "../Video/Filters/SDL_CRTFilter.h"

/**************************************************************************************************
 * SDL Common Types/Structures
 *************************************************************************************************/

typedef struct SDL_AudioInfo
{
	SDL_AudioDeviceID   device;
	uint8_t			    channels;
	int				    frequency;
	uint16_t            samples;
	bool                paused;
}
SDL_AudioInfo;

/* Structure for storing additional video info related specifically to SDL video rendering. */
typedef struct SDL_VideoInfo
{
	SDL_Window*         window;
	SDL_Renderer*       renderer;
	SDL_Texture*        texture;
	SDL_Rect            viewport;
	SDL_PixelFormatEnum format;
	SDL_CRTHandler     crt;
}
SDL_VideoInfo;

/**************************************************************************************************
 * SDL Common Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

SDL_VideoInfo* SDL_GetVideoInfoContext(void);

RETRO_END_DECLS

#endif
