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
#include <ctype.h>

#include <compat/strl.h>

#include <retro_math.h>
#include <retro_timers.h>

#include <dynamic/dylib.h>
#include <features/features_cpu.h>
#include <file/file_path.h>
#include <queues/task_queue.h>
#include <streams/file_stream.h>
#include <string/stdstring.h>
#include <time/rtime.h>

#if HAVE_COMPRESSION
#include <file/archive_file.h>
#include <lists/dir_list.h>
#endif

#include "MainEngine.h"
#include "SystemManager.h"
#include "SettingsManager.h"
#include "TaskHandler.h"
#include "Logging.h"

#include "Audio/AudioManager.h"
#include "Video/VideoManager.h"
#include "Input/InputManager.h"
#include "Window/WindowManager.h"

/**************************************************************************************************
 * Macro Definitions
 *************************************************************************************************/
#define LoadSymbol(V, S) do { \
	function_t retro_function = dylib_proc(system->core->library->handle, #S); \
	memcpy(&V, &retro_function, sizeof(retro_function)); \
	if (!V) { lmc_core_log(RETRO_LOG_ERROR, "Failed to load symbol: \"%s\"\n", #S); } \
} while (0)

#define LoadRetroSymbol(S) LoadSymbol(system->core->library->S, S)

/**************************************************************************************************
 * SystemManager Context
 *************************************************************************************************/

static SystemManager system_manager = { 0 };

/**************************************************************************************************
 * SystemManager Static Functions
 *************************************************************************************************/

/* Empty function for callbacks where NULL won't do. */
static void empty_function(void)
{
}

/* Gets time elapsed in microseconds. */
static retro_time_t GetTimeElapsed(void)
{
	return cpu_features_get_time_usec();
}

/* Gets CPU Features. */
static uint64_t GetCPUFeatures(void)
{
	return cpu_features_get();
}

/* A simple performance counter. */
static retro_perf_tick_t GetCorePerformanceCounter(void)
{
	return cpu_features_get_perf_counter();
}

/* Registers a performance counter. */
static void RegisterCorePerformanceCounter(struct retro_perf_counter* counter)
{
	SystemManager* system = GetSystemManagerContext();

	if (counter->registered || (system->total_performance_counters >= MAX_COUNTERS))
		return;

	system->performance_counters[system->total_performance_counters++] = counter;
	counter->registered = true;
}

/* Start a registered performance counter. */
static void StartCorePerformanceCounter(struct retro_perf_counter* counter)
{
	if (counter->registered)
	{
		counter->start = GetCorePerformanceCounter();
	}
}

/* Stop a registered performance counter. */
static void StopCorePerformanceCounter(struct retro_perf_counter* counter)
{
	counter->total += GetCorePerformanceCounter() - counter->start;
}

/* Log and output the state of performance counters. */
static void LogCorePerformanceCounters(struct retro_perf_counter** counters, unsigned total)
{
	unsigned i;
	for (i = 0; i < total; i++)
	{
		if (counters[i]->call_cnt)
		{
			lmc_core_log(RETRO_LOG_INFO,
				"[Performance]: %s: %i - %i\n",
				counters[i]->ident,
				(uint64_t)counters[i]->start,
				(uint64_t)counters[i]->total);
		}
	}
}

/* Log and output the state of core performance. */
static void LogCorePerformance(void)
{
	SystemManager* system = GetSystemManagerContext();

	lmc_core_log(RETRO_LOG_INFO, "[Performance]: Counters:\n");
	LogCorePerformanceCounters(system->performance_counters,
		system->total_performance_counters);
}

/* Core environment management. */
static bool CoreEnvironment(unsigned command, void* data)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	SystemManager* system = GetSystemManagerContext();
	SettingsManager* settings = GetSettingsManagerContext();
	AudioManager* audio = GetAudioManagerContext();
	VideoManager* video = GetVideoManagerContext();

	/* Called every frame. Handle first for efficiency. */
	if (command == RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE)
	{
		bool* updated = (bool*)data;

		if (updated)
		{
			*updated = system->core->options->modified;
			system->core->options->modified = false;
		}

		return true;
	}

	switch (command)
	{
		case RETRO_ENVIRONMENT_SET_ROTATION:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_ROTATION: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_OVERSCAN:
		{
			bool* value = (bool*)data;
			*value = true;
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_OVERSCAN: true");
			return true;
		}
		case RETRO_ENVIRONMENT_GET_CAN_DUPE:
		{
			bool* value = (bool*)data;
			*value = true;
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_CAN_DUPE: true");
			return true;
		}
		case RETRO_ENVIRONMENT_SET_MESSAGE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_MESSAGE");
			const struct retro_message* message = (const struct retro_message*)data;
			if (message)
			{
				lmc_core_log(RETRO_LOG_INFO, "\t%s\n", message->msg);
			}
			return true;
		}
		case RETRO_ENVIRONMENT_SHUTDOWN:
		{
			system->shutdown = true;
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SHUTDOWN: requested");
			return true;
		}
		case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_PERFORMANCE_LEVEL: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
		{
			const char** directory = (const char**)data;

			if (directory)
				*directory = settings->paths.path_system_directory;

			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_SYSTEM_DIRECTORY: \"%s\"",
				settings->paths.path_system_directory);

			return true;
		}
		case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
		{
			const enum retro_pixel_format* format = (enum retro_pixel_format*)data;

			if (*format > RETRO_PIXEL_FORMAT_RGB565)
				return false;

			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_PIXEL_FORMAT");

			return SetVideoPixelFormat(*format);
		}
		case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
		{
			const struct retro_input_descriptor* desc =
				(const struct retro_input_descriptor*)data;

			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_INPUT_DESCRIPTORS:");

			if (legacy_machine->log_level == LMC_LOG_VERBOSE)
			{
				while (desc->description != NULL)
				{
					printf("\tPort %u, Device 0x%04x, Index %u, ID %u: %s\n",
						desc->port, desc->device, desc->index, desc->id, desc->description);

					desc++;
				}
			}

			return true;
		}
		case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_KEYBOARD_CALLBACK");

			const struct retro_keyboard_callback* keyboard_cb =
				(const struct retro_keyboard_callback*)data;

			if (keyboard_cb)
				system->cb_keyboard = *keyboard_cb;

			return true;
		}
		case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_DISK_CONTROL_INTERFACE");

			const struct retro_disk_control_callback* disk_control_cb =
				(const struct retro_disk_control_callback*)data;

			if (disk_control_cb)
			{
				memset(&system->cb_disk_control, 0, 
					sizeof(struct retro_disk_control_ext_callback));
				memcpy(&system->cb_disk_control, disk_control_cb, 
					sizeof(struct retro_disk_control_callback));
			}
			return true;
		}
		case RETRO_ENVIRONMENT_SET_HW_RENDER:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_HW_RENDER");

			/*struct retro_hw_render_callback* hardware_render_cb =
				(struct retro_hw_render_callback*)data;

			if (hardware_render_cb)
			{
				hardware_render_cb->context_type = video->hw_context;
				hardware_render_cb->version_major = video->hw_api.version_major;
				hardware_render_cb->version_minor = video->hw_api.version_minor;
				hardware_render_cb->get_current_framebuffer = video->cb_get_framebuffer;
				if (video->cb_get_hw_proc_address)
					hardware_render_cb->get_proc_address = video->cb_get_hw_proc_address;
				system->cb_hw_render = *hardware_render_cb;
			}*/

			return true;
		}
		case RETRO_ENVIRONMENT_GET_VARIABLE:
		{
			struct retro_variable* variable = (struct retro_variable*)data;

			if (variable && variable->key)
				variable->value = GetOptionValue(variable->key);

			return true;
		}
		case RETRO_ENVIRONMENT_SET_VARIABLES:
		{
			struct retro_variable* variables = (struct retro_variable*)data;

			DeinitializeOptions();

			if (variables)
			{
				lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_VARIABLES");

				InitializeOptionsVariables(system->core->options_path,
					system->core->name,
					variables);
			}

			return true;
		}
		case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
		{
			bool value = *(const bool*)data;

			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_SUPPORT_NO_GAME: %s",
				value ? "true" : "false");

			system->core->library->supports_no_game = value;

			return true;
		}
		case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH:
		{
			const char** directory = (const char**)data;

			if (directory)
				*directory = settings->paths.path_core_directory;

			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_LIBRETRO_PATH: \"%s\"",
				settings->paths.path_core_directory);

			return true;
		}
		case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_FRAME_TIME_CALLBACK");

			const struct retro_frame_time_callback* frame_time_cb =
				(const struct retro_frame_time_callback*)data;

			if (frame_time_cb)
				system->cb_frame_time = *frame_time_cb;

			return true;
		}
		case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_AUDIO_CALLBACK");

			struct retro_audio_callback* audio_cb = (struct retro_audio_callback*)data;

			if (audio_cb)
				system->cb_audio = *audio_cb;

			return true;
		}
		case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_RUMBLE_INTERFACE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES:
		{
			uint64_t* capabilities = (uint64_t*)data;

			if (capabilities)
				*capabilities = (1 << RETRO_DEVICE_JOYPAD) |
								(1 << RETRO_DEVICE_MOUSE) |
								(1 << RETRO_DEVICE_KEYBOARD) |
								(1 << RETRO_DEVICE_ANALOG);

			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_INPUT_DEVICE_CAPABILITIES:");
			if (legacy_machine->log_level == LMC_LOG_VERBOSE)
			{
				printf("\tRETRO_DEVICE_JOYPAD: %s\n",
					(*capabilities & (1 << RETRO_DEVICE_JOYPAD)) ? "supported" : "not supported");
				printf("\tRETRO_DEVICE_ANALOG: %s\n",
					(*capabilities & (1 << RETRO_DEVICE_ANALOG)) ? "supported" : "not supported");
				printf("\tRETRO_DEVICE_MOUSE: %s\n",
					(*capabilities & (1 << RETRO_DEVICE_MOUSE)) ? "supported" : "not supported");
				printf("\tRETRO_DEVICE_KEYBOARD: %s\n",
					(*capabilities & (1 << RETRO_DEVICE_KEYBOARD)) ? "supported" : "not supported");
			}

			return true;
		}
		case RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_SENSOR_INTERFACE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_CAMERA_INTERFACE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_LOG_INTERFACE");

			struct retro_log_callback* log_cb = (struct retro_log_callback*)data;
			if (log_cb)
				log_cb->log = lmc_core_log;

			return true;
		}
		case RETRO_ENVIRONMENT_GET_PERF_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_PERF_INTERFACE");

			struct retro_perf_callback* performance_cb = (struct retro_perf_callback*)data;

			if (performance_cb)
			{
				performance_cb->get_time_usec = GetTimeElapsed;
				performance_cb->get_cpu_features = GetCPUFeatures;
				performance_cb->get_perf_counter = GetCorePerformanceCounter;
				performance_cb->perf_register = RegisterCorePerformanceCounter;
				performance_cb->perf_start = StartCorePerformanceCounter;
				performance_cb->perf_stop = StopCorePerformanceCounter;
				performance_cb->perf_log = LogCorePerformance;
			}

			return true;
		}
		case RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_LOCATION_INTERFACE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY:
		{
			const char** directory = (const char**)data;

			if (directory)
				*directory = settings->paths.path_content_directory;

			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_CONTENT_DIRECTORY: \"%s\"",
				settings->paths.path_content_directory);

			return true;
		}
		case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
		{
			const char** directory = (const char**)data;

			if (directory)
				*directory = system->core->save_directory;

			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_SAVE_DIRECTORY: \"%s\"",
				system->core->save_directory);

			return true;
		}
		case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_SYSTEM_AV_INFO: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_PROC_ADDRESS_CALLBACK: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_SUBSYSTEM_INFO: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
		{
			const struct retro_controller_info* info = (const struct retro_controller_info*)data;

			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CONTROLLER_INFO:");

			if (info)
			{
				unsigned port = 0;
				while (info[port].types != NULL && port < MAX_PLAYERS)
				{
					unsigned i;
					unsigned default_id   = RETRO_DEVICE_NONE;
					const char* default_desc = "None";

					for (i = 0; i < info[port].num_types; i++)
					{
						unsigned type_id      = info[port].types[i].id;
						const char* type_desc = info[port].types[i].desc;

						if (legacy_machine->log_level == LMC_LOG_VERBOSE)
							printf("\tPort %u, type %u: \"%s\" (0x%04x)\n",
								port, i, type_desc, type_id);

						/* Use the first non-NONE type as the default. */
						if (default_id == RETRO_DEVICE_NONE && type_id != RETRO_DEVICE_NONE)
						{
							default_id   = type_id;
							default_desc = type_desc;
						}
					}

					system->core->port_devices[port] = default_id;

					lmc_core_log(RETRO_LOG_INFO, "\tPort %u default: \"%s\" (0x%04x)",
						port, default_desc, default_id);

					port++;
				}
				system->core->port_count = port;
			}

			return true;
		}
		case RETRO_ENVIRONMENT_SET_MEMORY_MAPS:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_MEMORY_MAPS: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_GEOMETRY:
		{
			const struct retro_game_geometry* geometry = (const struct retro_game_geometry*)data;

			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_GEOMETRY: Dimensions: %ux%u, Aspect: %.3f",
				geometry->base_width, geometry->base_height, geometry->aspect_ratio);

			SetVideoGeometry(geometry);

			return true;
		}
		case RETRO_ENVIRONMENT_GET_USERNAME:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_USERNAME: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_LANGUAGE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_LANGUAGE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_CURRENT_SOFTWARE_FRAMEBUFFER: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_HW_RENDER_INTERFACE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_SUPPORT_ACHIEVEMENTS: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_SERIALIZATION_QUIRKS: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_HW_SHARED_CONTEXT: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_VFS_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_VFS_INTERFACE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_LED_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_LED_INTERFACE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE:
		{
			int* value = (int*)data;
			*value = 1 << 0 | 1 << 1;
			return true;
		}
		case RETRO_ENVIRONMENT_GET_MIDI_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_MIDI_INTERFACE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_FASTFORWARDING:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_FASTFORWARDING: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_TARGET_REFRESH_RATE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
		{
			bool* value = (bool*)data;

			if (value)
				*value = true;

			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_INPUT_BITMASKS: Supported");
			return true;
		}
		case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
		{
			/* libretro core options API version 1. */
			unsigned options_version = 1;
			unsigned* value = (unsigned*)data;

			if (value)
				*value = options_version;

			lmc_core_log(RETRO_LOG_INFO,
				"[Environment]: GET_CORE_OPTIONS_VERSION: Core option API v%u",
				options_version);

			return true;
		}
		case RETRO_ENVIRONMENT_SET_CORE_OPTIONS:
		{
			const struct retro_core_option_definition* options =
				(const struct retro_core_option_definition*)data;

			DeinitializeOptions();

			if (options)
			{
				lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CORE_OPTIONS");

				InitializeOptions(system->core->options_path,
					system->core->name, options);
			}

			return true;
		}
		case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL:
		{
			const struct retro_core_options_intl* options =
				(const struct retro_core_options_intl*)data;

			if (options && options->us)
			{
				DeinitializeOptions();

				lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CORE_OPTIONS_INTL");

				InitializeOptions(system->core->options_path,
					system->core->name, options->us);
			}

			return true;
		}
		case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY:
		{
			const struct retro_core_option_display* option_display =
				(const struct retro_core_option_display*)data;

			if (option_display)
				SetOptionVisibility(option_display->key, option_display->visible);

			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CORE_OPTIONS_DISPLAY");

			return true;
		}
		case RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_PREFERRED_HW_RENDER: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION:
		{
			/* libretro disk control interface version 1. */
			unsigned disk_control_version = 1;
			unsigned* value = (unsigned*)data;

			if (value)
				*value = disk_control_version;

			lmc_core_log(RETRO_LOG_INFO,
				"[Environment]: GET_DISK_CONTROL_INTERFACE_VERSION: Disk control API v%u",
				disk_control_version);

			return true;
		}
		case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_DISK_CONTROL_EXT_INTERFACE");
			const struct retro_disk_control_ext_callback* disk_control_ext_cb =
				(const struct retro_disk_control_ext_callback*)data;

			if (disk_control_ext_cb)
			{
				memcpy(&system->cb_disk_control, disk_control_ext_cb,
					sizeof(struct retro_disk_control_ext_callback));
			}
			return true;
		}
		case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_MESSAGE_INTERFACE_VERSION: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_MESSAGE_EXT:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_MESSAGE_EXT");
			const struct retro_message_ext* message_ext = (const struct retro_message_ext*)data;

			if (message_ext)
			{
				lmc_core_log(message_ext->level, "%s\n", message_ext->msg);
			}
			return true;
		}
		case RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_INPUT_MAX_USERS: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_AUDIO_BUFFER_STATUS_CALLBACK");
			const struct retro_audio_buffer_status_callback* buffer_status_cb =
				(const struct retro_audio_buffer_status_callback*)data;

			if (buffer_status_cb)
			{
				memset(&system->cb_audio_buffer_status, 0, 
					sizeof(struct retro_audio_buffer_status_callback));
				memcpy(&system->cb_audio_buffer_status, buffer_status_cb, 
					sizeof(struct retro_audio_buffer_status_callback));
			}

			return true;
		}
		case RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_MINIMUM_AUDIO_LATENCY:");
			const unsigned* audio_latency = (const unsigned*)data;

			if (audio_latency)
			{
				unsigned new_latency = *audio_latency;

				if (new_latency > MAX_LATENCY)
				{
					new_latency = MAX_LATENCY;

					if (legacy_machine->log_level == LMC_LOG_VERBOSE)
					{
						printf("\tAudio latency change to %dms is above maximum latency of %dms.\n",
							audio_latency, MAX_LATENCY);
					}
				}

				if (new_latency < audio->output.minimum_latency)
				{
					new_latency = audio->output.minimum_latency;

					if (legacy_machine->log_level == LMC_LOG_VERBOSE)
					{
						printf("\tAudio latency change to %dms is below minimum latency of %dms.\n",
							audio_latency, audio->output.minimum_latency);
					}
				}

				if (audio->output.latency != new_latency)
				{
					SetAudioLatency(new_latency);
				}

				if (legacy_machine->log_level == LMC_LOG_VERBOSE)
				{
					printf("\tAudio latency set to %dms.\n", audio->output.latency);
				}
			}

			return true;
		}
		case RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_FASTFORWARDING_OVERRIDE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CONTENT_INFO_OVERRIDE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_GAME_INFO_EXT:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_GAME_INFO_EXT: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CORE_OPTIONS_V2: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CORE_OPTIONS_V2_INTL: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_SET_VARIABLE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_VARIABLE: not implemented");
			return false;
		}
		case RETRO_ENVIRONMENT_GET_THROTTLE_STATE:
		{
			lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_THROTTLE_STATE: not implemented");
			return false;
		}
		default:
		{
			lmc_core_log(RETRO_LOG_DEBUG, "[Environment]: Unhandled event: #%u", command);
			return false;
		}
	}
	return true;
}

