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

#ifndef _SYSTEM_MANAGER_H
#define _SYSTEM_MANAGER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <queues/message_queue.h>
#include <streams/interface_stream.h>
#include <streams/file_stream.h>

#if defined HAVE_THREADS
#include <rthreads/rthreads.h>
#endif

#if defined(HAVE_COMPRESSION) && defined(HAVE_ZLIB)
#include <streams/rzip_stream.h>
#endif

#include "CoreLibrary.h"
#include "OptionsManager.h"
#include "Common/Common.h"

/**************************************************************************************************
 * Definitions
 *************************************************************************************************/

#define MAX_COUNTERS 64

/**************************************************************************************************
 * CoreManager Structure
 *************************************************************************************************/

typedef struct CoreManager
{
	/* Pointer to currently loaded libretro core library. */
	CoreLibrary*	library;
	/* Pointer to options manager for the core. */
	OptionsManager*	options;
	/* Name of the currently loaded core. */
	char			name[NAME_MAX_LENGTH];
	/* Version of the currently loaded core. */
	char			version[NAME_MAX_LENGTH];
	/* Valid content file extensions for the core. */
	char			extensions[NAME_MAX_LENGTH];
	/* File name of the currently loaded core library. */
	char			file_name[NAME_MAX_LENGTH];
	/* File path of the currently loaded core library. */
	char			file_path[PATH_MAX_LENGTH];
	/* Path to the core's options configuration file. */
	char			options_path[PATH_MAX_LENGTH];
	/* Directory for core-specific save files. */
	char			save_directory[PATH_MAX_LENGTH];
	/* Path to the SRAM save file. */
	char			save_path[PATH_MAX_LENGTH];
	/* Path to the RTC clock save file. */
	char			clock_path[PATH_MAX_LENGTH];
	/* Array of file paths to save state slots. */
	char**			state_paths;
	/* Device type active on each controller port. */
	unsigned		port_devices[MAX_PLAYERS];
	/* Number of controller ports the core uses. */
	unsigned		port_count;
	/* Whether the core requires the full path to content files. */
	bool			need_fullpath;
	/* Whether the core should block extraction of archived content. */
	bool			block_extract;
	/* Whether the core can run without content. */
	bool			contentless;
}
CoreManager;

/**************************************************************************************************
 * ContentManager Structure
 *************************************************************************************************/

typedef struct ContentManager
{
	uint8_t*	data;							/* Pointer to content data in memory. */
	size_t		size;							/* Size of content data in bytes. */
	char		file_name[NAME_MAX_LENGTH];		/* Content file name. */
	char		file_path[PATH_MAX_LENGTH];		/* Full file path to the content. */
	char		archive_path[PATH_MAX_LENGTH];	/* Path to the content within an archive. */
	char		temp_path[PATH_MAX_LENGTH];		/* Temporary path for extracted content. */
	bool		archived;						/* Whether content is inside an archive. */
	bool		extract;						/* Whether extraction is required. */
}
ContentManager;

/**************************************************************************************************
 * SystemManager Structure
 *************************************************************************************************/

typedef struct SystemManager
{
	/*********************************/
	/* ptr aligned variables.        */
	/*********************************/

	retro_time_t	(*cb_get_time_elapsed)(void);	/* Elapsed time query callback. */

	CoreManager*	core;				/* Pointer to the core manager. */
	ContentManager*	content;			/* Pointer to the content manager. */

	msg_queue_t		message_queue;		/* Message queue for task messages. */
#if defined HAVE_THREADS
	slock_t*		message_queue_lock;	/* Thread lock for the message queue. */
#endif

	/* Registered performance counter pointers. */
	struct retro_perf_counter* performance_counters[MAX_COUNTERS];

	/*********************************/
	/* 8 byte aligned variables.     */
	/*********************************/

	/* Frame time callback. */
	struct retro_frame_time_callback			cb_frame_time;
	/* Audio sample callback. */
	struct retro_audio_callback					cb_audio;
	/* Audio buffer status callback. */
	struct retro_audio_buffer_status_callback	cb_audio_buffer_status;
	/* Hardware render context callback. */
	struct retro_hw_render_callback				cb_hw_render;
	/* Extended disk control callback interface. */
	struct retro_disk_control_ext_callback		cb_disk_control;
	/* Keyboard event callback. */
	struct retro_keyboard_callback				cb_keyboard;

	/* Core audio/video info. */
	struct retro_system_av_info					av_info;

	size_t			message_queue_size;	/* Message queue capacity. */
	retro_usec_t	frame_time_last;	/* Last frame time in microseconds. */

	/*********************************/
	/* 4 byte aligned variables.     */
	/*********************************/

	unsigned total_performance_counters;	/* Total registered performance counters. */

	/*********************************/
	/* 2 byte aligned variables.     */
	/*********************************/

	/*********************************/
	/* 1 byte aligned variables.     */
	/*********************************/
#if defined(HAVE_COMPRESSION) && defined(HAVE_ZLIB)
	bool compress_states;	/* Whether save state files are compressed. */
	bool compress_saves;	/* Whether SRAM/RTC save files are compressed. */
#endif
	bool shutdown;			/* Whether a system shutdown was requested. */
	bool running;			/* Whether the core is currently running. */
	bool initialized;		/* Whether the system manager is initialized. */
}
SystemManager;

/**************************************************************************************************
 * SystemManager Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

SystemManager* GetSystemManagerContext(void);

void InitializeLibretro(bool threaded);

bool InitializeSystem(void);
void DeinitializeSystem(void);

bool RunCore(void);

bool SetControllerPortDevice(unsigned port, unsigned device);

void UpdateAudio(void);

bool ReadSaveFile(const char* path, const char* save_type, unsigned save_type_id);
bool WriteSaveFile(const char* path, const char* save_type, unsigned save_type_id);

RETRO_END_DECLS

#endif
