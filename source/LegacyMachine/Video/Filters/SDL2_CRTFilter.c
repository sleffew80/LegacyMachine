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

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "SDL2_CRTFilter.h"

/**************************************************************************************************
 * SDL2 CRTHandler Sturcture
 *************************************************************************************************/

struct _SDL2_CRTHandler
{
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Texture* overlay;
	Size2D size_fb;
	uint8_t glow;
	bool blur;
};

/**************************************************************************************************
 * Prototypes
 *************************************************************************************************/

/* Local prototypes */

static SDL_Texture* CreateTiledTexture(SDL_Renderer* renderer, int width, int height, int tile_width, int tile_height, const uint8_t* tile_data);

/**************************************************************************************************
 * SDL2 CRTFilter Functions
 *************************************************************************************************/

/* Create CRT effect. */
SDL2_CRTHandler SDL2_CRTCreate(SDL_Renderer* renderer, SDL_Texture* texture, CRTType type, int wnd_width, int wnd_height, bool blur)
{
	SDL2_CRTHandler crt = (SDL2_CRTHandler)calloc(1, sizeof(struct _SDL2_CRTHandler));
	if (crt == NULL)
		return NULL;

	crt->renderer = renderer;
	crt->texture = texture;
	crt->blur = blur;

	/* Get framebuffer size. */
	Uint32 format = 0;
	int access = 0;
	SDL_QueryTexture(texture, &format, &access, &crt->size_fb.width, &crt->size_fb.height);

	/* Build composed overlay with RGB mask + scanlines. */
	Pattern* pattern = &patterns[type];
	crt->glow = pattern->glow;
	SDL_Texture* tex_mask = CreateTiledTexture(renderer, wnd_width, wnd_height, pattern->width, pattern->height, pattern->mask);
	SDL_Texture* tex_scan = CreateTiledTexture(renderer, crt->size_fb.width, crt->size_fb.height * 2, 1, 2, pattern_scanline);
	SDL_SetTextureBlendMode(tex_scan, SDL_BLENDMODE_MOD);

	crt->overlay = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, wnd_width, wnd_height);
	SDL_SetRenderTarget(renderer, crt->overlay);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, tex_mask, NULL, NULL);
	if (type != CRT_SLOT)
		SDL_RenderCopy(renderer, tex_scan, NULL, NULL);
	SDL_SetRenderTarget(renderer, NULL);
	SDL_SetTextureBlendMode(crt->overlay, SDL_BLENDMODE_MOD);
	SDL_DestroyTexture(tex_scan);
	SDL_DestroyTexture(tex_mask);

	return crt;
}

/* Draws effect, gets locked texture data. */
void SDL2_CRTDraw(SDL2_CRTHandler crt, void* pixels, int pitch, SDL_Rect* dstrect)
{
	/* RF blur. */
	if (crt->blur)
		RFBlur((uint8_t*)pixels, crt->size_fb.width, crt->size_fb.height, pitch);

	/* Base image. */
	SDL_SetTextureBlendMode(crt->texture, SDL_BLENDMODE_NONE);
	SDL_RenderCopy(crt->renderer, crt->texture, NULL, dstrect);

	/* RGB + scanline overlay. */
	SDL_RenderCopy(crt->renderer, crt->overlay, NULL, dstrect);

	/* Glow overlay. */
	if (crt->glow != 0)
	{
		SDL_SetTextureBlendMode(crt->texture, SDL_BLENDMODE_ADD);
		SDL_SetTextureColorMod(crt->texture, crt->glow, crt->glow, crt->glow);
		SDL_RenderCopy(crt->renderer, crt->texture, NULL, dstrect);
	}
}

void SDL2_CRTSetRenderTarget(SDL2_CRTHandler crt, SDL_Texture* texture)
{
	if (crt != NULL && texture != NULL)
		crt->texture = texture;
}

void SDL2_CRTIncreaseGlow(SDL2_CRTHandler crt)
{
	if (crt != NULL && crt->glow < 255)
		crt->glow += 1;
}

void SDL2_CRTDecreaseGlow(SDL2_CRTHandler crt)
{
	if (crt != NULL && crt->glow > 0)
		crt->glow -= 1;
}

void SDL2_CRTSetBlur(SDL2_CRTHandler crt, bool blur)
{
	if (crt != NULL)
		crt->blur = blur;
}

void SDL2_CRTDelete(SDL2_CRTHandler crt)
{
	if (crt != NULL)
		SDL_DestroyTexture(crt->overlay);

	free(crt);
}

static void Blit(const uint8_t* srcptr, uint8_t* dstptr, int srcpitch, int lines, int dstpitch)
{
	int y;
	for (y = 0; y < lines; y += 1)
	{
		memcpy(dstptr, srcptr, srcpitch);
		srcptr += srcpitch;
		dstptr += dstpitch;
	}
}

static SDL_Texture* CreateTiledTexture(SDL_Renderer* renderer, int width, int height, int tile_width, int tile_height, const uint8_t* tile_data)
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