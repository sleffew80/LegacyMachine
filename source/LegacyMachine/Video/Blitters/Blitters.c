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
#include <stdlib.h>
#include <string.h>

#include "Blitters.h"

/**************************************************************************************************
 * Blitting Functions
 *************************************************************************************************/

 /* Generic blit. Blits a block of pixel data from source to destination, handling different 
    pitches. */
void Blit(const uint8_t* src_ptr, uint8_t* dst_ptr, int src_pitch, int lines, int dst_pitch)
{
	int y;
	for (y = 0; y < lines; y += 1)
	{
		memcpy(dst_ptr, src_ptr, src_pitch);
		src_ptr += src_pitch;
		dst_ptr += dst_pitch;
	}
}

/* Blit a frame with adjustments for upper and lower padding. Handles both positive padding
   (adding black lines) and negative padding (cropping lines). */
void BlitAdjustedFrame(const uint8_t* src_ptr, int src_size, int src_lines, int src_pitch, 
	uint8_t* dst_ptr, AdjustmentInfo* info)
{
	int total_crop_lines = 0;

	/* Handle upper padding. Set empty (black) lines if padding is positive or skip
	   lines if padding is negative. */
	if (info->upper_pad_lines > 0)
	{
		memset(dst_ptr, 0, info->upper_pad_size);
		dst_ptr += info->upper_pad_size;
	}
	else if (info->upper_pad_lines < 0)
	{
		for (int y = 0; y < abs(info->upper_pad_lines); y++)
			src_ptr += src_pitch;
	}

	/* Calculate total crop lines. */
	if (info->upper_pad_lines < 0 || info->lower_pad_lines < 0)
	{
		total_crop_lines = (info->upper_pad_lines < 0 ? abs(info->upper_pad_lines) : 0)
			+ (info->lower_pad_lines < 0 ? abs(info->lower_pad_lines) : 0);
	}


	/* Copy source lines to destination. */
	for (int y = 0; y < src_lines - total_crop_lines; y++)
	{
		memcpy(dst_ptr, src_ptr, src_pitch);
		src_ptr += src_pitch;
		dst_ptr += src_pitch;
	}

	/* Handle lower padding. */
	if (info->lower_pad_lines > 0)
	{
		memset(dst_ptr, 0, info->lower_pad_size);
	}
}

/* Blit a source frame onto a destination frame/image at the specified (dst_x, dst_y) position. */
void BlitOverlayedFrame(const uint8_t* src_ptr, int src_lines, int src_pitch,
	uint8_t* dst_ptr, int dst_lines, int dst_pitch, int dst_x, int dst_y)
{
	/* Calculate starting position in destination. */
	uint8_t* dst_line = dst_ptr + (dst_y * dst_pitch) + dst_x;
	const uint8_t* src_line = src_ptr;

	/* Copy each line from source to destination. */
	for (int y = 0; y < src_lines; y++)
	{
		memcpy(dst_line, src_line, src_pitch);
		src_line += src_pitch;
		dst_line += dst_pitch;
	}
}