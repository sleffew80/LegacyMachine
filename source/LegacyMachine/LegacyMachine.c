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
 * LegacyMachine Context
 *************************************************************************************************/

/* LegacyMachine main engine context. */
static MainEngine legacy_machine_context = { 0 };

/* Gets the current LegacyMachine main engine context. */
LMC_Engine LMC_GetContext(void)
{
	return &legacy_machine_context;
}

/**************************************************************************************************
 * LegacyMachine Initialization/Deinitialization
 *************************************************************************************************/

#if defined HAVE_MENU

/* Initializes LegacyMachine. */
bool LMC_Init(const char* program_name,
	int base_width, int base_height, int max_width, int max_height,
	float aspect_ratio, double fps, int layers, int sprites, int animations)
#else
/* Initializes LegacyMachine. */
bool LMC_Init(void)
#endif
{
	LMC_Engine legacy_machine = LMC_GetContext();
	DefaultInfo* defaults = GetDefaultInfoContext();

#if !defined HAVE_MENU
	const char* program_name = "LegacyMachine";
#endif
#if defined HAVE_THREADS
	bool threading = true;
#else
	bool threading = false;
#endif

	printf("LegacyMachine v%d.%d.%d %d-bit built %s %s\nLibretro v%u\n",
		LEGACY_MACHINE_VER_MAJ,
		LEGACY_MACHINE_VER_MIN,
		LEGACY_MACHINE_VER_REV,
		(int)(sizeof(UINTPTR_MAX) << 3), __DATE__, __TIME__,
		RETRO_API_VERSION);

#if defined(_WIN32) && !defined(_XBOX) && !defined(__WINRT__)
	/* Initialize the COM interface. */
	if (FAILED(CoInitialize(NULL)))
	{
		lmc_trace(LMC_LOG_ERRORS, "[Platform]: Failed to initialize the COM interface");
		LMC_Deinit();
		LMC_SetLastError(LMC_ERR_FAIL_PLATORM_INIT);
		return false;
	}
#endif

	/* Initialize settings manager. */
	legacy_machine->settings = GetSettingsManagerContext();

	/* Initialize platform manager. */
	legacy_machine->platform = GetPlatformManagerContext();

	/* Initialize window manager. */
	legacy_machine->window = GetWindowManagerContext();

	/* Initialize video manager. */
	legacy_machine->video = GetVideoManagerContext();

	/* Initialize audio manager. */
	legacy_machine->audio = GetAudioManagerContext();

	/* Initialize input manager. */
	legacy_machine->input = GetInputManagerContext();

#if defined HAVE_MENU
	/* Initialize menu manager. */
	legacy_machine->menu = GetMenuManagerContext();

	/* Initialize menu geometry and check for sane dimension values. */
	if (base_width >= 8)
	{
		legacy_machine->menu->av_info.geometry.base_width = base_width;
	}
	else
	{
		lmc_trace(LMC_LOG_ERRORS, 
			"base_width parameter must be greater than or equal to 8");
		LMC_Deinit();
		LMC_SetLastError(LMC_ERR_INV_PARAM);
		return false;
	}
	if (base_height >= 8)
	{
		legacy_machine->menu->av_info.geometry.base_height = base_height;
	}
	else
	{
		lmc_trace(LMC_LOG_ERRORS, 
			"base_height parameter must be greater than or equal to 8");
		LMC_Deinit();
		LMC_SetLastError(LMC_ERR_INV_PARAM);
		return false;
	}
	if (max_width >= base_width)
	{
		legacy_machine->menu->av_info.geometry.max_width = max_width;
	}
	else
	{
		lmc_trace(LMC_LOG_ERRORS,
			"max_width parameter must be greater than or equal to base_width");
		LMC_Deinit();
		LMC_SetLastError(LMC_ERR_INV_PARAM);
		return false;
	}
	if (max_height >= base_height)
	{
		legacy_machine->menu->av_info.geometry.max_height = max_height;
	}
	else
	{
		lmc_trace(LMC_LOG_ERRORS,
			"max_height parameter must be greater than or equal to base_height");
		LMC_Deinit();
		LMC_SetLastError(LMC_ERR_INV_PARAM);
		return false;
	}
	if (aspect_ratio > 0.24999f)
	{
		defaults->aspect_ratio = aspect_ratio;
	}
	else
	{
		defaults->aspect_ratio = 0.25f;
	}
	if (fps > 1.0)
	{
		legacy_machine->menu->av_info.timing.fps = fps;
	}
	else
	{
		lmc_trace(LMC_LOG_ERRORS,
			"fps parameter must be a positive double and greater than 1.0");
		LMC_Deinit();
		LMC_SetLastError(LMC_ERR_INV_PARAM);
		return false;
	}
	legacy_machine->menu->tile_engine = TLN_Init(base_width, base_height, layers, sprites, animations);
	if (!legacy_machine->menu->tile_engine)
	{
		lmc_trace(LMC_LOG_ERRORS, TLN_GetErrorString(TLN_GetLastError()));
		LMC_Deinit();
		LMC_SetLastError(LMC_ERR_TILENGINE);
		return false;
	}
	legacy_machine->menu->frame.pitch = (((max_width * 32) >> 3) + 3) & ~0x03;
	legacy_machine->menu->frame.data = malloc(legacy_machine->menu->frame.pitch * max_height);
	if (!legacy_machine->menu->frame.data)
	{
		lmc_trace(LMC_LOG_ERRORS, "[Menu]: Failed to allocate memory for menu frame buffer");
		LMC_Deinit();
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		return false;
	}
	legacy_machine->menu->available = true;
#endif
	/* Initialize libretro system manager. */
	legacy_machine->system = GetSystemManagerContext();

	/* Set internal program name (required for environment initialization). */
	strlcpy(legacy_machine->settings->program_name, program_name, NAME_MAX_LENGTH);

	/* Initialize platform dependent code. */
	InitializePlatform(PLATFORM_DRIVER_FIRST);

	/* Libretro SDK initialization. */
	InitializeLibretro(threading);

#if defined HAVE_MENU
	/* Additional Tilengine initialization. */
	TLN_SetTargetFps((int)legacy_machine->menu->av_info.timing.fps);
	TLN_SetLoadPath(legacy_machine->settings->paths.path_asset_directory);
	TLN_SetRenderTarget(legacy_machine->menu->frame.data, legacy_machine->menu->frame.pitch);
#endif

#if defined _DEBUG
	LMC_SetLogLevel(LMC_LOG_ERRORS);
#endif
	LMC_SetLastError(LMC_ERR_OK);

	return true;
}

