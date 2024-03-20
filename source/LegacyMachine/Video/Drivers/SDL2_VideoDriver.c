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
#include <SDL.h>

#include "../Common/SDL2_Common.h"
#include "../VideoDriver.h"
#include "../../MainEngine.h"
#include "../../Logging.h"

/**************************************************************************************************
 * SDL2 Video Variables
 *************************************************************************************************/

/* SDL video variables */

int out_pitch; /* Store output pitch value for pixel format conversions. */

/**************************************************************************************************
 * Prototypes
 *************************************************************************************************/

/* Local prototypes */

static void SDL2_CloseVideo(void);

/**************************************************************************************************
 * Local Video Functions
 *************************************************************************************************/

/* Initialize an SDL texture to be used as a framebuffer. */
static void InitializeFramebuffer(int width, int height)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	VideoInfo* video = GetVideoInfo();
	CRTFilter* filter = GetVideoFilter();

	if (sdl2_video->framebuffer != NULL)
		SDL_DestroyTexture(sdl2_video->framebuffer);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filter->enabled ? "1" : "0");
	sdl2_video->framebuffer = SDL_CreateTexture(sdl2_video->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
}

/**************************************************************************************************
 * SDL2 Video Functions
 *************************************************************************************************/

/* Initialize video. */
static bool SDL2_InitializeVideo(void)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	VideoInfo* video = GetVideoInfo();
	CRTFilter* filter = GetVideoFilter();
	int flags;

	/* Get window. */
	//window = SDL_GetWindowFromID(legacy_machine->window->params->identifier);

	/* Create render context. */
	flags = SDL_RENDERER_ACCELERATED;
	if (video->vsync)
		flags |= SDL_RENDERER_PRESENTVSYNC;
	sdl2_video->renderer = SDL_CreateRenderer(sdl2_video->window, -1, flags);
	if (!sdl2_video->renderer)
	{
		LMC_SetLastError(LMC_ERR_FAIL_VIDEO_INIT);
		lmc_trace(LMC_LOG_ERRORS, "Failed to initialize renderer: %s", SDL_GetError());
		return false;
	}

	/* Get driver name and information here. */
	SDL_RendererInfo renderer_info;
	SDL_GetRendererInfo(sdl2_video->renderer, &renderer_info);
	lmc_trace(LMC_LOG_VERBOSE, "Using SDL's '%s' render driver", renderer_info.name);

	/* Initialize framebuffer texture. */
	InitializeFramebuffer(video->frame->width, video->frame->height);

	sdl2_video->crt = SDL2_CRTCreate(sdl2_video->renderer, sdl2_video->framebuffer, filter->type, LMC_GetWindowWidth(), LMC_GetWindowHeight(), filter->blur);

	/* Video is initialized. */
	legacy_machine->video->initialized = true;

	return true;
}

/* Close video and free associated data. */
static void SDL2_CloseVideo(void)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	SDL2_CRTDelete(sdl2_video->crt);
	sdl2_video->crt = NULL;

	if (sdl2_video->framebuffer)
	{
		SDL_DestroyTexture(sdl2_video->framebuffer);
		sdl2_video->framebuffer = NULL;
	}

	if (sdl2_video->renderer)
	{
		SDL_DestroyRenderer(sdl2_video->renderer);
		sdl2_video->renderer = NULL;
	}
}

/* Set viewport dimensions. */
static void SDL2_SetVideoViewport(int x, int y, int width, int height)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();

	sdl2_video->viewport.x = x;
	sdl2_video->viewport.y = y;
	sdl2_video->viewport.w = width;
	sdl2_video->viewport.h = height;
}

/* Set pixel format. */
static bool SDL2_SetVideoPixelFormat(unsigned format) 
{
	VideoInfo* video = GetVideoInfo();

	switch (format)
	{
	case RETRO_PIXEL_FORMAT_0RGB1555:
		video->frame->format = SDL_PIXELFORMAT_ARGB1555;
		video->frame->depth = sizeof(uint16_t);
		break;
	case RETRO_PIXEL_FORMAT_XRGB8888:
		video->frame->format = SDL_PIXELFORMAT_ARGB8888;
		video->frame->depth = sizeof(uint32_t);
		break;
	case RETRO_PIXEL_FORMAT_RGB565:
		video->frame->format = SDL_PIXELFORMAT_RGB565;
		video->frame->depth = sizeof(uint16_t);
		break;
	default:
		lmc_core_log(RETRO_LOG_ERROR, "Unknown pixel type %u", format);
		return false;
	}
	
	return true;
}

/* Set video geometry. Some cores call this before window creation. */
static bool SDL2_SetVideoGeometry(const struct retro_game_geometry* geometry)
{
	VideoInfo* video = GetVideoInfo();

	if (video->frame->width != geometry->base_width ||
		video->frame->height != geometry->base_height ||
		video->aspect_ratio != geometry->aspect_ratio)
	{
		SetVideoGeometry(geometry);

		if (legacy_machine->window->initialized)
			legacy_machine->window->cb_resize(geometry);
	}
	return true;
}