static bool InitializeCore(void)
{
	SystemManager* system = GetSystemManagerContext();
	SettingsManager* settings = GetSettingsManagerContext();
	struct retro_system_info system_info = { 0 };
	char* state_path = (char*)malloc(PATH_MAX_LENGTH);

	if (!state_path)
	{
		lmc_core_log(RETRO_LOG_ERROR, 
			"[Core]: Failed to allocate memory to resolve state path information");
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		return false;
	}

	/* Assert required variables have been set. */
	retro_assert(system->core);
	retro_assert(strlen(system->core->file_path) > 1);
	retro_assert(system->content);

	/* libretro callbacks. */
	void (*SetEnvironment)(retro_environment_t) = NULL;
	void (*SetVideoRefresh)(retro_video_refresh_t) = NULL;
	void (*SetInputPoll)(retro_input_poll_t) = NULL;
	void (*SetInputState)(retro_input_state_t) = NULL;
	void (*SetAudioSample)(retro_audio_sample_t) = NULL;
	void (*SetAudioSampleBatch)(retro_audio_sample_batch_t) = NULL;

	/* Load libretro core library. */
	system->core->library->handle = dylib_load(system->core->file_path);

	if (!system->core->library->handle)
	{
		lmc_core_log(RETRO_LOG_ERROR, "[Core]: Failed to load core: %s", dylib_error());

		if (state_path)
			free(state_path);

		return false;
	}

	/* Initialize libretro core symbols. */
	LoadRetroSymbol(retro_init);
	LoadRetroSymbol(retro_deinit);
	LoadRetroSymbol(retro_api_version);
	LoadRetroSymbol(retro_get_system_info);
	LoadRetroSymbol(retro_get_system_av_info);
	LoadRetroSymbol(retro_set_environment);
	LoadRetroSymbol(retro_set_video_refresh);
	LoadRetroSymbol(retro_set_audio_sample);
	LoadRetroSymbol(retro_set_audio_sample_batch);
	LoadRetroSymbol(retro_set_input_poll);
	LoadRetroSymbol(retro_set_input_state);
	LoadRetroSymbol(retro_set_controller_port_device);
	LoadRetroSymbol(retro_reset);
	LoadRetroSymbol(retro_run);
	LoadRetroSymbol(retro_serialize_size);
	LoadRetroSymbol(retro_serialize);
	LoadRetroSymbol(retro_unserialize);
	LoadRetroSymbol(retro_cheat_reset);
	LoadRetroSymbol(retro_cheat_set);
	LoadRetroSymbol(retro_load_game);
	LoadRetroSymbol(retro_load_game_special);
	LoadRetroSymbol(retro_unload_game);
	LoadRetroSymbol(retro_get_region);
	LoadRetroSymbol(retro_get_memory_data);
	LoadRetroSymbol(retro_get_memory_size);

	LoadSymbol(SetEnvironment, retro_set_environment);
	LoadSymbol(SetVideoRefresh, retro_set_video_refresh);
	LoadSymbol(SetInputPoll, retro_set_input_poll);
	LoadSymbol(SetInputState, retro_set_input_state);
	LoadSymbol(SetAudioSample, retro_set_audio_sample);
	LoadSymbol(SetAudioSampleBatch, retro_set_audio_sample_batch);

	/* Safety check to satisfy compiler. */
	if (!SetEnvironment ||
		!SetVideoRefresh ||
		!SetInputPoll ||
		!SetInputState ||
		!SetAudioSample ||
		!SetAudioSampleBatch)
	{
		lmc_core_log(RETRO_LOG_ERROR, "[Core]: Failed to bind core symbols.");

		if (state_path)
			free(state_path);

		return false;
	}

	system->core->library->retro_get_system_info(&system_info);

	/* Store core library file name. */
	fill_pathname_base_noext(system->core->file_name, 
		system->core->file_path, 
		NAME_MAX_LENGTH);

	/* Build and store path to core options configuration directory. */
	fill_pathname_join(settings->paths.path_option_directory, 
		settings->paths.path_setting_directory, 
		system_info.library_name, 
		PATH_MAX_LENGTH);

	/* Store system information. */
	strlcpy(system->core->name, system_info.library_name, NAME_MAX_LENGTH);
	strlcpy(system->core->version, system_info.library_version, NAME_MAX_LENGTH);
	strlcpy(system->core->extensions, system_info.valid_extensions, NAME_MAX_LENGTH);
	system->core->need_fullpath = system_info.need_fullpath;
	system->core->block_extract = system_info.block_extract;

	/* Build and store path to content specific core options configuration file only if
	   content is present. */
	if (!system->core->contentless)
	{
		/* Store content file name and build content-specific options directory path. */
		fill_pathname_base_noext(system->content->file_name, 
			system->content->file_path, 
			NAME_MAX_LENGTH);
		fill_pathname_join(settings->paths.path_option_content_directory, 
			settings->paths.path_option_directory, 
			system->content->file_name, 
			PATH_MAX_LENGTH);
	}

	/* Create base path to core options, if it doesn't already exist. */
	if (!path_is_directory(settings->paths.path_option_directory))
		path_mkdir(settings->paths.path_option_directory);

	/* Determine whether path to core options file is core specific or content specific. */
	if (path_is_directory(settings->paths.path_option_content_directory))
		strlcpy(system->core->options_path, 
			settings->paths.path_option_content_directory, 
			PATH_MAX_LENGTH);
	else
		strlcpy(system->core->options_path, 
			settings->paths.path_option_directory, 
			PATH_MAX_LENGTH);

	fill_pathname_join(state_path, 
		settings->paths.path_state_directory,
		system->core->contentless ? system->core->file_name : system->content->file_name,
		PATH_MAX_LENGTH);

	size_t state_path_size = strlen(state_path) + strlen(system->core->name) + 8;
	system->core->state_paths = (char**)malloc(MAX_STATE_SLOTS * sizeof(char*));

	if (!system->core->state_paths)
	{
		lmc_core_log(RETRO_LOG_ERROR, "[Core]: Failed to allocate memory for state save paths.");
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		if (state_path)
			free(state_path);
		return false;
	}

	/* Allocate and build individual state save paths. */
	for (LMC_Slot slot = LMC_SLOT_QUICK; slot < MAX_STATE_SLOTS; slot++)
	{
		system->core->state_paths[slot] = (char*)malloc(state_path_size);
		if (system->core->state_paths[slot])
		{
			system->core->state_paths[slot][0] = '\0';
		}
		else
		{
			lmc_core_log(RETRO_LOG_ERROR, "[Core]: Failed to allocate memory for state save paths.");
			LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);

			if (state_path)
				free(state_path);

			return false;
		}
		snprintf(system->core->state_paths[slot], state_path_size,
			"%s (%s).st%d", state_path, system->core->name, slot);
	}

	/* Resolve core specific save directory. */
	fill_pathname_join(system->core->save_directory,
		settings->paths.path_save_directory,
		system->core->name,
		PATH_MAX_LENGTH);

	/* Create core specific save directory, if it doesn't already exist. */
	if (!path_is_directory(system->core->save_directory))
		path_mkdir(system->core->save_directory);

	/* Resolve core/content specific SRAM and RTC save paths. */
	const char* save_name =
		system->core->contentless ? system->core->name : system->content->file_name;

	fill_pathname_join_concat(system->core->save_path,
		system->core->save_directory,
		save_name,
		".srm",
		PATH_MAX_LENGTH);

	fill_pathname_join_concat(system->core->clock_path,
		system->core->save_directory,
		save_name,
		".rtc",
		PATH_MAX_LENGTH);

	/* Initialize primary libretro callbacks. */
	SetEnvironment(CoreEnvironment);
	SetVideoRefresh(RefreshVideo);
	SetInputPoll(PollInput);
	SetInputState(InputState);
	SetAudioSample(WriteAudioSample);
	SetAudioSampleBatch(WriteAudioSampleBatch);

	/* Initialize libretro core. */
	system->core->library->retro_init();
	system->core->library->initialized = true;

	lmc_core_log(RETRO_LOG_INFO, "[Core]: %s core loaded successfully", system->core->file_name);

	free(state_path);

	return true;
}