/* Deinitializes LegacyMachine. */
void LMC_Deinit(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	/* Close window and deinitialize video. */
	if (LMC_IsWindowActive())
		LMC_DeleteWindow();

	/* Save any changes made to settings. */
	if (legacy_machine->settings)
		if (legacy_machine->settings->modified)
			SaveSettingsConfigFile(legacy_machine->settings->paths.path_settings_config_file);

#if defined HAVE_MENU
	if (legacy_machine->menu)
	{
		if (legacy_machine->menu->tile_engine)
			TLN_Deinit();
		if (legacy_machine->menu->frame.data)
			free(legacy_machine->menu->frame.data);
	}
#endif

	/* Zero out legacy machine structure. */
	memset(legacy_machine, 0, sizeof(MainEngine));
}

/* Gets the current LegacyMachine library version. */
uint32_t LMC_GetVersion(void)
{
	LMC_SetLastError(LMC_ERR_OK);
	return LEGACY_MACHINE_HEADER_VERSION;
}

/**************************************************************************************************
 * LegacyMachine Callbacks
 *************************************************************************************************/

/* Sets the callback function for custom joypad input configuration.*/
void LMC_SetAutoConfigureJoypadCallback(LMC_AutoConfigureJoypadCallback callback)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	SetJoypadAutoConfigurationCallback(callback);

	LMC_SetLastError(LMC_ERR_OK);
}

