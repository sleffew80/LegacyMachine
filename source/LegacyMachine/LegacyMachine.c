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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <features/features_cpu.h>
#include <file/file_path.h>
#include <streams/file_stream.h>
#include <dynamic/dylib.h>
#include <compat/strl.h>

#include "LegacyMachine.h"
#include "MainEngine.h"
#include "Logging.h"
#include "Common/Common.h"

/**************************************************************************************************
 * Definitions
 *************************************************************************************************/

/* Magic number to recognize context object. */
#define RETRO_CONTEXT	0x00A4CADE

/**************************************************************************************************
 * Macro Definitions
 *************************************************************************************************/
#define LoadSymbol(V, S) do { \
   function_t func = dylib_proc(legacy_machine->system->current_core->handle, #S); \
   memcpy(&V, &func, sizeof(func)); \
   if (!V) { lmc_core_log(RETRO_LOG_ERROR, "Failed to load symbol: \"%s\"\n", #S); } \
} while (0)

#define LoadRetroSymbol(S) LoadSymbol(legacy_machine->system->current_core->S, S)

/**************************************************************************************************
 * LegacyMachine Initialization/Deinitialization
 *************************************************************************************************/
LMC_Engine					  legacy_machine;					/* LegacyMachine context. */

static struct retro_variable* retro_variables = NULL;		/* LibRetro core variables. */
#if defined HAVE_MENU
/*!
 * \brief
 * Initializes the legacy machine.
 *
 * \param program_name
 * Internal name of the program being created. Used as default window title. Also, used as base
 * directory name in environments where it is required.
 * 
 * \param base_width
 * Frontend menu's horizontal resolution in pixels.
 *
 * \param base_height
 * Frontend menu's vertical resolution in pixels.
 *
 * \param max_width
 * Framebuffer's maximum horizontal resolution in pixels.
 *
 * \param max_height
 * Framebuffer's maximum vertical resolution in pixels.
 *
 * \param aspect_ratio
 * Frontend menu's aspect ratio.
 *
 * \param fps
 * Frontend menu's refresh rate in frames per second.
 *
 * \param layers
 * Frontend menu's number of supported layers.
 * 
 * \param sprites
 * Frontend menu's number of supported sprites.
 * 
 * \param animations
 * Frontend menu's number of supported palette animation slots.
 * 
 * \remarks
 * Performs initialization of the main engine and it's drivers. Set all values to
 * NULL if not using a frontend menu - video will then be initialized from the core's
 * geometry.
 */
LMC_Engine LMC_Init(const char* program_name,
	int base_width, int base_height, int max_width, int max_height,
	float aspect_ratio, double fps, int layers, int sprites, int animations)
#else
/*!
 * \brief
 * Initializes the legacy machine.
 * 
 * \remarks
 * Performs initialization of the main engine and it's drivers.
 */
LMC_Engine LMC_Init(void)
#endif
{
	printf("LegacyMachine v%d.%d.%d %d-bit built %s %s\nLibRetro v%u\n",
		LEGACY_MACHINE_VER_MAJ,
		LEGACY_MACHINE_VER_MIN,
		LEGACY_MACHINE_VER_REV,
		(int)(sizeof(UINTPTR_MAX) << 3), __DATE__, __TIME__,
		RETRO_API_VERSION);

	/* Initialize LegacyMachine context. */
	LMC_Engine context;
	context = (LMC_Engine)calloc(sizeof(MainEngine), 1);
	context->header = RETRO_CONTEXT;

	/* Initialize settings manager. */
	context->settings = GetSettingsManagerContext();
	if (!context->settings)
	{
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_NULL_POINTER);
		return NULL;
	}
	/* Initialize platform manager. */
	context->platform = InitializePlatformDriver();
	if (!context->platform)
	{
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_NULL_POINTER);
		return NULL;
	}
	/* Initialize window manager. */
	context->window = InitializeWindowDriver();
	if (!context->window)
	{
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_NULL_POINTER);
		return NULL;
	}
	/* Initialize video driver. */
	context->video = InitializeVideoDriver();
	if (!context->video)
	{
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_NULL_POINTER);
		return NULL;
	}
	/* Initialize audio driver. */
	context->audio = InitializeAudioDriver();
	if (!context->audio)
	{
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_NULL_POINTER);
		return NULL;
	}
	/* Initialize input driver. */
	context->input = InitializeInputDriver();
	if (!context->input)
	{
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_NULL_POINTER);
		return NULL;
	}
