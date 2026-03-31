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
#include <string/stdstring.h>
#include <gfx/scaler/pixconv.h>

#include "VideoManager.h"
#include "../MainEngine.h"

/**************************************************************************************************
 * VideoManager Context
 *************************************************************************************************/

static VideoManager video_manager = { 0 };

/**************************************************************************************************
 * VideoDriver Context Array
 *************************************************************************************************/

const VideoDriver* video_drivers[] = {
#if defined HAVE_SDL
	&sdl_video_driver,
#endif
	NULL
};

/**************************************************************************************************
 * VideoDriver Context
 *************************************************************************************************/

static VideoDriver* video_driver = NULL;

/**************************************************************************************************
 * VideoDriver Framebuffers
 *************************************************************************************************/

/* Optional raw frame buffer placeholder for pixel conversions, where required. */
static uint8_t* raw_buffer = NULL;
/* Normalized source frame buffer. For normalizing frames to vertical resolutions of a multiple of
   60 - i.e. 60, 120, 240, or 480. */
static uint8_t* source_buffer = NULL;
/* Final post-processed output frame buffer to present to screen. Primarily for software filters. */
static uint8_t* output_buffer = NULL;

/* Track allocated sizes to reuse allocations whenever possible. */
static size_t raw_buffer_size = 0;
static size_t source_buffer_size = 0;
static size_t output_buffer_size = 0;

/**************************************************************************************************
 * VideoDriver Functions
 *************************************************************************************************/

/* Verify that VideoDriverID enum values match their corresponding indices in the video_drivers
   array. */
static void AssertVideoDriverEnumValues(void)
{
	for (unsigned i = 0; i < MAX_VIDEO_DRIVERS; ++i)
	{
		retro_assert((VideoDriverID)i == video_drivers[i]->id);
	}
}

/* Initialize the video driver. */
static VideoDriver* InitializeVideoDriver(VideoDriverID driver_id)
{
#if defined _DEBUG
	/* Verify VideoDriverID values when debugging. */
	AssertVideoDriverEnumValues();
#endif
	return (VideoDriver*)video_drivers[driver_id];
}

/* Get the current video driver context. */
static VideoDriver* GetVideoDriverContext(void)
{
	return video_driver;
}

/**************************************************************************************************
 * VideoManager Functions
 *************************************************************************************************/

/* Convert pixel format of source frame to match driver requirements, if required. */
static void ConvertPixelFormat(int width, int height,
	uint32_t src_format, const void* src, int src_pitch,
	uint32_t dst_format, void* dst, int dst_pitch)
{
	/* Fast path: No conversion needed. */
	if (src_format == dst_format)
	{
		if (src == dst)
			return;  /* Already identical, no conversion needed. */

		/* Simple line by line copy (handles different pitches). */
		const uint8_t* src_line = (const uint8_t*)src;
		uint8_t* dst_line = (uint8_t*)dst;
		int bytes_per_line = width * (src_format == RETRO_PIXEL_FORMAT_XRGB8888 ? 4 : 2);

		for (int y = 0; y < height; y++)
		{
			memcpy(dst_line, src_line, bytes_per_line);
			src_line += src_pitch;
			dst_line += dst_pitch;
		}
		return;
	}

	/* Conversion path: Upsampling only. */
	switch (src_format)
	{
		case RETRO_PIXEL_FORMAT_0RGB1555:
		{
			if (dst_format == RETRO_PIXEL_FORMAT_XRGB8888)
				conv_0rgb1555_argb8888(dst, src, width, height, dst_pitch, src_pitch);
			else
			{
				lmc_trace(LMC_LOG_ERRORS,
					"[Video]: Unsupported conversion: %s → %s",
					GetPixelFormatTypeString(src_format),
					GetPixelFormatTypeString(dst_format));
				retro_assert(false);
			}
			break;
		}
		case RETRO_PIXEL_FORMAT_RGB565:
		{
			if (dst_format == RETRO_PIXEL_FORMAT_XRGB8888)
				conv_rgb565_argb8888(dst, src, width, height, dst_pitch, src_pitch);
			else
			{
				lmc_trace(LMC_LOG_ERRORS,
					"[Video]: Unsupported conversion: %s → %s",
					GetPixelFormatTypeString(src_format),
					GetPixelFormatTypeString(dst_format));
				retro_assert(false);
			}
			break;
		}
		default:
		{
			lmc_trace(LMC_LOG_ERRORS,
				"[Video]: Unsupported conversion: %s → %s",
				GetPixelFormatTypeString(src_format),
				GetPixelFormatTypeString(dst_format));
			retro_assert(false);
			break;
		}
	}
}

