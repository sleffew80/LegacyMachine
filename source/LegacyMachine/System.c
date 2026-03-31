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
#include <file/file_path.h>

#include "LegacyMachine.h"
#include "MainEngine.h"

/**************************************************************************************************
 * Libretro Disk Control Management
 *************************************************************************************************/

/* Checks the current state of Libretro core's disk control. */
bool LMC_IsDiskControlEnabled(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	if (legacy_machine->system->cb_disk_control.set_eject_state &&
		legacy_machine->system->cb_disk_control.get_eject_state &&
		legacy_machine->system->cb_disk_control.get_image_index &&
		legacy_machine->system->cb_disk_control.set_image_index &&
		legacy_machine->system->cb_disk_control.get_num_images)
	{
		return true;
	}

	return false;
}

/* Gets the total number of disks/images for the currently loaded Libretro content. */
unsigned LMC_GetDiskCount(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	if (legacy_machine->system->cb_disk_control.get_num_images)
		return legacy_machine->system->cb_disk_control.get_num_images();

	return 0;
}

/* Gets the current disk index of the the currently loaded libretro content. */
unsigned LMC_GetDiskIndex(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	if (legacy_machine->system->cb_disk_control.get_image_index)
		return legacy_machine->system->cb_disk_control.get_image_index();

	return 0;
}

/* Switches to a given disk index for the currently loaded Libretro content. */
bool LMC_SwitchDisk(unsigned index)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	bool result = false;

	if (!legacy_machine->system->cb_disk_control.set_eject_state ||
		!legacy_machine->system->cb_disk_control.set_image_index)
		return false;

	if (index >= LMC_GetDiskCount())
		return false;

	legacy_machine->system->cb_disk_control.set_eject_state(true);
	result = legacy_machine->system->cb_disk_control.set_image_index(index);
	legacy_machine->system->cb_disk_control.set_eject_state(false);

	return result;
}

/**************************************************************************************************
 * Libretro Core Management
 *************************************************************************************************/

/* Checks if a Libretro core is currently loaded and running. */
bool LMC_IsCoreRunning(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	return legacy_machine->system->running;
}

/* Loads and initializes a Libretro core from a given filename. */
bool LMC_LoadCore(const char* filename, bool suppress_kb_ui_input)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	if (!filename)
	{
		lmc_core_log(RETRO_LOG_ERROR, "[Core]: No libretro core library file name provided");
		return false;
	}

	if (!InitializeSystem())
	{
		lmc_core_log(RETRO_LOG_ERROR, "[System]: Failed to initialize libretro system");
		return false;
	}

	/* Determine if path to core library file is absolute or relative to core directory. */
	if (path_is_absolute(filename))
	{
		strlcpy(legacy_machine->system->core->file_path, filename, PATH_MAX_LENGTH);
	}
	else
	{
		fill_pathname_join(legacy_machine->system->core->file_path,
			legacy_machine->settings->paths.path_core_directory,
			filename,
			PATH_MAX_LENGTH);
	}

	/* Apply configurable system settings. */
#if defined(HAVE_ZLIB)
	legacy_machine->system->compress_states = legacy_machine->settings->bools.compress_state_files;
	legacy_machine->system->compress_saves = legacy_machine->settings->bools.compress_save_files;
#endif

	/* Set keyboard behavior. */
	if (suppress_kb_ui_input)
	{
		legacy_machine->input->keyboard->interface_input = false;
		legacy_machine->input->keyboard->joypad_input = false;
	}

	return true;
}

/* Loads content into the currently loaded Libretro core. */
bool LMC_LoadContent(const char* filename)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	if (filename)
	{
		/* Determine if path to content file is absolute or relative to content directory. */
		if (path_is_absolute(filename))
		{
			strlcpy(legacy_machine->system->content->file_path, filename, PATH_MAX_LENGTH);
		}
		else
		{
			fill_pathname_join(legacy_machine->system->content->file_path,
				legacy_machine->settings->paths.path_content_directory,
				filename,
				PATH_MAX_LENGTH);
		}
	}
	else
		legacy_machine->system->core->contentless = true;

	return RunCore();
}