#ifdef HAVE_MENU
	/* Initialize menu manager. */
	context->menu = GetMenuManagerContext();
	if (!context->menu)
	{
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_NULL_POINTER);
		return NULL;
	}
	/* Initialize menu geometry and check for sane dimension values. */
	if (base_width >= 8)
	{
		context->menu->av_info.geometry.base_width = base_width;
	}
	else
	{
		lmc_trace(LMC_LOG_ERRORS,
			"base_width parameter must be greater than or equal to 8");
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_INV_PARAM);
		return NULL;
	}
	if (base_height >= 8)
	{
		context->menu->av_info.geometry.base_height = base_height;
	}
	else
	{
		lmc_trace(LMC_LOG_ERRORS,
			"base_height parameter must be greater than or equal to 8");
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_INV_PARAM);
		return NULL;
	}
	if (max_width >= base_width)
	{
		context->menu->av_info.geometry.max_width = max_width;
	}
	else
	{
		lmc_trace(LMC_LOG_ERRORS,
			"max_width parameter must be greater than or equal to base_width");
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_INV_PARAM);
		return NULL;
	}
	if (max_height >= base_height)
	{
		context->menu->av_info.geometry.max_height = max_height;
	}
	else
	{
		lmc_trace(LMC_LOG_ERRORS,
			"max_height parameter must be greater than or equal to base_height");
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_INV_PARAM);
		return NULL;
	}
	if (aspect_ratio > 0.24f)
	{
		context->menu->av_info.geometry.aspect_ratio = aspect_ratio;
	}
	else
	{
		context->menu->av_info.geometry.aspect_ratio = 0.25f;
	}
	if (fps > 0.0)
	{
		context->menu->av_info.timing.fps = fps;
	}
	else
	{
		lmc_trace(LMC_LOG_ERRORS,
			"fps parameter must be a positive double");
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_INV_PARAM);
		return NULL;
	}
	context->menu->frame_delay = 1000 / fps;
	context->menu->tile_engine = TLN_Init(base_width, base_height, layers, sprites, animations);
	if (!context->menu->tile_engine)
	{
		lmc_trace(LMC_LOG_ERRORS, TLN_GetErrorString(TLN_GetLastError()));
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_TILENGINE);
		return NULL;
	}
	context->menu->frame.pitch = (((max_width * 32) >> 3) + 3) & ~0x03;
	context->menu->frame.data = malloc(context->menu->frame.pitch * max_height);
	context->menu->available = true;
#endif
	context->system = GetSystemManagerContext();
	if (!context->system)
	{
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_NULL_POINTER);
		return NULL;
	}
	context->system->current_core = (CoreLibrary*)calloc(sizeof(CoreLibrary), 1);
	if (!context->system->current_core)
	{
		LMC_DeleteContext(context);
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		return NULL;
	}

	/* Set internal program name (required for environment initialization). */
	strlcpy(context->settings->program_name, program_name, NAME_MAX_LENGTH);

	/* Set as default context if it's the first one. */
	if (legacy_machine == NULL)
		legacy_machine = context;

	/* Get environment and initialize platform dependent code. */
	context->platform->cb_get_env();

	/* Create required directories if they don't already exist. */
	if (!path_is_directory(context->settings->setting_directory))
		path_mkdir(context->settings->setting_directory);
	if (!path_is_directory(context->settings->core_directory))
		path_mkdir(context->settings->core_directory);
	if (!path_is_directory(context->settings->content_directory))
		path_mkdir(context->settings->content_directory);
	if (!path_is_directory(context->settings->system_directory))
		path_mkdir(context->settings->system_directory);
	if (!path_is_directory(context->settings->save_directory))
		path_mkdir(context->settings->save_directory);
	if (!path_is_directory(context->settings->state_directory))
		path_mkdir(context->settings->state_directory);

#ifdef HAVE_MENU
	/* Additional Tilengine initialization. */
	TLN_SetTargetFps((int)fps);
	TLN_SetLoadPath(context->settings->asset_directory);
	TLN_SetRenderTarget(context->menu->frame.data, context->menu->frame.pitch);
#endif

	VideoInfo* video_info = GetVideoInfo();
	video_info->frame = calloc(sizeof(FrameInfo), 1);

#ifdef _DEBUG
	LMC_SetLogLevel(LMC_LOG_ERRORS);
#endif

	LMC_SetLastError(LMC_ERR_OK);

	return context;
}

/* Checks current engine context and verifies whether it is NULL. */
static bool check_context(LMC_Engine context)
{
	if (context != NULL)
	{
		if (context->header == RETRO_CONTEXT)
			return true;
	}
	return false;
}

/*!
 * \brief
 * Sets current engine context.
 *
 * \param context
 * LMC_Engine object to set as current context, returned by LMC_Init().
 *
 * \returns
 * True if success or false if wrong context is supplied.
 *
 */