/* Reset framebuffer data to zero. */
static void ResetFramebuffers(void)
{
	RenderInfo* output = GetRenderInfo();

	memset(&output->frame_adjust, 0, sizeof(AdjustmentInfo));
	memset(&output->source_frame, 0, sizeof(FrameInfo));
	memset(&output->output_frame, 0, sizeof(FrameInfo));
}

/* Get the current video manager context. */
VideoManager* GetVideoManagerContext(void)
{
	return &video_manager;
}

/* Get the current video driver ID. */
VideoDriverID GetVideoDriverID(void)
{
	VideoDriver* driver = GetVideoDriverContext();
	return driver ? driver->id : VIDEO_DRIVER_NONE;
}

/* Initialize the window manager with the specified window driver and window viewport
   dimensions. */
bool InitializeVideo(VideoDriverID driver_id, ViewportInfo* viewport)
{
	VideoManager* video = GetVideoManagerContext();
	VideoDriver* driver = InitializeVideoDriver(driver_id);

	if (!driver || !driver->cb_init || !driver->cb_set_viewport)
		return false;

	video_driver = driver;

	driver->cb_set_viewport(viewport);

	video->initialized = driver->cb_init();

	return video->initialized;
}

/* Deinitialize the video manager and its driver. */
void DeinitializeVideo(void)
{
	VideoManager* video = GetVideoManagerContext();
	VideoDriver* driver = GetVideoDriverContext();

	if (driver && driver->cb_deinit)
	{
		driver->cb_deinit();
	}

	video_driver = NULL;

	memset(video, 0, sizeof(VideoManager));
}

