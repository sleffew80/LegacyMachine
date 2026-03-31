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

#ifndef _VIDEO_MANAGER_H
#define _VIDEO_MANAGER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"
#include "CRTEffect.h"
#include "../Common/Common.h"
#include "../Logging.h"

/**************************************************************************************************
 * Video Definitions
 *************************************************************************************************/

#define VIDEO_DRIVER_FIRST (VideoDriverID)0

/**************************************************************************************************
 * VideoDriver Enumeration
 *************************************************************************************************/

typedef enum
{
#if defined HAVE_SDL
	VIDEO_DRIVER_SDL,
#endif
	VIDEO_DRIVER_NONE,
	MAX_VIDEO_DRIVERS = VIDEO_DRIVER_NONE
}
VideoDriverID;

/**************************************************************************************************
 * VideoDriver Structure
 *************************************************************************************************/

typedef struct VideoDriver
{
	/* Initialize video driver. */
	bool			(*cb_init)(void);
	/* Initialize framebuffers. */
	bool			(*cb_init_fb)(void);
	/* Refresh and render a video frame. */
	void			(*cb_refresh)(FrameInfo*);
	/* Deinitialize video driver. */
	void			(*cb_deinit)(void);
	/* Set the rendering viewport. */
	void			(*cb_set_viewport)(ViewportInfo*);
	/* Set the pixel format. */
	bool			(*cb_set_pixel_fmt)(unsigned);
	/* Set the video geometry. */
	bool			(*cb_set_geometry_fmt)(const struct retro_game_geometry*);
	/* Get a pointer to the framebuffer. */
	uintptr_t		(*cb_get_framebuffer)(void);
	/* Get hardware procedure address. */
	retro_hw_get_proc_address_t		(*cb_get_hw_proc_address)(void);
	/* Video driver ID. */
	VideoDriverID	id;
}
VideoDriver;

/**************************************************************************************************
 * VideoManager Structure
 *************************************************************************************************/

typedef struct VideoManager
{
	RenderInfo						output;			/* Video render output information. */
	CRTEffect						crt_effect;		/* CRT visual effect settings. */
	struct
	{
		unsigned version_major;	/* Hardware API major version. */
		unsigned version_minor;	/* Hardware API minor version. */
	}
	hw_api;											/* Hardware API version info. */
	enum retro_hw_context_type		hw_context;		/* Hardware rendering context type. */
	bool							initialized;	/* Whether the video manager is initialized. */
}
VideoManager;

/**************************************************************************************************
 * VideoDriver Array
 *************************************************************************************************/

extern const VideoDriver* video_drivers[];

/**************************************************************************************************
 * VideoDriver Contexts
 *************************************************************************************************/

extern VideoDriver sdl_video_driver;

/**************************************************************************************************
 * Video Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

/* Video Management Prototypes */

VideoManager* GetVideoManagerContext(void);
VideoDriverID GetVideoDriverID(void);

bool InitializeVideo(VideoDriverID driver_id, ViewportInfo* viewport);
void DeinitializeVideo(void);

bool InitializeFramebuffers(void);
void DeinitializeFramebuffers(void);

void RefreshVideo(const void* data, unsigned width, unsigned height, unsigned pitch);

RenderInfo* GetRenderInfo(void);
CRTEffect* GetCRTEffect(void);

void SetVideoViewport(ViewportInfo* viewport);
bool SetVideoPixelFormat(unsigned format);
void SetVideoGeometry(const struct retro_game_geometry* geometry);
void InitializeHardwareRenderCallback(void);
const char* GetPixelFormatTypeString(unsigned format);
const char* GetHardwareContextTypeString(enum retro_hw_context_type);
void Orthographic2D(float matrix[4][4], float left, float right, float bottom, float top);

RETRO_END_DECLS

#endif