bool LMC_SetContext(LMC_Engine context)
{
	if (check_context(context))
	{
		legacy_machine = context;
		LMC_SetLastError(LMC_ERR_OK);
		return true;
	}
	else
	{
		LMC_SetLastError(LMC_ERR_NULL_POINTER);
		return false;
	}
}

/*!
 * \brief
 * Returns the current engine context.
 *
 */
LMC_Engine LMC_GetContext(void)
{
	return legacy_machine;
}

/*!
 * \brief
 * Deinitialises current engine context and frees used resources.
 *
 */
void LMC_Deinit(void)
{
	if (legacy_machine != NULL)
	{
		LMC_DeleteContext(legacy_machine);
		legacy_machine = NULL;
	}
}

/*!
 * \brief
 * Deletes explicit context.
 *
 * \param context
 * Context reference to delete.
 *
 * \returns
 * True if success or false if an error occurs.
 * 
 */
bool LMC_DeleteContext(LMC_Engine context)
{
	if (!check_context(context))
	{
		LMC_SetLastError(LMC_ERR_NULL_POINTER);
		return false;
	}

	/* Free "engine" members. */
	if (context->system->current_core)
		free(context->system->current_core);
#ifdef HAVE_MENU
	if (context->menu->tile_engine)
		TLN_Deinit();
	if (context->menu->frame.data)
		free(context->menu->frame.data);
	if (context->menu)
		context->menu = NULL;
#endif
	if (context->video->info.frame)
		free(context->video->info.frame);
	if (context->system)
		context->system = NULL;
	if (context->input)
		context->input = NULL;
	if (context->audio)
		context->audio = NULL;
	if (context->video)
		context->video = NULL;
	if (context->window)
		context->window = NULL;
	if (context->platform)
		context->platform = NULL;

	free(context);
	return true;
}

/*!
 * \brief
 * Retrieves LegacyMachine library version.
 *
 * \returns
 * Returns a 32-bit integer containing three packed numbers:
 * bits 23:16 -> major version,
 * bits 15: 8 -> minor version,
 * bits  7: 0 -> bugfix revision
 *
 */
uint32_t LMC_GetVersion(void)
{
	LMC_SetLastError(LMC_ERR_OK);
	return LEGACY_MACHINE_HEADER_VERSION;
}

/**************************************************************************************************
 * LegacyMachine Menu Accessor Functions
 *************************************************************************************************/

#ifdef HAVE_MENU

/*!
 * \brief
 * Gets the location of the frontend menu's render target.
 *
 * \returns
 * Pointer to frontend menu's render target.
 * 
 */
uint8_t* LMC_GetMenuRenderTarget(void)
{
	LMC_SetLastError(LMC_ERR_OK);
	return legacy_machine->menu->frame.data;
}

/*!
 * \brief
 * Gets the pitch (bytes per scanline) of the frontend menu's render target.
 *
 * \returns
 * Frontend menu's pitch.
 * 
 */
int LMC_GetMenuRenderTargetPitch(void)
{
	LMC_SetLastError(LMC_ERR_OK);
	return legacy_machine->menu->frame.pitch;
}

/*!
 * \brief
 * Gets the Tilengine context associated with the frontend menu.
 *
 * \returns
 * Frontend menu's TLN_Engine context.
 * 
 */
TLN_Engine LMC_GetMenuTileEngineContext(void)
{
	LMC_SetLastError(LMC_ERR_OK);
	return legacy_machine->menu->tile_engine;
}

/*!
 * \brief
 * Sets the Tilengine context associated with the frontend menu.
 *
 * \remarks
 * LMC_Deinit will only delete the active Tilengine context. Any additional contexts
 * created should be delted with TLN_DeleteContext.
 * 
 */