/* Initialize output framebuffers. */
bool InitializeFramebuffers(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	VideoDriver* driver = GetVideoDriverContext();
	RenderInfo* output = GetRenderInfo();
	CRTEffect* crt_effect = GetCRTEffect();

	int total_padding = 0;
	int upper_padding = 0;
	int lower_padding = 0;
	int upper_padding_size = 0;
	int lower_padding_size = 0;

	int total_buffers = 1;
	bool raw_pixel_conversion_buffer = false;
	bool source_frame_normalizing_buffer = false;
	bool output_post_processing_buffer = false;

	/* Set bpp32 based on CRT filter requirement. */
	output->bpp32 = crt_effect->enabled;

	/* Reset framebuffers (do not free existing allocations here so they can be reused).
	   The buffers themselves (raw_buffer/source_buffer/output_buffer) are kept
	   allocated across calls unless a larger size is required. */
	ResetFramebuffers();

	/* Ensure frame pointers are cleared; we'll assign appropriate pointers below. */
	output->raw_frame.data = NULL;
	output->source_frame.data = NULL;
	output->output_frame.data = NULL;

	/* Store base raw frame size. */
	output->size = output->raw_frame.size;

	/* Create a separate pixel conversion buffer, if needed. */
	if ((output->bpp32) && (output->raw_frame.format != RETRO_PIXEL_FORMAT_XRGB8888))
	{
		output->raw_frame.depth = 32;
		output->raw_frame.pitch = (((output->raw_frame.width * output->raw_frame.depth) >> 3) + 3) & ~0x03;
		output->raw_frame.size = output->raw_frame.pitch * output->raw_frame.height;

		/* Reuse existing raw_buffer if large enough, otherwise reallocate. */
		if (raw_buffer == NULL || raw_buffer_size < output->raw_frame.size)
		{
			if (raw_buffer)
			{
				free(raw_buffer);
				raw_buffer = NULL;
				raw_buffer_size = 0;
			}
			raw_buffer = (uint8_t*)malloc(output->raw_frame.size);
			if (raw_buffer)
				raw_buffer_size = output->raw_frame.size;
		}

		output->raw_frame.data = raw_buffer;

		if (!output->raw_frame.data)
			return false;

		raw_pixel_conversion_buffer = true;
		total_buffers++;
	}

	/* Determine if source frame requires normalized adjusting and initialize accordingly. */
	output->source_frame.width = output->raw_frame.width;
	output->source_frame.format = output->raw_frame.format;
	output->source_frame.depth = output->raw_frame.depth;
	output->source_frame.pitch = output->raw_frame.pitch;
	output->source_frame.height = output->raw_frame.height;

	if (output->type != OUTPUT_RAW)
	{
		if (output->raw_frame.height <= 66)
		{
			if (output->raw_frame.height != 60)
			{
				output->frame_adjust.adjust = true;
				output->source_frame.height = 60;
			}
		}
		else if (output->raw_frame.height <= 132)
		{
			if (output->raw_frame.height != 120)
			{
				output->frame_adjust.adjust = true;
				output->source_frame.height = 120;
			}
		}
		else if (output->raw_frame.height <= 262)
		{
			if (output->raw_frame.height != 240)
			{
				output->frame_adjust.adjust = true;
				output->source_frame.height = 240;
			}
		}
		else if (output->raw_frame.height <= 524)
		{
			if (output->raw_frame.height != 480)
			{
				output->frame_adjust.adjust = true;
				output->source_frame.height = 480;
			}
		}
	}

	output->source_frame.size = output->source_frame.height * output->source_frame.pitch;

	/* Initialize adjustment parameters, if required. */
	if (output->frame_adjust.adjust)
	{
		total_padding = output->source_frame.height - output->raw_frame.height;

		/* Split evenly, adding any odd line to the lower side. */
		upper_padding = total_padding / 2;
		lower_padding = total_padding - upper_padding;

		upper_padding_size = upper_padding * output->source_frame.pitch;
		lower_padding_size = lower_padding * output->source_frame.pitch;

		output->frame_adjust.upper_pad_size = upper_padding_size;
		output->frame_adjust.lower_pad_size = lower_padding_size;
		output->frame_adjust.upper_pad_lines = upper_padding;
		output->frame_adjust.lower_pad_lines = lower_padding;
	}

	/* Reuse existing source_buffer if large enough, otherwise reallocate. */
	if (source_buffer == NULL || source_buffer_size < output->source_frame.size)
	{
		if (source_buffer)
		{
			free(source_buffer);
			source_buffer = NULL;
			source_buffer_size = 0;
		}
		source_buffer = (uint8_t*)malloc(output->source_frame.size);
		if (source_buffer)
			source_buffer_size = output->source_frame.size;
	}

	output->source_frame.data = source_buffer;

	if (!output->source_frame.data)
		return false;

	source_frame_normalizing_buffer = true;
	total_buffers++;

	/* Initialize output buffer. */
	output->output_frame.width = output->source_frame.width;
	output->output_frame.height = output->source_frame.height;
	output->output_frame.format = output->source_frame.format;
	output->output_frame.depth = output->source_frame.depth;
	output->output_frame.pitch = output->source_frame.pitch;
	output->output_frame.size = output->source_frame.size;

	lmc_trace(LMC_LOG_VERBOSE, "[Video]: Output information:");
	if (legacy_machine->log_level == LMC_LOG_VERBOSE)
	{
		int count = 1;
		printf("\tTotal active framebuffers: %i\n", total_buffers);
		printf("\tFramebuffer #%u: Raw frame size: %i\n", count, output->size);
		if (raw_pixel_conversion_buffer)
		{
			count++;
			printf("\tFramebuffer #%u: Raw pixel conversion frame size: %i\n", 
				count, output->raw_frame.size);
		}
		if (source_frame_normalizing_buffer)
		{
			count++;
			printf("\tFramebuffer #%u: Source normalized frame size: %i\n", 
				count, output->source_frame.size);
		}
		if (output_post_processing_buffer)
		{
			count++;
			printf("\tFramebuffer #%u: Output post-processing frame size: %i\n", 
				count, output->output_frame.size);
		}
	}

	/* After manager buffers are set up, let driver initialize its resources. */
	if (driver && driver->cb_init_fb)
	{
		if (!driver->cb_init_fb())
		{
			lmc_trace(LMC_LOG_ERRORS, "[Video]: Driver failed to initialize framebuffers");
			return false;
		}
	}

	return true;
}

/* Free framebuffers and release pointers. */
void DeinitializeFramebuffers(void)
{
	RenderInfo* output = GetRenderInfo();

	/* Release pointers to buffers. */
	output->raw_frame.data = NULL;
	output->source_frame.data = NULL;
	output->output_frame.data = NULL;

	/* Free any existing buffers and set them to null. */
	if (raw_buffer)
	{
		free(raw_buffer);
		raw_buffer = NULL;
		raw_buffer_size = 0;
	}
	if (source_buffer)
	{
		free(source_buffer);
		source_buffer = NULL;
		source_buffer_size = 0;
	}
	if (output_buffer)
	{
		free(output_buffer);
		output_buffer = NULL;
		output_buffer_size = 0;
	}
}

