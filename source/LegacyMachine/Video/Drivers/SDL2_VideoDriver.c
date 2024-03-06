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

#include "../VideoDriver.h"
#include "../../MainEngine.h"
#include "../../Logging.h"

#include "../Filters/SDL2_CRTFilter.h"

/**************************************************************************************************
 * SDL2 Video Variables
 *************************************************************************************************/

/* SDL video variables */

static SDL_Renderer* renderer = NULL;
static SDL_Texture* backbuffer = NULL;
static uint8_t* framebuffer = NULL;
static SDL_Rect viewport = { 0 };
static SDL2_CRTHandler crt;

/**************************************************************************************************
 * SDL2 Video Structures
 *************************************************************************************************/

/* Frame information. */
static FrameInfo frame_info = { 0 };

/**************************************************************************************************
 * Prototypes
 *************************************************************************************************/

/* Local prototypes */

static bool SDL2_InitializeVideo(void);
static void SDL2_CloseVideo(void);

/**************************************************************************************************
 * Local Video Functions
 *************************************************************************************************/

/* Get video frame info. */
static FrameInfo* GetVideoFrameInfo(void)
{
	return (FrameInfo*)legacy_machine->video->frame;
}

/* Get pointer to the crt video filter. */
static CRTFilter* GetVideoFilter(void)
{
	return (CRTFilter*)legacy_machine->video->filter;
}

/* Initialize the framebuffer. */
static void InitializeFramebuffer(int width, int height)
{

}

/* Initialize an SDL texture to be used as a backbuffer. */
static void InitializeBackbuffer(int width, int height)
{
	CRTFilter* crt_filter = GetVideoFilter();

	if (backbuffer != NULL)
		SDL_DestroyTexture(backbuffer);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, crt_filter->enabled ? "1" : "0");
	backbuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
}

/* Store video geometry locally. */
static void SetVideoGeometry(const struct retro_game_geometry* geometry)
{
	FrameInfo* frame = GetVideoFrameInfo();

	frame->aspect_ratio = geometry->aspect_ratio;
	frame->base_width = geometry->base_width;
	frame->base_height = geometry->base_height;
	if ((frame->max_width == 0) || (frame->max_height == 0))
	{
		frame->max_width = geometry->max_width;
		frame->max_height = geometry->max_height;
	}
}

/**************************************************************************************************
 * SDL2 Video Functions
 *************************************************************************************************/

/* Initialize video. */
static bool SDL2_InitializeVideo(void)
{
	SDL_Window* window;
	FrameInfo* frame = GetVideoFrameInfo();
	CRTFilter* crt_filter = GetVideoFilter();
	int flags;

	/* Get window. */
	window = SDL_GetWindowFromID(legacy_machine->window->params->identifier);

	/* Create render context. */
	flags = SDL_RENDERER_ACCELERATED;
	if (legacy_machine->window->params->flags & LMC_CWF_VSYNC)
		flags |= SDL_RENDERER_PRESENTVSYNC;
	renderer = SDL_CreateRenderer(window, -1, flags);
	if (!renderer)
	{
		LMC_SetLastError(LMC_ERR_FAIL_VIDEO_INIT);
		lmc_trace(LMC_LOG_ERRORS, "Failed to initialize renderer: %s", SDL_GetError());
		return false;
	}

	/* Get driver name and information here. */
	SDL_RendererInfo renderer_info;
	SDL_GetRendererInfo(renderer, &renderer_info);
	lmc_trace(LMC_LOG_VERBOSE, "Using SDL's '%s' render driver", renderer_info.name);

	/* Initialize backbuffer texture. */
	InitializeBackbuffer(frame->base_width, frame->base_height);

	crt = SDL2_CRTCreate(renderer, backbuffer, crt_filter->type, LMC_GetWindowWidth(), LMC_GetWindowHeight(), crt_filter->blur);

	/* Video is initialized. */
	legacy_machine->video->initialized = true;

	return true;
}

/* Close video and free associated data. */
static void SDL2_CloseVideo(void)
{
	SDL2_CRTDelete(crt);
	crt = NULL;

	if (backbuffer)
	{
		SDL_DestroyTexture(backbuffer);
		backbuffer = NULL;
	}

	if (renderer)
	{
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}
}

/* Set viewport dimensions. */
static void SDL2_SetVideoViewport(int x, int y, int width, int height)
{
	viewport.x = x;
	viewport.y = y;
	viewport.w = width;
	viewport.h = height;
}