bool LMC_SetMenuTileEngineContext(TLN_Engine context)
{
	if (TLN_SetContext(context))
	{
		LMC_SetLastError(LMC_ERR_OK);

		// Verify dimensions.
		if (TLN_GetWidth() > legacy_machine->menu->av_info.geometry.max_width)
		{
			lmc_trace(LMC_LOG_ERRORS,
				"base_width parameter must be less than or equal to max_width");
			LMC_SetLastError(LMC_ERR_INV_PARAM);
			return false;
		}
		if (TLN_GetWidth() < 8)
		{
			lmc_trace(LMC_LOG_ERRORS,
				"base_width parameter must be greater than or equal to 8");
			LMC_SetLastError(LMC_ERR_INV_PARAM);
			return false;
		}
		if (TLN_GetHeight() > legacy_machine->menu->av_info.geometry.max_height)
		{
			lmc_trace(LMC_LOG_ERRORS,
				"base_height parameter must be less than or equal to max_height");
			LMC_SetLastError(LMC_ERR_INV_PARAM);
			return false;
		}
		if (TLN_GetHeight() < 8)
		{
			lmc_trace(LMC_LOG_ERRORS,
				"base_height parameter must be greater than or equal to 8");
			LMC_SetLastError(LMC_ERR_INV_PARAM);
			return false;
		}

		// Update pointer to Tilengine context.
		legacy_machine->menu->tile_engine = TLN_GetContext();

		// Update screen geometry to match new Tilengine's context.
		legacy_machine->menu->av_info.geometry.base_width = TLN_GetWidth();
		legacy_machine->menu->av_info.geometry.base_height = TLN_GetHeight();

		// Syncronize Tilengine's fps with LegacyMachine's menu fps.
		TLN_SetTargetFps((int)legacy_machine->menu->av_info.timing.fps);

		return true;
	}
	else
	{
		LMC_SetLastError(LMC_ERR_TILENGINE);
		return false;
	}
}

#endif

/**************************************************************************************************
 * LegacyMachine Callbacks
 *************************************************************************************************/

/*!
 * \brief
 * Sets the function for custom input configuration.
 *
 */
void LMC_SetAutoConfigureJoypadCallback(LMC_AutoConfigureJoypadCallback callback)
{
	LMC_SetLastError(LMC_ERR_OK);
	legacy_machine->input->cb_auto_config = callback;
}

/**************************************************************************************************
 * LegacyMachine Path Management
 *************************************************************************************************/

 /*!
  * \brief
  * Gets the path to a standard directory.
  * 
  * \param path_type
  * Standard path to get.
  *
  */
const char* LMC_GetPath(LMC_Path path_type)
{
	switch (path_type)
	{
	case LMC_MAIN_PATH:
		return legacy_machine->settings->main_directory;
	case LMC_SETTING_PATH:
		return legacy_machine->settings->setting_directory;
	case LMC_ASSET_PATH:
		return legacy_machine->settings->asset_directory;
	case LMC_CORE_PATH:
		return legacy_machine->settings->core_directory;
	case LMC_CONTENT_PATH:
		return legacy_machine->settings->content_directory;
	case LMC_SYSTEM_PATH:
		return legacy_machine->settings->system_directory;
	case LMC_SAVE_PATH:
		return legacy_machine->settings->save_directory;
	case LMC_STATE_PATH:
		return legacy_machine->settings->state_directory;
	default:
		break;
	}
	lmc_trace(LMC_LOG_ERRORS,
		"Supplied path parameter does not exist");
	LMC_SetLastError(LMC_ERR_INV_PATH);
	return "";
}

/**************************************************************************************************
 * LibRetro Core Management
 *************************************************************************************************/

/* Gets time elapsed in microseconds. */
static retro_time_t GetTimeElapsed(void)
{
	return cpu_features_get_time_usec();
}

/* Gets CPU Features.  */
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
	if (counter->registered || (legacy_machine->system->total_performance_counters >= MAX_COUNTERS))
		return;

	legacy_machine->system->performance_counters[legacy_machine->system->total_performance_counters++] = counter;
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
	lmc_core_log(RETRO_LOG_INFO, "[Performance]: Counters:\n");
	LogCorePerformanceCounters(legacy_machine->system->performance_counters,
		legacy_machine->system->total_performance_counters);
}

