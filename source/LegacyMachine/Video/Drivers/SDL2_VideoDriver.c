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

#if defined HAVE_OPENGL
#include "../../Common/GL_Common.h"
#include "../Shared/GL_Shared.h"
#endif

/**************************************************************************************************
 * SDL2 Video Variables
 *************************************************************************************************/

/* SDL video variables */

int out_pitch; /* Store output pitch value for pixel format conversions. */

/*****************************************************************************
/* OpenGL Shaders
/****************************************************************************/

#if defined HAVE_OPENGL
static const char* gl_vertex_shader_source =
"#version 150\n"
"in vec2 i_position;\n"
"in vec2 i_coordinate;\n"
"out vec2 o_coordinate;\n"
"uniform mat4 u_mvp;\n"
"void main() {\n"
"o_coordinate = i_coordinate;\n"
"gl_Position = vec4(i_position, 0.0, 1.0) * u_mvp;\n"
"}";

static const char* gl_fragment_shader_source =
"#version 150\n"
"in vec2 o_coordinate;\n"
"uniform sampler2D u_texture;\n"
"void main() {\n"
"gl_FragColor = texture2D(u_texture, o_coordinate);\n"
"}";
#endif

/**************************************************************************************************
 * Prototypes
 *************************************************************************************************/

/* Local prototypes */

static void SDL2_ConfigCRTEffect(LMC_CRT type, bool blur);
static void SDL2_EnableRFBlur(bool mode);
static void SDL2_ToggleCRTEffect(void);
static void SDL2_DisableCRTEffect(void);
static void SDL2_CloseVideo(void);
#if defined HAVE_OPENGL
static void SDL2_GL_SetVideoViewport(int x, int y, int width, int height);
static void SDL2_GL_CloseVideo(void);
static uintptr_t SDL2_GetFramebuffer(void);
#endif

/**************************************************************************************************
 * Local Video Functions
 *************************************************************************************************/

/* Initialize an SDL texture to be used as a framebuffer. */
static void InitializeFramebuffer(int width, int height)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	VideoInfo* video = GetVideoInfo();
	CRTFilter* filter = GetVideoFilter();

	if (sdl2_video->texture != NULL)
		SDL_DestroyTexture(sdl2_video->texture);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filter->enabled ? "1" : "0");
	sdl2_video->texture = SDL_CreateTexture(sdl2_video->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
}

#ifdef HAVE_OPENGL
/* Initialize OpenGL rendering and a texture to be used as a framebuffer. */
static void SDL2_GL_InitializeFramebuffer(int width, int height)
{
	GL_VideoInfo* gl_video = GL_GetVideoInfoContext();
	SystemManager* system = GetSystemManagerContext();

	glGenFramebuffers(1, &gl_video->framebuffer_id);
	glBindFramebuffer(GL_FRAMEBUFFER, gl_video->framebuffer_id);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl_video->texture, 0);

	if (system->cb_hw_render.depth && system->cb_hw_render.stencil) {
		glGenRenderbuffers(1, &gl_video->renderbuffer_id);
		glBindRenderbuffer(GL_RENDERBUFFER, gl_video->renderbuffer_id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gl_video->renderbuffer_id);
	}
	else if (system->cb_hw_render.depth) {
		glGenRenderbuffers(1, &gl_video->renderbuffer_id);
		glBindRenderbuffer(GL_RENDERBUFFER, gl_video->renderbuffer_id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gl_video->renderbuffer_id);
	}

	if (system->cb_hw_render.depth || system->cb_hw_render.stencil)
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	retro_assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
#endif

/**************************************************************************************************
 * SDL2 Video Functions
 *************************************************************************************************/

