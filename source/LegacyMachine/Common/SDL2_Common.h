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

#ifndef _SDL2_COMMON_H_
#define _SDL2_COMMON_H_

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <SDL.h>

#include "../Video/Filters/SDL2_CRTFilter.h"

/**************************************************************************************************
 * SDL2 Common Types/Structures
 *************************************************************************************************/

/* Structure for storing additional video info related specifically to SDL2 video rendering. */
typedef struct SDL2_VideoInfo
{
    SDL_Window*     window;
#if defined HAVE_OPENGL
    SDL_GLContext   context;
#endif
    SDL_Renderer*   renderer;
    SDL_Texture*    texture;
    SDL_Rect        viewport;
    SDL2_CRTHandler crt;
}
SDL2_VideoInfo;

/**************************************************************************************************
 * SDL2 Common Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

SDL2_VideoInfo* SDL2_GetVideoInfoContext(void);

RETRO_END_DECLS

#endif