/* Core environment manager. */
static bool CoreEnvironment(unsigned cmd, void* data)
{
	/* Called every frame. Handle first for efficiency. */
	if (cmd == RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE)
	{
		bool* value = (bool*)data;
		*value = false;
		return true;
	}

	switch (cmd)
	{
	case RETRO_ENVIRONMENT_SET_ROTATION:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_ROTATION: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_GET_OVERSCAN:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_OVERSCAN: not implemented");
		return false;
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
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_MESSAGE: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_SHUTDOWN:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SHUTDOWN: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_PERFORMANCE_LEVEL: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
	{
		*(const char**)data = legacy_machine->settings->system_directory;
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_SYSTEM_DIRECTORY: \"%s\"",
			legacy_machine->settings->system_directory);

		return true;
	}
	case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
	{
		const enum retro_pixel_format* format = (enum retro_pixel_format*)data;

		if (*format > RETRO_PIXEL_FORMAT_RGB565)
			return false;

		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_PIXEL_FORMAT");

		return legacy_machine->video->cb_set_pixel_fmt(*format);
	}
	case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_INPUT_DESCRIPTORS: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_KEYBOARD_CALLBACK: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_DISK_CONTROL_INTERFACE: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_SET_HW_RENDER:
	{
		struct retro_hw_render_callback* hardware_render_callback = (struct retro_hw_render_callback*)data;
		hardware_render_callback->get_current_framebuffer = legacy_machine->video->cb_get_framebuffer;
		if (legacy_machine->video->cb_get_hw_proc_address)
			hardware_render_callback->get_proc_address = legacy_machine->video->cb_get_hw_proc_address;
		legacy_machine->system->cb_hw_render = *hardware_render_callback;
		return true;
	}
	case RETRO_ENVIRONMENT_GET_VARIABLE:
	{
		struct retro_variable* variable = (struct retro_variable*)data;

		if (!retro_variables)
			return false;

		for (const struct retro_variable* v = retro_variables; v->key; ++v) {
			if (strcmp(variable->key, v->key) == 0) {
				variable->value = v->value;
				lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_VARIABLE: \"%s\" : \"%s\"", v->key, v->value);
				break;
			}
		}

		return true;
	}
	case RETRO_ENVIRONMENT_SET_VARIABLES:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_VARIABLES");

		const struct retro_variable* variables = (const struct retro_variable*)data;
		size_t total_variables = 0;

		for (const struct retro_variable* v = variables; v->key; ++v) {
			total_variables++;
		}

		retro_variables = (struct retro_variable*)calloc(total_variables + 1, sizeof(*retro_variables));
		for (unsigned i = 0; i < total_variables; ++i) {
			const struct retro_variable* in_variable = &variables[i];
			struct retro_variable* out_variable = &retro_variables[i];

			const char* semicolon = strchr(in_variable->value, ';');
			const char* first_pipe = strchr(in_variable->value, '|');

			retro_assert(semicolon && *semicolon);
			semicolon++;
			while (isspace(*semicolon))
				semicolon++;

			if (first_pipe) {
				out_variable->value = malloc((first_pipe - semicolon) + 1);
				memcpy((char*)out_variable->value, semicolon, first_pipe - semicolon);
				((char*)out_variable->value)[first_pipe - semicolon] = '\0';
			}
			else {
				out_variable->value = strdup(semicolon);
			}

			out_variable->key = strdup(in_variable->key);
			retro_assert(out_variable->key && out_variable->value);
		}

		return true;
	}
	case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
	{
		legacy_machine->system->current_core->supports_no_game = *(bool*)data;

		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_SUPPORT_NO_GAME: %s",
			legacy_machine->system->current_core->supports_no_game ? "true" : "false");

		return true;
	}
	case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH:
	{
		*(const char**)data = legacy_machine->settings->core_directory;
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_LIBRETRO_PATH: \"%s\"",
			legacy_machine->settings->core_directory);

		return true;
	}
	case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_FRAME_TIME_CALLBACK");

		const struct retro_frame_time_callback* frame_time =
			(const struct retro_frame_time_callback*)data;
		legacy_machine->system->cb_frame_time = *frame_time;

		return true;
	}
	case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_AUDIO_CALLBACK");

		struct retro_audio_callback* audio_callback = (struct retro_audio_callback*)data;
		legacy_machine->system->cb_audio = *audio_callback;

		return true;
	}
	case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_RUMBLE_INTERFACE: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_INPUT_DEVICE_CAPABILITIES: not implemented");
		return false;
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

		struct retro_log_callback* cb_log = (struct retro_log_callback*)data;
		cb_log->log = lmc_core_log;

		return true;
	}
	case RETRO_ENVIRONMENT_GET_PERF_INTERFACE:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_PERF_INTERFACE");

		struct retro_perf_callback* performance = (struct retro_perf_callback*)data;
		performance->get_time_usec = GetTimeElapsed;
		performance->get_cpu_features = GetCPUFeatures;
		performance->get_perf_counter = GetCorePerformanceCounter;
		performance->perf_register = RegisterCorePerformanceCounter;
		performance->perf_start = StartCorePerformanceCounter;
		performance->perf_stop = StopCorePerformanceCounter;
		performance->perf_log = LogCorePerformance;

		return true;
	}
	case RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_LOCATION_INTERFACE: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY:
	{
		*(const char**)data = legacy_machine->settings->content_directory;
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_CONTENT_DIRECTORY: \"%s\"",
			legacy_machine->settings->content_directory);

		return true;
	}
	case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
	{
		*(const char**)data = legacy_machine->settings->save_directory;
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_SAVE_DIRECTORY: \"%s\"",
			legacy_machine->settings->save_directory);

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
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CONTROLLER_INFO: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_SET_MEMORY_MAPS:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_MEMORY_MAPS: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_SET_GEOMETRY:
	{
		const struct retro_game_geometry* geometry = (const struct retro_game_geometry*)data;

		lmc_core_log(RETRO_LOG_INFO,
			"[Environment]: SET_GEOMETRY: Dimensions: %ux%u, Aspect: %.3f",
			geometry->base_width, geometry->base_height, geometry->aspect_ratio);

		return legacy_machine->video->cb_set_geometry_fmt(geometry);
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
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_AUDIO_VIDEO_ENABLE: %i", value);
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
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_INPUT_BITMASKS: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_CORE_OPTIONS_VERSION: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_SET_CORE_OPTIONS:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CORE_OPTIONS: not implemented");

		/* TODO */

		return false;
	}
	case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CORE_OPTIONS_INTL: not implemented");

		/* TODO */

		return false;
	}
	case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CORE_OPTIONS_DISPLAY: not implemented");

		/* TODO */

		return false;
	}
	case RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_PREFERRED_HW_RENDER: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_DISK_CONTROL_INTERFACE_VERSION: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_DISK_CONTROL_EXT_INTERFACE: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_MESSAGE_INTERFACE_VERSION: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_SET_MESSAGE_EXT:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_MESSAGE_EXT: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: GET_INPUT_MAX_USERS: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_AUDIO_BUFFER_STATUS_CALLBACK: not implemented");
		return false;
	}
	case RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_MINIMUM_AUDIO_LATENCY: not implemented");
		return false;
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

		/* TODO */

		return false;
	}
	case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL:
	{
		lmc_core_log(RETRO_LOG_INFO, "[Environment]: SET_CORE_OPTIONS_V2_INTL: not implemented");

		/* TODO */

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
		lmc_core_log(RETRO_LOG_DEBUG, "[Environment]: Unhandled event: #%u", cmd);
		return false;
	}

	return false;
}