/* Initialize video. */
static bool SDL2_InitializeVideo(void)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	VideoInfo* video = GetVideoInfo();
	CRTFilter* filter = GetVideoFilter();
	SDL_RendererInfo renderer_info;
	int flags;

	/* Initialize software crt filter. */
	filter->cb_config_crt = SDL2_ConfigCRTEffect;
	filter->cb_enable_rf = SDL2_EnableRFBlur;
	filter->cb_toggle_crt = SDL2_ToggleCRTEffect;
	filter->cb_deinit_crt = SDL2_DisableCRTEffect;
	filter->type = CRT_SLOT;
	filter->blur = false;

	/* Set renderer flags. */
	flags = SDL_RENDERER_ACCELERATED;
	if (video->vsync)
		flags |= SDL_RENDERER_PRESENTVSYNC;

	/* List available renderers. */
	lmc_trace(LMC_LOG_VERBOSE, "[SDL2]: Available renderers:");
	if (legacy_machine->log_level >= LMC_LOG_VERBOSE)
	{
		for (unsigned i = 0; i < SDL_GetNumRenderDrivers(); ++i)
		{
			if (SDL_GetRenderDriverInfo(i, &renderer_info) == 0)
				printf("\tRenderer #%i: '%s'\n", i, renderer_info.name);
		}
	}

	/* Create render context. */
	sdl2_video->renderer = SDL_CreateRenderer(sdl2_video->window, -1, flags);
	if (!sdl2_video->renderer)
	{
		LMC_SetLastError(LMC_ERR_FAIL_VIDEO_INIT);
		lmc_trace(LMC_LOG_ERRORS, "[SDL2]: Failed to initialize renderer: %s", SDL_GetError());
		return false;
	}

	/* Get current renderer driver name and information. */	
	SDL_GetRendererInfo(sdl2_video->renderer, &renderer_info);
	lmc_trace(LMC_LOG_VERBOSE, "[SDL2]: Using '%s' render driver", renderer_info.name);

	/* Initialize framebuffer texture. */
	InitializeFramebuffer(video->frame->width, video->frame->height);

	/* Initialize crt filter. */
	sdl2_video->crt = SDL2_CRTCreate(sdl2_video->renderer, sdl2_video->texture, filter->type, LMC_GetWindowWidth(), LMC_GetWindowHeight(), filter->blur);

	/* Video is initialized. */
	legacy_machine->video->initialized = true;

	return true;
}

#ifdef HAVE_OPENGL
/* Initialize OpenGL video. */
static bool SDL2_GL_InitializeVideo(void)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	GL_VideoInfo* gl_video = GL_GetVideoInfoContext();
	VideoInfo* video = GetVideoInfo();
	ViewportInfo* viewport = GetViewportInfo();
	SystemManager* system = GetSystemManagerContext();

	/* Initialize and compile default shaders from source. */
	GL_InitializeShaders(gl_vertex_shader_source, gl_fragment_shader_source);

	/* Set swap interval to vertical sync or immediate and assign to window. */
	if (video->vsync)
		SDL_GL_SetSwapInterval(1);
	else
		SDL_GL_SetSwapInterval(0);

	SDL_GL_SwapWindow(sdl2_video->window);

	/* Set video viewport dimensions. */
	SDL2_GL_SetVideoViewport(viewport->x, viewport->y, viewport->w, viewport->h);

	/* Create a video texture. */
	if (gl_video->texture)
		glDeleteTextures(1, &gl_video->texture);

	gl_video->texture = 0;

	glGenTextures(1, &gl_video->texture);

	if (!gl_video->texture)
	{
		lmc_trace(LMC_LOG_ERRORS, "[SDL2 GL]: Failed to create a video texture");
		return false;
	}

	/* Create and bind texture and then configure for framebuffer initialization. */
	glBindTexture(GL_TEXTURE_2D, gl_video->texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, video->max_width, video->max_height, 0,
		video->frame->type, video->frame->format, NULL);

	/* Unbind texture. */
	glBindTexture(GL_TEXTURE_2D, 0);

	/* Initialize framebuffer and vertex data. */
	SDL2_GL_InitializeFramebuffer(video->max_width, video->max_height);

	GL_RefreshVertexData();

	system->cb_hw_render.context_reset();

	return true;
}
#endif

/* Close video and free associated data. */
static void SDL2_CloseVideo(void)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	SDL2_CRTDelete(sdl2_video->crt);
	sdl2_video->crt = NULL;

	if (sdl2_video->texture)
	{
		SDL_DestroyTexture(sdl2_video->texture);
		sdl2_video->texture = NULL;
	}

	if (sdl2_video->renderer)
	{
		SDL_DestroyRenderer(sdl2_video->renderer);
		sdl2_video->renderer = NULL;
	}
}

#ifdef HAVE_OPENGL
/* Close OpenGL video and free associated data. */
static void SDL2_GL_CloseVideo(void)
{
	GL_VideoInfo* gl_video = GL_GetVideoInfoContext();

	if (gl_video->framebuffer_id)
		glDeleteFramebuffers(1, &gl_video->framebuffer_id);

	if (gl_video->texture)
		glDeleteTextures(1, &gl_video->texture);

	if (gl_video->shader->vertex_array)
		glDeleteVertexArrays(1, &gl_video->shader->vertex_array);

	if (gl_video->shader->vertex_buffer)
		glDeleteBuffers(1, &gl_video->shader->vertex_buffer);

	if (gl_video->shader->program)
		glDeleteProgram(gl_video->shader->program);

	if (gl_video->shader)
		free(gl_video->shader);

	gl_video->framebuffer_id = 0;
	gl_video->texture = 0;
}
#endif

