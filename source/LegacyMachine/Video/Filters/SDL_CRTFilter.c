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

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "SDL_CRTFilter.h"

/**************************************************************************************************
 * SDL CRTHandler Structure
 *************************************************************************************************/

struct _SDL_CRTHandler
{
	SDL_Renderer*	renderer;	/* SDL renderer used for drawing. */
	SDL_Texture*	texture;	/* Source framebuffer texture. */
	SDL_Texture*	overlay;	/* Composited CRT overlay texture. */
	uint32_t		scanlines;	/* Number of scanlines for the effect. */
	Size2D			size_fb;	/* Framebuffer dimensions. */
	uint8_t			glow;		/* Current glow intensity value. */
};

/**************************************************************************************************
 * Prototypes
 *************************************************************************************************/

/* Static prototypes */

static SDL_Texture* CreateTiledTexture(SDL_Renderer* renderer,
	int width, int height,
	int tile_width, int tile_height,
	const uint8_t* tile_data);

/**************************************************************************************************
 * SDL CRTFilter Functions
 *************************************************************************************************/

/* Create CRT effect. */
SDL_CRTHandler SDL_FilterCRTCreate(SDL_Renderer* renderer, SDL_Texture* texture, VideoOutput output,
	CRTMask type, int wnd_width, int wnd_height)
{
	SDL_CRTHandler crt_handler = (SDL_CRTHandler)calloc(1, sizeof(struct _SDL_CRTHandler));
	if (crt_handler == NULL)
		return NULL;

	crt_handler->renderer = renderer;
	crt_handler->texture = texture;

	/* Get framebuffer size. */
	Uint32 format = 0;
	int access = 0;

	SDL_QueryTexture(texture, 
		&format, 
		&access, 
		&crt_handler->size_fb.width, 
		&crt_handler->size_fb.height);

	/* Calculate scanline count. */
	if (output == OUTPUT_RAW)
		crt_handler->scanlines = crt_handler->size_fb.height * 2;
	else
		crt_handler->scanlines = 480;

	/* Build composed overlay with RGB mask + scanlines. */
	Pattern* pattern = &patterns[type];
	crt_handler->glow = pattern->glow;

	SDL_Texture* tex_mask = CreateTiledTexture(renderer,
		wnd_width, wnd_height,
		pattern->width, pattern->height,
		pattern->mask);

	SDL_Texture* tex_scan = CreateTiledTexture(renderer,
		crt_handler->size_fb.width,
		crt_handler->scanlines,
		1,
		2,
		pattern_scanline);

	SDL_SetTextureBlendMode(tex_scan, SDL_BLENDMODE_MOD);

	crt_handler->overlay = SDL_CreateTexture(renderer, 
		SDL_PIXELFORMAT_ARGB8888, 
		SDL_TEXTUREACCESS_TARGET, 
		wnd_width, wnd_height);

	SDL_SetRenderTarget(renderer, crt_handler->overlay);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, tex_mask, NULL, NULL);

	if (type != CRT_SLOT)
		SDL_RenderCopy(renderer, tex_scan, NULL, NULL);

	SDL_SetRenderTarget(renderer, NULL);
	SDL_SetTextureBlendMode(crt_handler->overlay, SDL_BLENDMODE_MOD);
	SDL_DestroyTexture(tex_scan);
	SDL_DestroyTexture(tex_mask);

	return crt_handler;
}

/* Draws effect, gets locked texture data. */
void SDL_FilterCRTDraw(SDL_CRTHandler crt_handler, void* pixels, int pitch, SDL_Rect* dstrect)
{
	/* Base image. */
	SDL_SetTextureBlendMode(crt_handler->texture, SDL_BLENDMODE_NONE);
	SDL_RenderCopy(crt_handler->renderer, crt_handler->texture, NULL, dstrect);

	/* RGB + scanline overlay. */
	SDL_RenderCopy(crt_handler->renderer, crt_handler->overlay, NULL, dstrect);

	/* Glow overlay. */
	if (crt_handler->glow != 0)
	{
		SDL_SetTextureBlendMode(crt_handler->texture, SDL_BLENDMODE_ADD);

		SDL_SetTextureColorMod(crt_handler->texture, 
			crt_handler->glow, 
			crt_handler->glow, 
			crt_handler->glow);

		SDL_RenderCopy(crt_handler->renderer, crt_handler->texture, NULL, dstrect);
	}
}

/* Set the source texture for an existing CRT handler. */
void SDL_FilterCRTSetRenderTarget(SDL_CRTHandler crt_handler, SDL_Texture* texture)
{
	if (crt_handler != NULL && texture != NULL)
		crt_handler->texture = texture;
}

/* Increase the glow intensity of the CRT effect by one step. */
void SDL_FilterCRTIncreaseGlow(SDL_CRTHandler crt_handler)
{
	if (crt_handler != NULL && crt_handler->glow < 255)
		crt_handler->glow += 1;
}

/* Decrease the glow intensity of the CRT effect by one step. */
void SDL_FilterCRTDecreaseGlow(SDL_CRTHandler crt_handler)
{
	if (crt_handler != NULL && crt_handler->glow > 0)
		crt_handler->glow -= 1;
}

/* Destroy and free a CRT handler and its associated overlay texture. */
void SDL_FilterCRTDelete(SDL_CRTHandler crt_handler)
{
	if (crt_handler != NULL)
		SDL_DestroyTexture(crt_handler->overlay);

	free(crt_handler);
}

/* Create an SDL texture filled by tiling a small pixel pattern across the given dimensions. */
static SDL_Texture* CreateTiledTexture(SDL_Renderer* renderer, int width, int height,
	int tile_width, int tile_height, const uint8_t* tile_data)
{
	SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);

	const int tile_pitch = tile_width * 4;
	SDL_Rect dstrect = { 0, 0, tile_width, tile_height };
	for (dstrect.y = 0; dstrect.y <= height - tile_height; dstrect.y += tile_height)
	{
		uint8_t* dstptr = (uint8_t*)surface->pixels + dstrect.y * surface->pitch;
		for (dstrect.x = 0; dstrect.x <= width - tile_width; dstrect.x += tile_width)
		{
			Blit(tile_data, dstptr, tile_pitch, tile_height, surface->pitch);
			dstptr += tile_pitch;
		}
		if (dstrect.x < width)
			Blit(tile_data, dstptr, (width - dstrect.x) * 4, tile_height, surface->pitch);
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	return texture;
}