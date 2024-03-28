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
#include "VideoDriver.h"
#include "../MainEngine.h"

/**************************************************************************************************
 * VideoDriver Context Array
 *************************************************************************************************/

const VideoDriver* video_drivers[] = {
#if defined(HAVE_SDL2) && defined(HAVE_OPENGL)
	&sdl2_gl_video_driver,
#endif
#if defined HAVE_SDL2
	&sdl2_video_driver,
#endif  
	NULL
};

/**************************************************************************************************
 * VideoDriver Initialization
 *************************************************************************************************/

/* Initialize the video driver. */
VideoDriver* InitializeVideoDriver(void)
{
	return (VideoDriver*)video_drivers[0];
}

/* Get video info. */
VideoInfo* GetVideoInfo(void)
{
	return &legacy_machine->video->info;
}

/* Get the crt video filter. */
CRTFilter* GetVideoFilter(void)
{
	return &legacy_machine->video->filter;
}

/* Store video geometry locally. */
void SetVideoGeometry(const struct retro_game_geometry* geometry)
{
	VideoInfo* info = GetVideoInfo();

	info->aspect_ratio = geometry->aspect_ratio;
	info->frame->width = geometry->base_width;
	info->frame->height = geometry->base_height;
	info->max_width = geometry->max_width;
	info->max_height = geometry->max_height;
}

/* Initialize system hardware render callback structure. */
void InitializeHardwareRenderCallback(void)
{
	legacy_machine->system->cb_hw_render.context_type = legacy_machine->video->hw_context;
	legacy_machine->system->cb_hw_render.version_major = legacy_machine->video->hw_api.version_major;
	legacy_machine->system->cb_hw_render.version_minor = legacy_machine->video->hw_api.version_minor;
	legacy_machine->system->cb_hw_render.get_current_framebuffer = legacy_machine->video->cb_get_framebuffer;
	if (legacy_machine->video->cb_get_hw_proc_address)
		legacy_machine->system->cb_hw_render.get_proc_address = legacy_machine->video->cb_get_hw_proc_address;
}

/* Set the hardware render callback being used. */
void SetHardwareRenderCallback(struct retro_hw_render_callback* hw_render)
{
	struct retro_hw_render_callback* hardware_render_callback = hw_render;
	hardware_render_callback->context_type = legacy_machine->video->hw_context;
	hardware_render_callback->version_major = legacy_machine->video->hw_api.version_major;
	hardware_render_callback->version_minor = legacy_machine->video->hw_api.version_minor;
	hardware_render_callback->get_current_framebuffer = legacy_machine->video->cb_get_framebuffer;
	if (legacy_machine->video->cb_get_hw_proc_address)
		hardware_render_callback->get_proc_address = legacy_machine->video->cb_get_hw_proc_address;
	legacy_machine->system->cb_hw_render = *hardware_render_callback;
}

/* Get hardware context type as human readable text. */
const char* GetHardwareContextTypeString(enum retro_hw_context_type hw_context)
{
	switch (hw_context) {
	case RETRO_HW_CONTEXT_NONE:
		return "RETRO_HW_CONTEXT_NONE";
		break;
	case RETRO_HW_CONTEXT_OPENGL:
		return "RETRO_HW_CONTEXT_OPENGL";
		break;
	case RETRO_HW_CONTEXT_OPENGLES2:
		return "RETRO_HW_CONTEXT_OPENGLES2";
		break;
	case RETRO_HW_CONTEXT_OPENGL_CORE:
		return "RETRO_HW_CONTEXT_OPENGL_CORE";
		break;
	case RETRO_HW_CONTEXT_OPENGLES3:
		return "RETRO_HW_CONTEXT_OPENGLES3";
		break;
	case RETRO_HW_CONTEXT_OPENGLES_VERSION:
		return "RETRO_HW_CONTEXT_OPENGLES_VERSION";
		break;
	case RETRO_HW_CONTEXT_VULKAN:
		return "RETRO_HW_CONTEXT_VULKAN";
		break;
	case RETRO_HW_CONTEXT_DIRECT3D:
		return "RETRO_HW_CONTEXT_DIRECT3D";
		break;
	default:
		return "";
	}
}

/* Creates an orthographic projection matrix. */
void Orthographic2D(float matrix[4][4], float left, float right, float bottom, float top)
{
	matrix[0][0] = 1; matrix[0][1] = 0; matrix[0][2] = 0; matrix[0][3] = 0;
	matrix[1][0] = 0; matrix[1][1] = 1; matrix[1][2] = 0; matrix[1][3] = 0;
	matrix[2][0] = 0; matrix[2][1] = 0; matrix[2][2] = 1; matrix[2][3] = 0;
	matrix[3][0] = 0; matrix[3][1] = 0; matrix[3][2] = 0; matrix[3][3] = 1;

	matrix[0][0] = 2.0f / (right - left);
	matrix[1][1] = 2.0f / (top - bottom);
	matrix[2][2] = -1.0f;
	matrix[3][0] = -(right + left) / (right - left);
	matrix[3][1] = -(top + bottom) / (top - bottom);
}