/* Set viewport dimensions. */
static void SDL2_SetVideoViewport(int x, int y, int width, int height)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();

	sdl2_video->viewport.x = x;
	sdl2_video->viewport.y = y;
	sdl2_video->viewport.w = width;
	sdl2_video->viewport.h = height;
}

#ifdef HAVE_OPENGL
/* Set viewport dimensions. */
static void SDL2_GL_SetVideoViewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}
#endif

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
		lmc_core_log(RETRO_LOG_ERROR, "[SDL2]: Unknown pixel type %u", format);
		return false;
	}
	
	return true;
}

#ifdef HAVE_OPENGL
/* Set pixel format. */
static bool SDL2_GL_SetVideoPixelFormat(unsigned format)
{
	VideoInfo* video = GetVideoInfo();

	switch (format)
	{
	case RETRO_PIXEL_FORMAT_0RGB1555:
		video->frame->format = GL_UNSIGNED_SHORT_5_5_5_1;
		video->frame->type = GL_BGRA;
		video->frame->depth = sizeof(uint16_t);
		break;
	case RETRO_PIXEL_FORMAT_XRGB8888:
		video->frame->format = GL_UNSIGNED_INT_8_8_8_8_REV;
		video->frame->type = GL_BGRA;
		video->frame->depth = sizeof(uint32_t);
		break;
	case RETRO_PIXEL_FORMAT_RGB565:
		video->frame->format = GL_UNSIGNED_SHORT_5_6_5;
		video->frame->type = GL_RGB;
		video->frame->depth = sizeof(uint16_t);
		break;
	default:
		lmc_core_log(RETRO_LOG_ERROR, "[SDL2 GL]: Unknown pixel type %u", format);
		return false;
	}

	return true;
}
#endif

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

	/* Update pitch if necessary. */
	if (pitch != video->frame->pitch)
		video->frame->pitch = pitch;

	if (data)
	{
		/* Convert to 32bpp ARGB if necessary. */
		if (video->frame->format != SDL_PIXELFORMAT_ARGB8888)
		{
			/* Convert frame data to 32bpp depth and store in frame info. */
			SDL_ConvertPixels(
				/* framebuffer width  */
				video->frame->width,
				/* framebuffer height */
				video->frame->height,
				/* source format      */
				video->frame->format,
				/* source             */
				data,
				/* source pitch       */
				pitch,
				/* destination format */
				SDL_PIXELFORMAT_ARGB8888,
				/* destination        */
				video->frame->data,
				/* destination pitch  */
				out_pitch
			);
		}
		else
		{
			/* Store pointer directly to data in frame info. */
			video->frame->data = data;
			out_pitch = pitch;
		}
		/* Clear renderer and apply pixel data to framebuffer texture. */
		SDL_RenderClear(sdl2_video->renderer);
		SDL_UpdateTexture(sdl2_video->texture, NULL, video->frame->data, out_pitch);
	}
	if (filter->enabled && sdl2_video->crt != NULL)
	{
		/* Apply framebuffer texture to render target with added crt filter effect. */
		SDL2_CRTDraw(sdl2_video->crt, video->frame->data, out_pitch, &sdl2_video->viewport);
	}
	else
	{
		/* Apply framebuffer texture to render target. */
		SDL_SetTextureBlendMode(sdl2_video->texture, SDL_BLENDMODE_NONE);
		SDL_RenderCopy(sdl2_video->renderer, sdl2_video->texture, NULL, &sdl2_video->viewport);
	}
	/* Render to window. */
	SDL_RenderPresent(sdl2_video->renderer);
}