static bool InitializeContent(void)
{
	SystemManager* system = GetSystemManagerContext();
	SettingsManager* settings = GetSettingsManagerContext();

	if (system->core->contentless && !system->core->library->supports_no_game)
	{
		lmc_core_log(RETRO_LOG_ERROR, "[Content]: Core requires content but none was provided");
		return false;
	}

	if (!system->core->contentless)
	{
#if defined HAVE_COMPRESSION
		system->content->archived = path_is_compressed_file(system->content->file_path);

		if (!system->core->block_extract && system->content->archived)
		{
			/* Get internal archive file list. */
			struct string_list* archive_list = 
				file_archive_get_file_list(system->content->file_path, system->core->extensions);

			if (!archive_list)
			{
				lmc_core_log(RETRO_LOG_ERROR, 
					"[Content]: Failed to allocate memory for archive's file list");
				return false;
			}

			if (archive_list && (archive_list->size > 0))
			{
				const char* archive_file = NULL;

				/* Sort list alphabetically. */
				if (archive_list->size > 1)
					dir_list_sort(archive_list, true);

				archive_file = archive_list->elems[0].data;

				if (!string_is_empty(archive_file))
				{
					/* Build archive file path. */
					snprintf(system->content->archive_path, PATH_MAX_LENGTH,
						"%s#%s", system->content->file_path, archive_file);

					system->content->extract = true;
				}
				else
				{
					lmc_core_log(RETRO_LOG_ERROR,
						"[Content]: Couldn't find any files with a valid extension inside archive");
					return false;
				}
			}

			string_list_free(archive_list);
		}
#endif
		if (system->core->need_fullpath)
		{
#if defined HAVE_COMPRESSION
			if (system->content->archived && system->content->extract)
			{
				lmc_core_log(RETRO_LOG_INFO,
					"[Content]: Core requires uncompressed content. Extracting archive to temporary directory.");

				if (!file_archive_extract_file(system->content->archive_path,
					system->core->extensions, settings->paths.path_temp_directory,
					system->content->temp_path, PATH_MAX_LENGTH))
				{
					lmc_core_log(RETRO_LOG_ERROR,
						"[Content]: Failed to extract archived content from: \"%s\"",
						system->content->file_path);
					return false;
				}

				lmc_core_log(RETRO_LOG_INFO,
					"[Content]: Archive successfully extracted to: \"%s\"",
					system->content->temp_path);
			}
			else if (!path_is_valid(system->content->file_path))
#else
			if (!path_is_valid(system->content->file_path))
#endif
			{
				if (system->core->library->supports_no_game)
				{
					lmc_core_log(RETRO_LOG_WARN,
						"[Content]: Content file not found: \"%s\". Core supports running without content.",
						system->content->file_path);
					system->core->contentless = true;
					return true;
				}
				lmc_core_log(RETRO_LOG_ERROR,
					"[Content]: Content file not found: \"%s\"",
					system->content->file_path);
				return false;
			}
		}
		else
		{
			lmc_core_log(RETRO_LOG_INFO,
				"[Content]: Reading content: \"%s\"", system->content->file_path);

#if defined HAVE_COMPRESSION
			if (system->content->archived)
			{
				if (!file_archive_compressed_read(system->content->archive_path,
					(void**)&system->content->data, NULL, &system->content->size))
				{
					lmc_core_log(RETRO_LOG_ERROR,
						"[Content]: Failed to read archived content file: \"%s\"",
						system->content->file_path);
					return false;
				}
			}
			else
			{
#endif
				if (!filestream_read_file(system->content->file_path,
					(void**)&system->content->data, &system->content->size))
				{
					lmc_core_log(RETRO_LOG_ERROR,
						"[Content]: Failed to read content file: \"%s\"",
						system->content->file_path);
					return false;
				}
#if defined HAVE_COMPRESSION
			}
#endif
			if (system->content->size < 1)
			{
				lmc_core_log(RETRO_LOG_ERROR, "[Content]: Failed to query content file size");
				return false;
			}
		}
	}

	return true;
}

