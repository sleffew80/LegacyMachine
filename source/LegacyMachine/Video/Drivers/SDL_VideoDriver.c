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
#include <SDL.h>
#include <string/stdstring.h>

#include "../Common/SDL_Common.h"
#include "../VideoManager.h"
//#include "../../Presets.h"
#include "../../MainEngine.h"

/**************************************************************************************************
 * Prototypes
 *************************************************************************************************/

 /* Static prototypes */

static void SDL_FilterInitCRTEffect(void);
static void SDL_FilterToggleCRTEffect(void);
static void SDL_DriverCloseVideo(void);
static uintptr_t SDL_DriverGetFramebuffer(void);

/**************************************************************************************************
 * SDL Video Internal Functions
 *************************************************************************************************/


/**************************************************************************************************
 * SDL Video Functions
 *************************************************************************************************/

 /* Initialize video. */
static bool SDL_DriverInitializeVideo(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	SDL_VideoInfo* sdl_video = SDL_GetVideoInfoContext();
	RenderInfo* output = GetRenderInfo();
	CRTEffect* crt_effect = GetCRTEffect();
	SDL_RendererInfo renderer_info;
	int flags;

	lmc_trace(LMC_LOG_VERBOSE, "[Video] [SDL]: Initializing video...");

	/* Initialize software crt effect filter. */
	crt_effect->cb_toggle_crt = SDL_FilterToggleCRTEffect;
	crt_effect->type = GetCRTMaskEnumFromSetting();

	/* Set renderer flags. */
	flags = SDL_RENDERER_ACCELERATED;
	if (output->vsync)
		flags |= SDL_RENDERER_PRESENTVSYNC;

	/* List available renderers. */
	lmc_trace(LMC_LOG_VERBOSE, "[Video] [SDL]: Available renderers:");
	if (legacy_machine->log_level >= LMC_LOG_VERBOSE)
	{
		for (unsigned i = 0; i < SDL_GetNumRenderDrivers(); ++i)
		{
			if (SDL_GetRenderDriverInfo(i, &renderer_info) == 0)
				printf("\tRenderer #%i: '%s'\n", i, renderer_info.name);
		}
	}

	/* Create render context. */
	sdl_video->renderer = SDL_CreateRenderer(sdl_video->window, -1, flags);
	if (!sdl_video->renderer)
	{
		LMC_SetLastError(LMC_ERR_FAIL_VIDEO_INIT);
		lmc_trace(LMC_LOG_ERRORS, "[Video] [SDL]: Failed to initialize renderer: %s", SDL_GetError());
		return false;
	}

	/* Get current renderer driver name and information. */
	SDL_GetRendererInfo(sdl_video->renderer, &renderer_info);
	lmc_trace(LMC_LOG_VERBOSE, "[Video] [SDL]: Using '%s' render driver", renderer_info.name);

	/* Video driver is initialized. */
	legacy_machine->video->initialized = true;

	/* Video info and framebuffers still need to be initialized. */
	output->current = false;

	lmc_trace(LMC_LOG_VERBOSE, "[Video] [SDL]: Video initialized successfully");
	LMC_SetLastError(LMC_ERR_OK);

	return true;
}

/* Initialize an SDL texture to be used as a framebuffer. */
static bool SDL_DriverInitializeFramebuffer(void)
{
	SDL_VideoInfo* sdl_video = SDL_GetVideoInfoContext();
	RenderInfo* output = GetRenderInfo();
	CRTEffect* crt_effect = GetCRTEffect();

	if (sdl_video->texture != NULL)
		SDL_DestroyTexture(sdl_video->texture);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, crt_effect->enabled ? "1" : "0");

	sdl_video->texture = SDL_CreateTexture(sdl_video->renderer, 
		output->bpp32 ? SDL_PIXELFORMAT_ARGB8888 : sdl_video->format, 
		SDL_TEXTUREACCESS_STREAMING, 
		output->output_frame.width, 
		output->output_frame.height);

	if (!sdl_video->texture)
		return false;

	if (crt_effect->enabled)
	{
		SDL_FilterInitCRTEffect();
		SDL_FilterCRTSetRenderTarget(sdl_video->crt, sdl_video->texture);
	}

	return true;
}

/* Close video and free associated data. */
static void SDL_DriverCloseVideo(void)
{
	SDL_VideoInfo* sdl_video = SDL_GetVideoInfoContext();
	SDL_FilterCRTDelete(sdl_video->crt);
	sdl_video->crt = NULL;
	lmc_trace(LMC_LOG_VERBOSE, "[Video] [SDL]: Closing video...");

	DeinitializeFramebuffers();

	if (sdl_video->texture)
	{
		SDL_DestroyTexture(sdl_video->texture);
		sdl_video->texture = NULL;
	}

	if (sdl_video->renderer)
	{
		SDL_DestroyRenderer(sdl_video->renderer);
		sdl_video->renderer = NULL;
	}
}