#if defined HAVE_OPENGL
static void SDL2_GL_RefreshVideo(const void* data, unsigned width, unsigned height, unsigned pitch)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	GL_VideoInfo* gl_video = GL_GetVideoInfoContext();
	VideoInfo* video = GetVideoInfo();
	ViewportInfo* viewport = GetViewportInfo();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, gl_video->texture);

	/* Update pitch if necessary. */
	if (pitch != video->frame->pitch)
		video->frame->pitch = pitch;

	if (data)
	{	
		/* Store pointer to data in frame info. */
		video->frame->data = data;
	}

	/* Verify frame data and apply to texture. */
	if (video->frame->data && video->frame->data != RETRO_HW_FRAME_BUFFER_VALID) 
	{
		glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLuint)video->frame->pitch / (GLuint)video->frame->depth);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
			(GLuint)video->frame->type, (GLuint)video->frame->format, video->frame->data);
	}

	/* Set viewport. */
	SDL2_GL_SetVideoViewport(viewport->x, viewport->y, viewport->w, viewport->h);

	/* Clear the color buffer. */
	glClear(GL_COLOR_BUFFER_BIT);

	/* Bind shader program. */
	glUseProgram(gl_video->shader->program);

	/* Select active texture unit. */
	glActiveTexture(GL_TEXTURE0);

	/* Bind Texture to target. */
	glBindTexture(GL_TEXTURE_2D, gl_video->texture);

	/* Bind vertex array to render from. */
	glBindVertexArray(gl_video->shader->vertex_array);
	
	/* Render primitives from array data */
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	/* Unbind vertex array. */
	glBindVertexArray(0);

	/* Unbind shader program. */
	glUseProgram(0);

	/* Swap buffers. */
	SDL_GL_SwapWindow(sdl2_video->window);
}
#endif

/* Returns a pointer to the framebuffer. */
static uintptr_t SDL2_GetFramebuffer(void)
{
	VideoInfo* video = GetVideoInfo();

	return (uintptr_t)video->frame->data;
}

#ifdef HAVE_OPENGL
/* Gets a hardware procedure by name. */
static retro_hw_get_proc_address_t SDL2_GL_GetProcAddress(const char* name)
{
	return (retro_hw_get_proc_address_t)SDL_GL_GetProcAddress(name);
}
#endif

/**************************************************************************************************
 * SDL2 CRT Filter Functions
 *************************************************************************************************/

/* Enables CRT simulation post-processing effect to give true retro appeareance.  */
static void SDL2_ConfigCRTEffect(LMC_CRT type, bool blur)
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
	sdl2_video->crt = SDL2_CRTCreate(sdl2_video->renderer, sdl2_video->texture, filter->type, video->frame->width, video->frame->height, filter->blur);
}

/* Enables or disables RF emulation on CRT effect. */
static void SDL2_EnableRFBlur(bool mode)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();

	SDL2_CRTSetBlur(sdl2_video->crt, mode);
}

/* Turns CRT effect on/off. */
static void SDL2_ToggleCRTEffect(void)
{
	SDL2_VideoInfo* sdl2_video = SDL2_GetVideoInfoContext();
	VideoInfo* video = GetVideoInfo();
	CRTFilter* filter = GetVideoFilter();

	filter->enabled = !filter->enabled;
	InitializeFramebuffer(video->frame->width, video->frame->height);
	SDL2_CRTSetRenderTarget(sdl2_video->crt, sdl2_video->texture);
}

/* Disables the CRT post-processing effect. */
static void SDL2_DisableCRTEffect(void)
{
	VideoInfo* video = GetVideoInfo();
	CRTFilter* filter = GetVideoFilter();

	filter->enabled = false;
	InitializeFramebuffer(video->frame->width, video->frame->height);
}

/**************************************************************************************************
 * SDL2 Video Driver
 *************************************************************************************************/

VideoDriver sdl2_video_driver = {
	SDL2_InitializeVideo,
	SDL2_RefreshVideo,
	SDL2_CloseVideo,
	SDL2_SetVideoViewport,
	SDL2_SetVideoPixelFormat,
	SDL2_SetVideoGeometry,
	SDL_Delay,
#if SDL_VERSION_ATLEAST(2,18,0)
	SDL_GetTicks64,
#else 
	SDL_GetTicks,
#endif
	SDL2_GetFramebuffer,
	NULL,
	RETRO_HW_CONTEXT_NONE,
	0,
	0,
	0,
	false
};

#ifdef HAVE_OPENGL
VideoDriver sdl2_gl_video_driver = {
	SDL2_GL_InitializeVideo,
	SDL2_GL_RefreshVideo,
	SDL2_GL_CloseVideo,
	SDL2_GL_SetVideoViewport,
	SDL2_GL_SetVideoPixelFormat,
	SDL2_SetVideoGeometry,
	SDL_Delay,
#if (SDL_MAJOR_VERSION == 2) && (SDL_MINOR_VERSION >= 18)
	SDL_GetTicks64,
#else 
	SDL_GetTicks,
#endif
	SDL2_GetFramebuffer,
	SDL2_GL_GetProcAddress,
	RETRO_HW_CONTEXT_NONE,
	0,
	0,
	0,
	false
};
#endif