static bool LoadContent(void)
{
	SystemManager* system = GetSystemManagerContext();
	struct retro_game_info content_info = { 0 };

	/* Resolve path to content. */
	if (!system->core->contentless)
	{
		if (system->core->need_fullpath && system->content->archived && system->content->extract)
		{
			content_info.path = strdup(system->content->temp_path);
		}
		else if (system->content->archived)
		{
			content_info.path = strdup(system->content->archive_path);
		}
		else
		{
			content_info.path = strdup(system->content->file_path);
		}
	}

	/* Initialize remaining content info variables. */
	if (system->content->data)
		content_info.data = system->content->data;
	content_info.size = system->content->size;
	content_info.meta = "";

	/* Load core content. contentless flag already accounts for supports_no_game
	   validation performed in InitializeContent. */
	if (!system->core->contentless)
	{
		if (!system->core->library->retro_load_game(&content_info))
		{
			lmc_core_log(RETRO_LOG_ERROR,
				"[Core]: The core failed to load the content");
			return false;
		}
	}
	else
	{
		if (!system->core->library->retro_load_game(NULL))
		{
			lmc_core_log(RETRO_LOG_ERROR,
				"[Core]: The core failed to load without content");
			return false;
		}
	}

	lmc_core_log(RETRO_LOG_INFO, "[Core]: Content loaded successfully");

	/* If reading save files fails, proceed with a warning. */
	if (!ReadSaveFile(system->core->save_path, "SRAM", RETRO_MEMORY_SAVE_RAM))
	{
		lmc_core_log(RETRO_LOG_WARN,
			"[SRAM]: Failed to read SRAM save file: \"%s\"",
			system->core->save_path);
	}
	if (!ReadSaveFile(system->core->clock_path, "RTC", RETRO_MEMORY_RTC))
	{
		lmc_core_log(RETRO_LOG_WARN,
			"[RTC]: Failed to read RTC save file: \"%s\"",
			system->core->clock_path);
	}

	return true;
}

