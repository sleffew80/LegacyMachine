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

#ifndef _RETRO_TYPES_H
#define _RETRO_TYPES_H

#include <retro_common_api.h>
#include <retro_miscellaneous.h>
#include <streams/interface_stream.h>

/**************************************************************************************************
 * Additional Libretro Specific Types/Structures
 *************************************************************************************************/

typedef struct retro_sram_block
{
	void* data;
	size_t size;
	unsigned type;
} retro_sram_block_t;

typedef struct retro_serialize_info
{
	const void* data_const;
	void* data;
	size_t size;
} retro_serialize_info_t;

typedef struct retro_save_state_task
{
	intfstream_t* file;
	void* data;
	ssize_t size;
	ssize_t written;
	ssize_t read;
	int slot;
	char path[PATH_MAX_LENGTH];
	bool compress;
	bool mute;
}
retro_save_state_task_t;

#endif