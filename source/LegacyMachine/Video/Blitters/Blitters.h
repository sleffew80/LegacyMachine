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

#ifndef _BLITTERS_H
#define _BLITTERS_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "../../Common/Common.h"

/**************************************************************************************************
 * Blitting Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

void Blit(const uint8_t* src_ptr, uint8_t* dst_ptr, int src_pitch, int lines, int dst_pitch);
void BlitAdjustedFrame(const uint8_t* src_ptr, int src_size, int src_lines, int src_pitch,
	uint8_t* dst_ptr, AdjustmentInfo* info);
void BlitOverlayedFrame(const uint8_t* src_ptr, int src_lines, int src_pitch,
	uint8_t* dst_ptr, int dst_lines, int dst_pitch, int dst_x, int dst_y);

RETRO_END_DECLS

#endif