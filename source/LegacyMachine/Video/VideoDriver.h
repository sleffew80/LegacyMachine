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

#ifndef _VIDEO_DRIVER_H
#define _VIDEO_DRIVER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"
#include "CRTFilter.h"

/**************************************************************************************************
 * FrameInfo Structure
 *************************************************************************************************/

typedef struct FrameInfo
{
	float    aspect_ratio;
	int32_t  base_width;
	int32_t  base_height;
	int32_t  max_width;
	int32_t  max_height;
	uint32_t pitch;
	uint32_t pixel_format;
	uint32_t bpp;
	uint32_t out_pitch;
	uint32_t out_bpp;
}
FrameInfo;

/**************************************************************************************************
 * VideoDriver Structure
 *************************************************************************************************/

typedef struct VideoDriver
{
	bool							(*cb_init)(void);
	void							(*cb_refresh)(const void*, unsigned, unsigned, unsigned);
	void							(*cb_deinit)(void);
	void							(*cb_set_viewport)(int, int, int, int);
	bool							(*cb_set_pixel_fmt)(unsigned);
	bool							(*cb_set_geometry_fmt)(const struct retro_game_geometry*);
	void							(*cb_set_delay)(uint32_t);
	uint64_t						(*cb_get_ticks)(void);
	uintptr_t						(*cb_get_framebuffer)(void);
	retro_hw_get_proc_address_t		(*cb_get_hw_proc_address)(void);
	enum retro_hw_context_type		hw_context;
	FrameInfo*						frame;
	CRTFilter*						filter;
	bool							initialized;
}
VideoDriver;

/**************************************************************************************************
 * VideoDriver Array
 *************************************************************************************************/

extern const VideoDriver* video_drivers[];

/**************************************************************************************************
 * VideoDriver Contexts
 *************************************************************************************************/

extern VideoDriver sdl2_video_driver;

/**************************************************************************************************
 * VideoDriver Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

VideoDriver* InitializeVideoDriver(void);

RETRO_END_DECLS

#endif