/**************************************************************************************************
 * SystemManager Functions
 *************************************************************************************************/

/* Returns the current system manager context. */
SystemManager* GetSystemManagerContext(void)
{
	return &system_manager;
}

/* Initializes Libretro library subsystems. */
void InitializeLibretro(bool threaded)
{
	rtime_init();

	task_queue_deinit();
	task_queue_init(threaded, PushMessageQueueTask);
}

/* Initializes the system manager context. */
bool InitializeSystem(void)
{
	SystemManager* system = GetSystemManagerContext();

	if (system->initialized)
		DeinitializeSystem();

	system->cb_get_time_elapsed = GetTimeElapsed;

	system->core = (CoreManager*)calloc(sizeof(CoreManager), 1);
	if (!system->core)
	{
		DeinitializeSystem();
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		return false;
	}
	system->core->library = (CoreLibrary*)calloc(sizeof(CoreLibrary), 1);
	if (!system->core->library)
	{
		DeinitializeSystem();
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		return false;
	}
	system->core->options = GetOptionsManagerContext();
	if (!system->core->options)
	{
		DeinitializeSystem();
		LMC_SetLastError(LMC_ERR_NULL_POINTER);
		return false;
	}

	/* Default all ports to RETRO_DEVICE_JOYPAD. SET_CONTROLLER_INFO may override these. */
	for (unsigned i = 0; i < MAX_PLAYERS; i++)
		system->core->port_devices[i] = RETRO_DEVICE_JOYPAD;
	system->core->port_count = MAX_PLAYERS;
	system->content = (ContentManager*)calloc(sizeof(ContentManager), 1);
	if (!system->content)
	{
		DeinitializeSystem();
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		return false;
	}

	system->initialized = true;

	return true;
}