/* Refresh core's video. */
static void CoreRefreshVideo(const void* data, unsigned width, unsigned height, size_t pitch)
{
	legacy_machine->video->cb_refresh(data, width, height, pitch);
}

/* Write core's audio. */
static void CoreAudioSample(int16_t left, int16_t right)
{
	int16_t buf[2] = { left, right };
	legacy_machine->audio->cb_write(buf, 1);
}

/* Batch write core's audio. */
static size_t CoreAudioSampleBatch(const int16_t* data, size_t frames)
{
	return legacy_machine->audio->cb_write(data, frames);
}

/* Poll input for running core. */
static void CorePollInput(void)
{
	legacy_machine->input->cb_poll;
}

/* Get input's state for running core. */
static int16_t CoreGetInputState(unsigned port, unsigned device, unsigned index, unsigned id)
{
	return legacy_machine->input->cb_get_state(port, device, index, id);
}

/*!
 * \brief
 * Gets the Core's active running state.
 *
 * \returns
 * True if a core is active and currently running and false if not.
 *
 */
bool LMC_IsCoreRunning(void)
{
	return legacy_machine->system->current_core->running;
}

/*!
 * \brief
 * Loads and initializes a libretro core.
 *
 * \param filename
 * Path on the filesystem to load a core from.
 * 
 * \returns
 * True if a core loads successfully and false if core loading fails.
 * 
 */
bool LMC_LoadCore(const char* filename)
{
	void (*set_environment)(retro_environment_t) = NULL;
	void (*set_video_refresh)(retro_video_refresh_t) = NULL;
	void (*set_input_poll)(retro_input_poll_t) = NULL;
	void (*set_input_state)(retro_input_state_t) = NULL;
	void (*set_audio_sample)(retro_audio_sample_t) = NULL;
	void (*set_audio_sample_batch)(retro_audio_sample_batch_t) = NULL;

	char* fullpath = (char*)malloc(PATH_MAX_LENGTH);
	strlcpy(fullpath, legacy_machine->settings->core_directory, PATH_MAX_LENGTH);
	fill_pathname_slash(fullpath, sizeof(fullpath));
	strlcat(fullpath, filename, PATH_MAX_LENGTH);

	legacy_machine->system->current_core->handle = dylib_load(fullpath);

	if (!legacy_machine->system->current_core->handle)
	{
		lmc_core_log(RETRO_LOG_ERROR, "Failed to load core: %s", dylib_error());
		return false;
	}

	LoadRetroSymbol(retro_init);
	LoadRetroSymbol(retro_deinit);
	LoadRetroSymbol(retro_api_version);
	LoadRetroSymbol(retro_get_system_info);
	LoadRetroSymbol(retro_get_system_av_info);
	LoadRetroSymbol(retro_set_controller_port_device);
	LoadRetroSymbol(retro_reset);
	LoadRetroSymbol(retro_run);
	LoadRetroSymbol(retro_load_game);
	LoadRetroSymbol(retro_unload_game);

	LoadSymbol(set_environment, retro_set_environment);
	LoadSymbol(set_video_refresh, retro_set_video_refresh);
	LoadSymbol(set_input_poll, retro_set_input_poll);
	LoadSymbol(set_input_state, retro_set_input_state);
	LoadSymbol(set_audio_sample, retro_set_audio_sample);
	LoadSymbol(set_audio_sample_batch, retro_set_audio_sample_batch);

	set_environment(CoreEnvironment);
	set_video_refresh(CoreRefreshVideo);
	set_input_poll(CorePollInput);
	set_input_state(CoreGetInputState);
	set_audio_sample(CoreAudioSample);
	set_audio_sample_batch(CoreAudioSampleBatch);

	legacy_machine->system->current_core->retro_init();
	legacy_machine->system->current_core->initialized = true;

	if (fullpath)
		free(fullpath);

	lmc_core_log(RETRO_LOG_INFO, "%s Core loaded", filename);
	return true;
}