/* Set pixel format. */
static bool SDL2_SetVideoPixelFormat(unsigned format) {
	FrameInfo* frame = GetVideoFrameInfo();

	switch (format)
	{
	case RETRO_PIXEL_FORMAT_0RGB1555:
		frame->pixel_format = SDL_PIXELFORMAT_ARGB1555;
		frame->bpp = sizeof(uint16_t);
		break;
	case RETRO_PIXEL_FORMAT_XRGB8888:
		frame->pixel_format = SDL_PIXELFORMAT_ARGB8888;
		frame->bpp = sizeof(uint32_t);
		break;
	case RETRO_PIXEL_FORMAT_RGB565:
		frame->pixel_format = SDL_PIXELFORMAT_RGB565;
		frame->bpp = sizeof(uint16_t);
		break;
	default:
		lmc_core_log(RETRO_LOG_ERROR, "Unknown pixel type %u", format);
		return false;
	}

	/* Output at 32bpp. */
	frame->out_bpp = sizeof(uint32_t);
	
	return true;
}

/* Set video geometry. Some cores call this before window creation. */
static bool SDL2_SetVideoGeometry(const struct retro_game_geometry* geometry)
{
	FrameInfo* frame = GetVideoFrameInfo();

	if (frame->base_width != geometry->base_width ||
		frame->base_height != geometry->base_height ||
		frame->aspect_ratio != geometry->aspect_ratio)
	{
		SetVideoGeometry(geometry);

		if (legacy_machine->window->initialized)
			legacy_machine->window->cb_resize_to_aspect(geometry);
	}
	return true;
}

/* Refreshes a single frame of video. */
static void SDL2_RefreshVideo(const void* data, unsigned width, unsigned height, unsigned pitch)
{
	FrameInfo* frame = GetVideoFrameInfo();
	CRTFilter* crt_filter = GetVideoFilter();

	/* Lock video texture for modifying. */
	SDL_LockTexture(backbuffer, NULL, (void**)&framebuffer, &frame->out_pitch);

	if (data != NULL)
	{
		/* Convert to 32bpp ARGB if necessary. */
		if (frame->pixel_format != SDL_PIXELFORMAT_ARGB8888)
		{
			/* Convert data into 32bpp framebuffer. */
			SDL_ConvertPixels(
				frame->base_width,	/* framebuffer width  */
				frame->base_height,	/* framebuffer height */
				frame->pixel_format,		/* source format      */
				data,						/* source             */
				pitch,						/* source pitch       */
				SDL_PIXELFORMAT_ARGB8888,	/* destination format */
				framebuffer,				/* destination        */
				frame->out_pitch			/* destination pitch  */
			);
		}
		else
		{
			/* Assign data directly to framebuffer. */
			framebuffer = data;
		}
	}
#ifdef HAVE_MENU
	else
	{
		TLN_SetRenderTarget(framebuffer, pitch);
	}
#endif
	if (crt_filter->enabled && crt != NULL)
	{
		/* Apply and render frame using crt filter. */
		SDL2_CRTDraw(crt, framebuffer, frame->out_pitch, &viewport);
	}
	else
	{
		/* End frame and apply to render target. */
		SDL_UnlockTexture(backbuffer);
		SDL_SetTextureBlendMode(backbuffer, SDL_BLENDMODE_NONE);
		SDL_RenderCopy(renderer, backbuffer, NULL, &viewport);
	}

	SDL_RenderPresent(renderer);
}

/* Returns a pointer to the framebuffer. */
static uintptr_t SDL2_GetFramebuffer(void)
{
	return (uintptr_t)framebuffer;
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
	FrameInfo* frame = GetVideoFrameInfo();
	CRTFilter* crt_filter = GetVideoFilter();

	if (crt != NULL)
		SDL2_CRTDelete(crt);

	crt_filter->type = (CRTType)type;
	crt_filter->blur = blur;
	crt_filter->enabled = true;
	InitializeBackbuffer(frame->base_width,frame->base_height);
	crt = SDL2_CRTCreate(renderer, backbuffer, crt_filter->type, frame->base_width, frame->base_height, crt_filter->blur);
}

/* Enables or disables RF emulation on CRT effect. */
void SDL2_EnableRFBlur(bool mode)
{
	SDL2_CRTSetBlur(crt, mode);
}

/* Turns CRT effect on/off. */
void SDL2_ToggleCRTEffect(void)
{
	FrameInfo* frame = GetVideoFrameInfo();
	CRTFilter* crt_filter = GetVideoFilter();

	crt_filter->enabled = !crt_filter->enabled;
	InitializeBackbuffer(frame->base_width, frame->base_height);
	SDL2_CRTSetRenderTarget(crt, backbuffer);
}

/* Disables the CRT post-processing effect. */
void SDL2_DisableCRTEffect(void)
{
	FrameInfo* frame = GetVideoFrameInfo();
	CRTFilter* crt_filter = GetVideoFilter();

	crt_filter->enabled = false;
	InitializeBackbuffer(frame->base_width, frame->base_height);
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
#if SDL_PATCHLEVEL >= 18
	SDL_GetTicks64,
#else 
	SDL_GetTicks,
#endif
	SDL2_GetFramebuffer,
	SDL2_GetProcAddress,
	RETRO_HW_CONTEXT_NONE,
	&frame_info,
	&sdl2_video_filter,
	false
};