/* Refresh the video frame. */
void RefreshVideo(const void* data, unsigned width, unsigned height, unsigned pitch)
{
	RenderInfo* output = GetRenderInfo();
	VideoDriver* driver = GetVideoDriverContext();

	/* Null data indicates frame dupe - core reusing previous frame. */
	if (!data)
		return;

	/* Detect resolution change and update framebuffers. */
	if (!output->current ||
		output->raw_frame.width != width ||
		output->raw_frame.height != height)
	{
		/* Update geometry if dimensions changed. */
		if (output->raw_frame.width != width || output->raw_frame.height != height)
		{
			struct retro_game_geometry geometry = {
				.base_width = width,
				.base_height = height,
				.max_width = output->max_width > width ? output->max_width : width,
				.max_height = output->max_height > height ? output->max_height : height,
				.aspect_ratio = output->aspect_ratio
			};
			SetVideoGeometry(&geometry);
		}

		output->raw_frame.pitch = pitch;
		output->raw_frame.size = output->raw_frame.pitch * output->raw_frame.height;

		/* Reinitialize framebuffers. */
		if (!InitializeFramebuffers())
		{
			lmc_trace(LMC_LOG_ERRORS, "[Video]: Failed to initialize framebuffers");
			return;
		}

		output->current = true;
	}

	if (output->bpp32 && output->raw_frame.format != RETRO_PIXEL_FORMAT_XRGB8888)
	{
		/* Convert pixel format, if required. */
		ConvertPixelFormat(width, height,
			output->raw_frame.format, data, pitch,
			RETRO_PIXEL_FORMAT_XRGB8888, output->raw_frame.data, output->raw_frame.pitch);
	}
	else
	{
		/* No conversion required, just assign data pointer and dimensions. */
		output->raw_frame.data = (uint8_t*)data;
		output->raw_frame.width = width;
		output->raw_frame.height = height;
		output->raw_frame.pitch = pitch;
	}

	/* Adjust frame with vertical padding or cropping if necessary.
	   Otherwise, pass data pointer to "source" frame. */
	if (output->frame_adjust.adjust)
	{
		BlitAdjustedFrame(
			output->raw_frame.data,
			output->raw_frame.size,
			output->raw_frame.height,
			output->raw_frame.pitch,
			output->source_frame.data,
			&output->frame_adjust
		);
	}
	else
	{
		output->source_frame.data = output->raw_frame.data;
	}

	output->output_frame.data = output->source_frame.data;

	if (driver && driver->cb_refresh)
		driver->cb_refresh(&output->output_frame);
}

/* Get video output info. */
RenderInfo* GetRenderInfo(void)
{
	VideoManager* video = GetVideoManagerContext();

	return &video->output;
}

/* Get the crt video filter. */
CRTEffect* GetCRTEffect(void)
{
	VideoManager* video = GetVideoManagerContext();

	return &video->crt_effect;
}

void SetVideoViewport(ViewportInfo* viewport)
{
	VideoDriver* driver = GetVideoDriverContext();
	if (driver && driver->cb_set_viewport)
		driver->cb_set_viewport(viewport);
}

/* Set pixel format. */
bool SetVideoPixelFormat(unsigned format)
{
	RenderInfo* output = GetRenderInfo();
	VideoDriver* driver = GetVideoDriverContext();

	if (format > RETRO_PIXEL_FORMAT_RGB565)
		return false;

	/* Update manager state. */
	output->raw_frame.format = format;

	if (!output->bpp32)
	{
		switch (format)
		{
			case RETRO_PIXEL_FORMAT_0RGB1555:
			{
				output->raw_frame.depth = 16;
				break;
			}
			case RETRO_PIXEL_FORMAT_XRGB8888:
			{
				output->raw_frame.depth = 32;
				break;
			}
			case RETRO_PIXEL_FORMAT_RGB565:
			{
				output->raw_frame.depth = 16;
				break;
			}
		}
	}
	else
		output->raw_frame.depth = 32;

	/* Notify driver of format change. */
	if (driver && driver->cb_set_pixel_fmt)
		return driver->cb_set_pixel_fmt(format);

	return true;
}