/* Refreshes a single frame of video. */
static void SDL2_RefreshVideo(const void* data, unsigned width, unsigned height, unsigned pitch)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	VideoInfo* video = GetVideoInfo();
	CRTFilter* filter = GetVideoFilter();

	/* Lock video texture for modifying. */
	SDL_LockTexture(sdl2_video->framebuffer, NULL, (void**)&video->frame->data, &out_pitch);

	if (data != NULL)
	{
		/* Convert to 32bpp ARGB if necessary. */
		if (video->frame->format != SDL_PIXELFORMAT_ARGB8888)
		{
			/* Convert data into 32bpp framebuffer. */
			SDL_ConvertPixels(
				video->frame->width,		/* framebuffer width  */
				video->frame->height,		/* framebuffer height */
				video->frame->format,		/* source format      */
				data,						/* source             */
				pitch,						/* source pitch       */
				SDL_PIXELFORMAT_ARGB8888,	/* destination format */
				video->frame->data,			/* destination        */
				out_pitch					/* destination pitch  */
			);
		}
		else
		{
			/* Assign data directly to framebuffer. */
			video->frame->data = data;
		}
	}
#ifdef HAVE_MENU
	else
	{
		TLN_SetRenderTarget(video->frame->data, pitch);
	}
#endif
	if (filter->enabled && sdl2_video->crt != NULL)
	{
		/* Apply and render frame using crt filter. */
		SDL2_CRTDraw(sdl2_video->crt, video->frame->data, out_pitch, &sdl2_video->viewport);
	}
	else
	{
		/* End frame and apply to render target. */
		SDL_UnlockTexture(video->frame->data);
		SDL_SetTextureBlendMode(video->frame->data, SDL_BLENDMODE_NONE);
		SDL_RenderCopy(sdl2_video->renderer, video->frame->data, NULL, &sdl2_video->viewport);
	}

	SDL_RenderPresent(sdl2_video->renderer);
}

/* Returns a pointer to the framebuffer. */
static uintptr_t SDL2_GetFramebuffer(void)
{
	VideoInfo* video = GetVideoInfo();

	return (uintptr_t)video->frame->data;
}

/* Gets a hardware procedure by name. */
static retro_hw_get_proc_address_t SDL2_GetProcAddress(const char* name)
{
	return (retro_hw_get_proc_address_t)SDL_GL_GetProcAddress(name);
}

/**************************************************************************************************
 * SDL2 CRT Filter Functions
 *************************************************************************************************/

/* Enables CRT simulation post-processing effect to give true retro appeareance.  */
void SDL2_ConfigCRTEffect(LMC_CRT type, bool blur)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	VideoInfo* video = GetVideoInfo();
	CRTFilter* filter = GetVideoFilter();

	if (sdl2_video->crt != NULL)
		SDL2_CRTDelete(sdl2_video->crt);

	filter->type = (CRTType)type;
	filter->blur = blur;
	filter->enabled = true;
	InitializeFramebuffer(video->frame->width, video->frame->height);
	sdl2_video->crt = SDL2_CRTCreate(sdl2_video->renderer, sdl2_video->framebuffer, filter->type, video->frame->width, video->frame->height, filter->blur);
}

/* Enables or disables RF emulation on CRT effect. */
void SDL2_EnableRFBlur(bool mode)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();

	SDL2_CRTSetBlur(sdl2_video->crt, mode);
}

/* Turns CRT effect on/off. */
void SDL2_ToggleCRTEffect(void)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	VideoInfo* video = GetVideoInfo();
	CRTFilter* filter = GetVideoFilter();

	filter->enabled = !filter->enabled;
	InitializeFramebuffer(video->frame->width, video->frame->height);
	SDL2_CRTSetRenderTarget(sdl2_video->crt, sdl2_video->framebuffer);
}

/* Disables the CRT post-processing effect. */
void SDL2_DisableCRTEffect(void)
{
	VideoInfo* video = GetVideoInfo();
	CRTFilter* filter = GetVideoFilter();

	filter->enabled = false;
	InitializeFramebuffer(video->frame->width, video->frame->height);
}

/**************************************************************************************************
 * SDL2 Video Driver
 *************************************************************************************************/

CRTFilter sdl2_video_filter = {
	SDL2_ConfigCRTEffect,
	SDL2_EnableRFBlur,
	SDL2_ToggleCRTEffect,
	SDL2_DisableCRTEffect,
	CRT_SHADOW, 
	false, 
	true
};

VideoDriver sdl2_video_driver = {
	SDL2_InitializeVideo,
	SDL2_RefreshVideo,
	SDL2_CloseVideo,
	SDL2_SetVideoViewport,
	SDL2_SetVideoPixelFormat,
	SDL2_SetVideoGeometry,
	SDL_Delay,
#if (SDL_MAJOR_VERSION == 2) && (SDL_MINOR_VERSION >= 18)
	SDL_GetTicks64,
#else 
	SDL_GetTicks,
#endif
	SDL2_GetFramebuffer,
	SDL2_GetProcAddress,
	RETRO_HW_CONTEXT_NONE,
	0,
	0,
	false
};