/* Set viewport dimensions. */
static void SDL_DriverSetVideoViewport(ViewportInfo* viewport)
{
	SDL_VideoInfo* sdl_video = SDL_GetVideoInfoContext();

	sdl_video->viewport.x = viewport->x;
	sdl_video->viewport.y = viewport->y;
	sdl_video->viewport.w = viewport->w;
	sdl_video->viewport.h = viewport->h;
}

/* Set pixel format. */
static bool SDL_DriverSetVideoPixelFormat(unsigned format)
{
	SDL_VideoInfo* sdl_video = SDL_GetVideoInfoContext();

	/* Store SDL pixel format equivalencies. */
	switch (format)
	{
	case RETRO_PIXEL_FORMAT_0RGB1555:
		sdl_video->format = SDL_PIXELFORMAT_ARGB1555;
		break;
	case RETRO_PIXEL_FORMAT_XRGB8888:
		sdl_video->format = SDL_PIXELFORMAT_ARGB8888;
		break;
	case RETRO_PIXEL_FORMAT_RGB565:
		sdl_video->format = SDL_PIXELFORMAT_RGB565;
		break;
	default:
		lmc_core_log(RETRO_LOG_ERROR, "[Video] [SDL]: Unknown pixel type %u", format);
		return false;
	}

	return true;
}

/* Set video geometry. Some cores call this before window creation. */
static bool SDL_DriverSetVideoGeometry(const struct retro_game_geometry* geometry)
{
	/* Driver-specific geometry handling can go here if needed. */
	/* For SDL, most geometry handling is done by the manager and window. */
	/* This callback is optional and can be NULL if not needed. */
	return true;
}

/* Refreshes a single frame of video. */
static void SDL_DriverRefreshVideo(FrameInfo* frame)
{
	SDL_VideoInfo* sdl_video = SDL_GetVideoInfoContext();
	CRTEffect* crt_effect = GetCRTEffect();

	/* Update texture. */
	SDL_UpdateTexture(sdl_video->texture, NULL, frame->data, frame->pitch);
	/* Render with or without CRT. */
	if (crt_effect->enabled && sdl_video->crt != NULL)
	{
		SDL_FilterCRTDraw(sdl_video->crt, frame->data, frame->pitch, &sdl_video->viewport);
	}
	else
	{
		SDL_RenderClear(sdl_video->renderer);
		SDL_RenderCopy(sdl_video->renderer, sdl_video->texture, NULL, &sdl_video->viewport);
	}

	SDL_RenderPresent(sdl_video->renderer);
}

/* Returns a pointer to the framebuffer. */
static uintptr_t SDL_DriverGetFramebuffer(void)
{
	RenderInfo* output = GetRenderInfo();

	return (uintptr_t)output->raw_frame.data;
}

/**************************************************************************************************
 * SDL CRT Filter Functions
 *************************************************************************************************/

/* Initialize and build the CRT effect overlay for the current window dimensions. */
static void SDL_FilterInitCRTEffect(void)
{
	SDL_VideoInfo* sdl_video = SDL_GetVideoInfoContext();
	RenderInfo* output = GetRenderInfo();
	ViewportInfo* viewport = GetViewportInfo();
	CRTEffect* crt_effect = GetCRTEffect();

	if (sdl_video->crt != NULL)
		SDL_FilterCRTDelete(sdl_video->crt);

	if (!output->fullscreen)
		sdl_video->crt = SDL_FilterCRTCreate(sdl_video->renderer, 
			sdl_video->texture, 
			output->type, 
			crt_effect->type, 
			LMC_GetWindowWidth(), 
			LMC_GetWindowHeight());
	else
		sdl_video->crt = SDL_FilterCRTCreate(sdl_video->renderer, 
			sdl_video->texture, 
			output->type, 
			crt_effect->type, 
			viewport->w - viewport->x, 
			viewport->h - viewport->y);
}

/* Turns CRT effect on/off. */
static void SDL_FilterToggleCRTEffect(void)
{
	SDL_VideoInfo* sdl_video = SDL_GetVideoInfoContext();
	RenderInfo* output = GetRenderInfo();
	CRTEffect* crt_effect = GetCRTEffect();

	crt_effect->enabled = !crt_effect->enabled;

	output->current = false;
}

/**************************************************************************************************
 * SDL Video Driver
 *************************************************************************************************/

VideoDriver sdl_video_driver = {
	SDL_DriverInitializeVideo,			/* cb_init */
	SDL_DriverInitializeFramebuffer,		/* cb_init_fb */
	SDL_DriverRefreshVideo,				/* cb_refresh */
	SDL_DriverCloseVideo,				/* cb_deinit */
	SDL_DriverSetVideoViewport,			/* cb_set_viewport */
	SDL_DriverSetVideoPixelFormat,		/* cb_set_pixel_fmt */
	SDL_DriverSetVideoGeometry,			/* cb_set_geometry_fmt */
	SDL_DriverGetFramebuffer,			/* cb_get_framebuffer */
	NULL,							/* cb_get_hw_proc_address */
	VIDEO_DRIVER_SDL
};