/*!
 * \brief
 * Loads and initializes a specific libretro core's content.
 *
 * \param filename
 * Path on the filesystem to load content from.
 * 
 * \returns
 * True if a core's content loads successfully and false if content loading fails.
 * 
 */
bool LMC_LoadContent(const char* filename)
{
	struct retro_system_av_info av_info = { 0 };
	struct retro_system_info system_info = { 0 };
	struct retro_game_info content_info = { filename, 0 };
	char* fullpath = (char*)malloc(PATH_MAX_LENGTH);

	uint8_t* content_data = NULL;
	int64_t content_size = 0;

	if (filename) {
		legacy_machine->system->current_core->retro_get_system_info(&system_info);

		if (!system_info.need_fullpath)
		{
			if (!filestream_read_file(filename,
				(void**)&content_data,
				content_size))
			{
				lmc_core_log(RETRO_LOG_ERROR, "Failed to load %s", filename);
				return false;
			}

			if (content_size < 0)
			{
				lmc_core_log(RETRO_LOG_ERROR, "Failed to query game file size");
				return false;
			}
			content_info.path = filename;
		}
		else
		{
			strlcpy(fullpath, legacy_machine->settings->content_directory, PATH_MAX_LENGTH);
			fill_pathname_slash(fullpath, sizeof(fullpath));
			strlcat(fullpath, filename, PATH_MAX_LENGTH);

			if (!filestream_read_file(fullpath,
				(void**)&content_data,
				content_size))
			{
				lmc_core_log(RETRO_LOG_ERROR, "Failed to load %s", filename);
				return false;
			}

			if (content_size < 0)
			{
				lmc_core_log(RETRO_LOG_ERROR, "Failed to query game file size");
				return false;
			}	
			content_info.path = fullpath;
		}
		content_info.meta = "";
		content_info.data = content_data;
		content_info.size = (size_t)content_size;
	}

	if (!legacy_machine->system->current_core->retro_load_game(&content_info))
	{
		lmc_core_log(RETRO_LOG_ERROR, "The core failed to load the content");
		return false;
	}

	legacy_machine->system->current_core->retro_get_system_av_info(&av_info);

	legacy_machine->video->cb_set_geometry_fmt(&av_info.geometry);

	legacy_machine->window->cb_init();
	legacy_machine->audio->cb_init(av_info.timing.sample_rate);

	if (content_info.data)
		free((void*)content_info.data);

	if (fullpath)
		free(fullpath);

	/* Now that we have the system info, set the window title. */
	char window_title[255];
	snprintf(window_title, sizeof(window_title), "LegacyMachine %s %s", system_info.library_name, system_info.library_version);
	LMC_SetWindowTitle(window_title);

	legacy_machine->system->current_core->running = true;

	return true;
}

/*!
 * \brief
 * Closes current libretro core.
 *
 */
void LMC_CloseCore(void)
{
	if (legacy_machine->system->current_core->initialized)
		legacy_machine->system->current_core->retro_deinit();

	if (legacy_machine->system->current_core->handle)
		dylib_close(legacy_machine->system->current_core->handle);

	memset(legacy_machine->system->current_core, 0, sizeof(*legacy_machine->system->current_core));
}

/*!
 * \brief
 * Updates the menu or runs a single loop of a libretro core and then draws a single frame.
 *
 * \param frame
 * Optional frame number. Set to 0 to autoincrement from previous value.
 * 
 */
