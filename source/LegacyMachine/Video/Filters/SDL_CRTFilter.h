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

/*
* Credits - SDL_CRTFilter adapted from Tilengine's crt filter.
*
* Tilengine - The 2D retro graphics engine with raster effects.
* Copyright (C) 2015-2022 Marc Palacios Domenech <mailto:megamarc@hotmail.com>
* All rights reserved
* */

#ifndef _SDL_CRT_FILTER_H
#define _SDL_CRT_FILTER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <SDL.h>

#include "../CRTEffect.h"

typedef struct _SDL_CRTHandler* SDL_CRTHandler;

/**************************************************************************************************
 * SDL CRTFilter Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

SDL_CRTHandler SDL_FilterCRTCreate(SDL_Renderer* renderer, SDL_Texture* texture, VideoOutput output, CRTMask type, int wnd_width, int wnd_height);
void SDL_FilterCRTDraw(SDL_CRTHandler crt, void* pixels, int pitch, SDL_Rect* dstrect);
void SDL_FilterCRTSetRenderTarget(SDL_CRTHandler crt, SDL_Texture* texture);
void SDL_FilterCRTIncreaseGlow(SDL_CRTHandler crt);
void SDL_FilterCRTDecreaseGlow(SDL_CRTHandler crt);
void SDL_FilterCRTDelete(SDL_CRTHandler crt);

RETRO_END_DECLS

#endif