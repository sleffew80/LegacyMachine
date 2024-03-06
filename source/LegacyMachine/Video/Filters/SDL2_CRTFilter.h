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

/*
* Credits - SDL2_CRTFilter adapted from Tilengine's crt filter.
*
* Tilengine - The 2D retro graphics engine with raster effects.
* Copyright (C) 2015-2022 Marc Palacios Domenech <mailto:megamarc@hotmail.com>
* All rights reserved
* */

#ifndef _SDL2_CRT_FILTER_H
#define _SDL2_CRT_FILTER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <retro_library.h>

#include <SDL.h>

#include "../CRTFilter.h"

typedef struct _SDL2_CRTHandler* SDL2_CRTHandler;

/**************************************************************************************************
 * SDL2 CRTFilter Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

SDL2_CRTHandler SDL2_CRTCreate(SDL_Renderer* renderer, SDL_Texture* framebuffer, CRTType type, int wnd_width, int wnd_height, bool blur);
void SDL2_CRTDraw(SDL2_CRTHandler crt, void* pixels, int pitch, SDL_Rect* dstrect);
void SDL2_CRTSetRenderTarget(SDL2_CRTHandler crt, SDL_Texture* framebuffer);
void SDL2_CRTIncreaseGlow(SDL2_CRTHandler crt);
void SDL2_CRTDecreaseGlow(SDL2_CRTHandler crt);
void SDL2_CRTSetBlur(SDL2_CRTHandler crt, bool blur);
void SDL2_CRTDelete(SDL2_CRTHandler crt);

RETRO_END_DECLS

#endif