/* Closes the currently loaded Libretro core and unloads its content. */
void LMC_CloseCore(void)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	/* If writing a save file fails, proceed with a warning. */
	if (!WriteSaveFile(legacy_machine->system->core->save_path, "SRAM", RETRO_MEMORY_SAVE_RAM))
	{
		lmc_core_log(RETRO_LOG_WARN,
			"[SRAM]: Failed to write SRAM save file: \"%s\"",
			legacy_machine->system->core->save_path);
	}
	if (!WriteSaveFile(legacy_machine->system->core->clock_path, "RTC", RETRO_MEMORY_RTC))
	{
		lmc_core_log(RETRO_LOG_WARN,
			"[RTC]: Failed to write RTC save file: \"%s\"",
			legacy_machine->system->core->clock_path);
	}

	if (legacy_machine->system->core->options->initialized)
		DeinitializeOptions();

	if (legacy_machine->system->core->library->initialized)
	{
		if (legacy_machine->system->running)
			legacy_machine->system->core->library->retro_unload_game();

		legacy_machine->system->core->library->retro_deinit();
	}

	if (legacy_machine->system->core->library->handle)
		dylib_close(legacy_machine->system->core->library->handle);

	for (LMC_Slot slot = LMC_SLOT_QUICK; slot < MAX_STATE_SLOTS; slot++)
	{
		if (legacy_machine->system->core->state_paths[slot])
		{
			free(legacy_machine->system->core->state_paths[slot]);
			legacy_machine->system->core->state_paths[slot] = NULL;
		}
	}

	if (legacy_machine->system->core->state_paths)
	{
		free(legacy_machine->system->core->state_paths);
		legacy_machine->system->core->state_paths = NULL;
	}

	DeinitializeAudio();

	legacy_machine->audio->output.latency = legacy_machine->audio->output.minimum_latency;

	memset(legacy_machine->settings->paths.path_option_directory, 0, PATH_MAX_LENGTH);
	memset(legacy_machine->settings->paths.path_option_content_directory, 0, PATH_MAX_LENGTH);

	legacy_machine->settings->paths.path_option_directory[0] = '\0';
	legacy_machine->settings->paths.path_option_content_directory[0] = '\0';

	if (legacy_machine->system->initialized)
		DeinitializeSystem();

	/* Restore keyboard behavior. */
	legacy_machine->input->keyboard->interface_input = true;
	legacy_machine->input->keyboard->joypad_input = legacy_machine->settings->bools.input_keyboard_enable_joypad_input;

#if defined HAVE_MENU
	/* Flush all joypad digital inputs to prevent stale state carrying over to
	   the menu (e.g. a confirm key held at core shutdown re-triggering a selection). */
	if (legacy_machine->input && legacy_machine->input->joypad)
	{
		unsigned p;
		for (p = 0; p < MAX_PLAYERS; p++)
			legacy_machine->input->joypad->state[p].digital_inputs = 0;
	}

	/* Restore menu video settings. */
	SetVideoPixelFormat(RETRO_PIXEL_FORMAT_XRGB8888);
	SetVideoGeometry(&legacy_machine->menu->av_info.geometry);
	legacy_machine->video->output.raw_frame.pitch = legacy_machine->menu->frame.pitch;
#endif

}

/* Saves the current state of the Libretro core. */
void LMC_SaveState(LMC_Slot slot)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	void* data = NULL;

	if (!legacy_machine->system->core->library->retro_serialize_size() > 0)
	{
		lmc_core_log(RETRO_LOG_INFO, "[State]: Current core does not support save states");
		return;
	}

	size_t size = legacy_machine->system->core->library->retro_serialize_size();

#if defined(HAVE_ZLIB)
	PushSaveStateTask(legacy_machine->system->core->state_paths[slot],
		data, size, slot, legacy_machine->system->compress_states);
#else
	PushSaveStateTask(legacy_machine->system->core->state_paths[slot],
		data, size, slot, false);
#endif
}

/*  Loads a saved state into the Libretro core. */
void LMC_LoadState(LMC_Slot slot)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	void* data = NULL;

	if (!legacy_machine->system->core->library->retro_serialize_size() > 0)
	{
		lmc_core_log(RETRO_LOG_INFO,
			"[State]: Current core does not support save states");
		return;
	}

	if (!path_is_valid(legacy_machine->system->core->state_paths[slot]))
	{
		lmc_core_log(RETRO_LOG_INFO,
			"[State]: Save state slot #%d is currently empty.", slot);
		return;
	}

	PushLoadStateTask(legacy_machine->system->core->state_paths[slot], slot);
}

/* Sets the controller device type for a given port. If the core is currently running,
   the change takes effect immediately. */
bool LMC_SetControllerPortDevice(unsigned port, unsigned device)
{
	return SetControllerPortDevice(port, device);
}