/* Deinitializes the system manager context. */
void DeinitializeSystem(void)
{
	SystemManager* system = GetSystemManagerContext();

	if (system->core)
	{
		if (system->core->library)
			free(system->core->library);

		free(system->core);
	}

	if (system->content)
		free(system->content);

	memset(system, 0, sizeof(SystemManager));
}

/* Initialize and run the libretro core with content (if present). */
bool RunCore(void)
{
	SystemManager* system = GetSystemManagerContext();
	WindowManager* window = GetWindowManagerContext();
	AudioManager* audio = GetAudioManagerContext();
	VideoManager* video = GetVideoManagerContext();

	if (!InitializeCore())
	{
		lmc_core_log(RETRO_LOG_ERROR, "[Core]: Failed to initialize libretro core");
		return false;
	}

	if (!InitializeContent())
	{
		lmc_core_log(RETRO_LOG_ERROR, "[Content]: Failed to initialize core content");
		return false;
	}

	if (!LoadContent())
	{
		lmc_core_log(RETRO_LOG_ERROR, "[Content]: Failed to load core content");
		return false;
	}

	/* Pre-initialize hw render callbacks. */
	system->cb_hw_render.context_reset = empty_function;
	system->cb_hw_render.context_destroy = empty_function;

	/* Initialize libretro audio/video output. */
	system->core->library->retro_get_system_av_info(&system->av_info);

	SetVideoGeometry(&system->av_info.geometry);

	if (!window->initialized)
		InitializeWindow(WINDOW_DRIVER_FIRST);

	InitializeAudio(AUDIO_DRIVER_FIRST, system->av_info.timing.sample_rate);

	/* Set controller device type for each port. */
	if (system->core->library->retro_set_controller_port_device)
	{
		for (unsigned port = 0; port < system->core->port_count; port++)
			system->core->library->retro_set_controller_port_device(port, system->core->port_devices[port]);
	}

	system->running = true;

	lmc_core_log(RETRO_LOG_INFO, "[Core]: Running core...");

	return true;
}