/* Store video geometry locally. */
void SetVideoGeometry(const struct retro_game_geometry* geometry)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	RenderInfo* output = GetRenderInfo();
	VideoDriver* driver = GetVideoDriverContext();

	/* Check if geometry actually changed. */
	if (output->raw_frame.width != geometry->base_width ||
		output->raw_frame.height != geometry->base_height ||
		output->aspect_ratio != geometry->aspect_ratio)
	{
		/* Update manager state. */
		output->aspect_ratio = geometry->aspect_ratio;
		output->raw_frame.width = geometry->base_width;
		output->raw_frame.height = geometry->base_height;
		output->max_width = geometry->max_width;
		output->max_height = geometry->max_height;

		/* Invalidate current output (needs reinitialization). */
		output->current = false;

		/* Notify window manager to resize if already initialized. */
		if (legacy_machine->window->initialized)
			SetWindowGeometry(geometry);

		/* Notify driver of geometry change. */
		if (driver && driver->cb_set_geometry_fmt)
			driver->cb_set_geometry_fmt(geometry);
	}
}

/* Initialize system hardware render callback structure. */
void InitializeHardwareRenderCallback(void)
{
	/*LMC_Engine legacy_machine = LMC_GetContext();

	legacy_machine->system->cb_hw_render.context_type = legacy_machine->video->hw_context;
	legacy_machine->system->cb_hw_render.version_major = legacy_machine->video->hw_api.version_major;
	legacy_machine->system->cb_hw_render.version_minor = legacy_machine->video->hw_api.version_minor;
	legacy_machine->system->cb_hw_render.get_current_framebuffer = legacy_machine->video->cb_get_framebuffer;
	if (legacy_machine->video->cb_get_hw_proc_address)
		legacy_machine->system->cb_hw_render.get_proc_address = legacy_machine->video->cb_get_hw_proc_address;*/
}

/* Set the hardware render callback being used. */
void SetHardwareRenderCallback(struct retro_hw_render_callback* hw_render)
{
	/*LMC_Engine legacy_machine = LMC_GetContext();

	struct retro_hw_render_callback* hardware_render_callback = hw_render;
	hardware_render_callback->context_type = legacy_machine->video->hw_context;
	hardware_render_callback->version_major = legacy_machine->video->hw_api.version_major;
	hardware_render_callback->version_minor = legacy_machine->video->hw_api.version_minor;
	hardware_render_callback->get_current_framebuffer = legacy_machine->video->cb_get_framebuffer;
	if (legacy_machine->video->cb_get_hw_proc_address)
		hardware_render_callback->get_proc_address = legacy_machine->video->cb_get_hw_proc_address;
	legacy_machine->system->cb_hw_render = *hardware_render_callback;*/
}

/* Get pixel format as human readable text. */
const char* GetPixelFormatTypeString(unsigned format)
{
	switch (format)
	{
		case RETRO_PIXEL_FORMAT_0RGB1555:
			return "RETRO_PIXEL_FORMAT_0RGB1555";
		case RETRO_PIXEL_FORMAT_XRGB8888:
			return "RETRO_PIXEL_FORMAT_XRGB8888";
		case RETRO_PIXEL_FORMAT_RGB565:
			return "RETRO_PIXEL_FORMAT_RGB565";
	}

	return "Invalid format";
}

/* Get hardware context type as human readable text. */
const char* GetHardwareContextTypeString(enum retro_hw_context_type hw_context)
{
	switch (hw_context)
	{
		case RETRO_HW_CONTEXT_NONE:
			return "RETRO_HW_CONTEXT_NONE";
		case RETRO_HW_CONTEXT_OPENGL:
			return "RETRO_HW_CONTEXT_OPENGL";
		case RETRO_HW_CONTEXT_OPENGLES2:
			return "RETRO_HW_CONTEXT_OPENGLES2";
		case RETRO_HW_CONTEXT_OPENGL_CORE:
			return "RETRO_HW_CONTEXT_OPENGL_CORE";
		case RETRO_HW_CONTEXT_OPENGLES3:
			return "RETRO_HW_CONTEXT_OPENGLES3";
		case RETRO_HW_CONTEXT_OPENGLES_VERSION:
			return "RETRO_HW_CONTEXT_OPENGLES_VERSION";
		case RETRO_HW_CONTEXT_VULKAN:
			return "RETRO_HW_CONTEXT_VULKAN";
		case RETRO_HW_CONTEXT_DIRECT3D:
			return "RETRO_HW_CONTEXT_DIRECT3D";
	}

	return "Invalid Context";
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