/* Updates the menu or runs a single loop of a Libretro core and then draws a single frame. */
void LMC_UpdateFrame(int frame)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	/* Auto-increment frame if 0 */
	if (frame != 0)
		legacy_machine->frame = frame;
	else
		legacy_machine->frame += 1;

	if (legacy_machine->system->running)
	{
		/* Update the game loop timer. */
		if (legacy_machine->system->cb_frame_time.callback) 
		{
			retro_time_t current = legacy_machine->system->cb_get_time_elapsed();
			retro_time_t delta = current - legacy_machine->system->frame_time_last;

			if (!legacy_machine->system->frame_time_last)
				delta = legacy_machine->system->cb_frame_time.reference;
			legacy_machine->system->frame_time_last = current;
			legacy_machine->system->cb_frame_time.callback(delta);
		}

		/* Ask the core to emit the audio. */
		if (legacy_machine->system->cb_audio.callback)
		{
			legacy_machine->system->cb_audio.callback();
		}

		/* Run a single loop. */
		legacy_machine->system->core->library->retro_run();

		/* Check if core requested to be closed. */
		if (legacy_machine->system->shutdown)
		{
			LMC_CloseCore();
		}
	}
#if defined HAVE_MENU
	else
	{
		/* Poll input. */
		PollInput();

		/* Update the frontend menu via Tilengine. */
		TLN_UpdateFrame(0);

		/* Refresh the frontend menu. */
		RefreshVideo(
			legacy_machine->menu->frame.data,
			legacy_machine->menu->av_info.geometry.base_width,
			legacy_machine->menu->av_info.geometry.base_height,
			legacy_machine->menu->frame.pitch);
	}
#endif

	/* Check if audio needs reinitialization */
	UpdateAudio();

	/* Check if there are any pending tasks in the task queue. */
	task_queue_check();
}

/**************************************************************************************************
 * LegacyMachine Logging Functions
 *************************************************************************************************/

/* Sets logging level for current instance. */
void LMC_SetLogLevel(LMC_LogLevel log_level)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	if (legacy_machine != NULL)
		legacy_machine->log_level = log_level;
}

/* Array of error text. */
const char* const errornames[] =
{
	"No error",
	"Not enough memory",
	"Null pointer as required argument",
	"Invalid parameter",
	"Invalid path",
	"Failed to initialize platform",
	"Failed to initialize configuration settings",
	"Failed to initialize window",
	"Failed to initialize video",
	"Failed to initialize audio",
	"Failed to initialize input",
	"Libretro core error",
	"Tilengine error",
	"External error",
	"Unsupported function",
};

/* Sets the global error code of LegacyMachine. */
void LMC_SetLastError(LMC_Error error)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	legacy_machine->error = error;
	if (error != LMC_ERR_OK)
		lmc_trace(LMC_LOG_ERRORS, errornames[error]);
}

/* Gets the last error after an invalid operation. */
LMC_Error LMC_GetLastError(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	return legacy_machine->error;
}

/* Gets the string description of the specified error code. */
const char* LMC_GetErrorString(LMC_Error error)
{
	if (error < MAX_ERRORS)
		return errornames[error];
	else
		return "Invalid error code";
}

/* Logs a message to the LegacyMachine log output. */
void LMC_Log(LMC_LogLevel log_level, const char* format, ...)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	if (log_level == LMC_LOG_ERRORS)
		LMC_SetLastError(LMC_ERR_EXTERNAL);

	if (legacy_machine != NULL && legacy_machine->log_level >= log_level)
	{
		char line[255];
		va_list ap;

		va_start(ap, format);
		vsprintf(line, format, ap);
		va_end(ap);

		printf("%s\n", line);
	}
}

/* Outputs trace message */
void lmc_trace(LMC_LogLevel log_level, const char* format, ...)
{
	LMC_Engine legacy_machine = LMC_GetContext();

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
	LMC_Engine legacy_machine = LMC_GetContext();

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

		printf("Libretro: [%s] %s\n", log_level_str[level], line);

		if (level == RETRO_LOG_ERROR)
			LMC_SetLastError(LMC_ERR_LIBRETRO);
	}
}