/* Set the controller device type for a given port. */
bool SetControllerPortDevice(unsigned port, unsigned device)
{
	SystemManager* system = GetSystemManagerContext();

	if (port >= MAX_PLAYERS)
		return false;

	if (!system->core)
		return false;

	system->core->port_devices[port] = device;

	if (system->running && system->core->library->retro_set_controller_port_device)
	{
		system->core->library->retro_set_controller_port_device(port, device);
		lmc_core_log(RETRO_LOG_INFO,
			"[Core]: Port %u device type set to 0x%04x", port, device);
	}

	return true;
}

/* Update audio settings and reinitialize the audio driver. */
void UpdateAudio(void)
{
	AudioManager* audio = GetAudioManagerContext();

	if (audio->reinitialize)
	{
		lmc_trace(LMC_LOG_VERBOSE, "[Audio]: Latency changed, reinitializing audio driver...");

		ReinitializeAudio();
	}
}

/* Reads a save file from disk into the core's memory. */
bool ReadSaveFile(const char* path, const char* save_type, unsigned save_type_id)
{
	SystemManager* system = GetSystemManagerContext();
	int64_t file_size;
	void* file_buffer;
	size_t save_size;
	void* save_data;

	if (!path_is_valid(path))
		return true;

	save_size = system->core->library->retro_get_memory_size(save_type_id);
	if (!save_size)
		return true;

	save_data = system->core->library->retro_get_memory_data(save_type_id);

#if defined(HAVE_COMPRESSION) && defined(HAVE_ZLIB)
	if (!rzipstream_read_file(path, &file_buffer, &file_size))
#else
	if (!filestream_read_file(path, &file_buffer, &file_size))
#endif
		return false;

	if (file_size > 0)
	{
		if (file_size > save_size)
		{
			lmc_core_log(RETRO_LOG_WARN,
				"[%s]: %s file is larger than expected. Truncating from %u bytes to %u bytes.",
				save_type, save_type, (unsigned)file_size, (unsigned)save_size);
			file_size = save_size;
		}
		memcpy(save_data, file_buffer, (size_t)file_size);
	}

	if (file_buffer)
		free(file_buffer);

	lmc_core_log(RETRO_LOG_INFO,
		"[%s]: Loaded %u bytes of %s data.",
		save_type, (unsigned)save_size, save_type);

	return true;
}

/* Writes the core's memory save data to a file on disk. */
bool WriteSaveFile(const char* path, const char* save_type, unsigned save_type_id)
{
	SystemManager* system = GetSystemManagerContext();
	void* save_data;
	size_t save_size;

	save_size = system->core->library->retro_get_memory_size(save_type_id);
	if (!save_size)
		return true;

	save_data = system->core->library->retro_get_memory_data(save_type_id);

#if defined(HAVE_COMPRESSION) && defined(HAVE_ZLIB)
	if (system->compress_saves)
	{
		if (!rzipstream_write_file(path, save_data, save_size))
			return false;
	}
	else
#endif
	{
		if (!filestream_write_file(path, save_data, save_size))
			return false;
	}

	lmc_core_log(RETRO_LOG_INFO,
		"[%s]: Saved %u bytes of %s data.",
		save_type, (unsigned)save_size, save_type);

	return true;
}
