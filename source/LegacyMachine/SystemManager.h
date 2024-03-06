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

#ifndef _SYSTEM_MANAGER_H
#define _SYSTEM_MANAGER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "CoreLibrary.h"

/**************************************************************************************************
 * Definitions
 *************************************************************************************************/

#define MAX_COUNTERS 64

/**************************************************************************************************
 * SystemManager Structure
 *************************************************************************************************/

typedef struct SystemManager
{
	retro_usec_t				frame_time_last;

	CoreLibrary* current_core;

	struct retro_perf_counter* performance_counters[MAX_COUNTERS];

	struct retro_variable* variables;

	struct retro_system_av_info	av_info;
	struct retro_system_info	system_info;
	struct retro_game_info		content_info;

	/* libretro callbacks */
	struct retro_frame_time_callback	cb_frame_time;
	struct retro_audio_callback			cb_audio;
	struct retro_hw_render_callback		cb_hw_render; 

	unsigned total_performance_counters;
}
SystemManager;

/**************************************************************************************************
 * SystemManager Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

SystemManager* GetSystemManagerContext(void);

RETRO_END_DECLS

#endif