void LMC_UpdateFrame(int frame)
{
	/* Auto-increment frame if 0 */
	if (frame != 0)
		legacy_machine->frame = frame;
	else
		legacy_machine->frame += 1;

	if (legacy_machine->system->current_core->running)
	{
		/* Update the game loop timer. */
		if (legacy_machine->system->cb_frame_time.callback) {
			retro_time_t current = GetTimeElapsed();
			retro_time_t delta = current - legacy_machine->system->frame_time_last;

			if (!legacy_machine->system->frame_time_last)
				delta = legacy_machine->system->cb_frame_time.reference;
			legacy_machine->system->frame_time_last = current;
			legacy_machine->system->cb_frame_time.callback(delta);
		}

		/* Ask the core to emit the audio. */
		if (legacy_machine->system->cb_audio.callback) {
			legacy_machine->system->cb_audio.callback();
		}

		/* Run a single loop. */
		legacy_machine->system->current_core->retro_run();
	}
#ifdef HAVE_MENU
	else
	{
		legacy_machine->video->cb_set_pixel_fmt(RETRO_PIXEL_FORMAT_XRGB8888);
		legacy_machine->video->cb_set_geometry_fmt(&legacy_machine->menu->av_info.geometry);
		legacy_machine->menu->frame_time = LMC_GetTicks();
		if (legacy_machine->menu->frame_time > legacy_machine->menu->frame_trigger)
		{
			legacy_machine->menu->frame_trigger = legacy_machine->menu->frame_time + legacy_machine->menu->frame_delay;
			
			/* Update the frontend menu via tilengine. */
			TLN_UpdateFrame(0);
		}

		/* Draw a single frame from the frontend. */
		legacy_machine->video->cb_refresh(NULL,//legacy_machine->menu->frame.data,
			legacy_machine->menu->av_info.geometry.base_width,
			legacy_machine->menu->av_info.geometry.base_height,
			legacy_machine->menu->frame.pitch);
	}
#endif
}

/**************************************************************************************************
 * LegacyMachine Logging Functions
 *************************************************************************************************/

/*!
 * \brief
 * Sets logging level for current instance.
 *
 * \param log_level
 * Value to set, member of the LMC_LogLevel enumeration.
 *
 */
void LMC_SetLogLevel(LMC_LogLevel log_level)
{
	if (legacy_machine != NULL)
		legacy_machine->log_level = log_level;
}

/* Array of error text. */
const char* const errornames[] =
{
	"No error",
	"Not enough memory",
	"Null pointer as required argument",
	"Invalid parameter"
	"Invalid path"
	"Failed to initialize window",
	"Failed to initialize video",
	"Failed to initialize audio",
	"Failed to initialize input",
	"LibRetro core error",
	"Tilengine error",
	"Unsupported function",
};

/*!
 * \brief
 * Sets the global error code of LegacyMachine.
 *
 * \param error
 * Error code to set.
 *
 */
void LMC_SetLastError(LMC_Error error)
{
	if (check_context(legacy_machine))
	{
		legacy_machine->error = error;
		if (error != LMC_ERR_OK)
			lmc_trace(LMC_LOG_ERRORS, errornames[error]);
	}
}

/*!
 * \brief
 * Returns the last error after an invalid operation.
 *
 * \returns
 * Last error after an invalid operation.
 * 
 */
LMC_Error LMC_GetLastError(void)
{
	if (check_context(legacy_machine))
		return legacy_machine->error;
	else
		return LMC_ERR_NULL_POINTER;
}

/*!
 * \brief
 * Returns the string description of the specified error code.
 *
 * \param error
 * Error code to get description.
 *
 * \returns
 * String description of the specified error code.
 * 
 */
const char* LMC_GetErrorString(LMC_Error error)
{
	if (error < MAX_ERRORS)
		return errornames[error];
	else
		return "Invalid error code";
}

/* Outputs trace message */
void lmc_trace(LMC_LogLevel log_level, const char* format, ...)
{
	if (legacy_machine != NULL && legacy_machine->log_level >= log_level)
	{
		char line[255];
		va_list ap;

		va_start(ap, format);
		vsprintf(line, format, ap);
		va_end(ap);

		printf("LegacyMachine: %s\n", line);
	}
}

/* Handles and outputs core log messages */
void lmc_core_log(enum retro_log_level level, const char* format, ...)
{
	if (legacy_machine != NULL && legacy_machine->log_level > 0)
	{
		char line[4095];
		static const char* log_level_str[] = { "Debug", "Info", "Warning", "Error" };
		va_list ap;

		va_start(ap, format);
		vsnprintf(line, sizeof(line), format, ap);
		va_end(ap);

		if ((level != RETRO_LOG_ERROR) && (legacy_machine->log_level == LMC_LOG_ERRORS))
			return;

		printf("LibRetro: [%s] %s\n", log_level_str[level], line);

		if (level == RETRO_LOG_ERROR)
			LMC_SetLastError(LMC_ERR_LIBRETRO);
	}
}