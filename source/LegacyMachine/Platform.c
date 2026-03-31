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
#include <retro_timers.h>
#include <features/features_cpu.h>

#include "LegacyMachine.h"
#include "MainEngine.h"

/**************************************************************************************************
 * LegacyMachine Path Management
 *************************************************************************************************/

/* Gets the path to a standard directory. */
const char* LMC_GetPath(LMC_Path path_type)
{
	LMC_Engine legacy_machine = LMC_GetContext();

	LMC_SetLastError(LMC_ERR_OK);

	switch (path_type)
	{
		case LMC_PATH_MAIN:
			return legacy_machine->settings->paths.path_main_directory;
		case LMC_PATH_SETTINGS:
			return legacy_machine->settings->paths.path_setting_directory;
		case LMC_PATH_ASSETS:
			return legacy_machine->settings->paths.path_asset_directory;
		case LMC_PATH_CORE:
			return legacy_machine->settings->paths.path_core_directory;
		case LMC_PATH_CONTENT:
			return legacy_machine->settings->paths.path_content_directory;
		case LMC_PATH_SYSTEM:
			return legacy_machine->settings->paths.path_system_directory;
		case LMC_PATH_SAVES:
			return legacy_machine->settings->paths.path_save_directory;
		case LMC_PATH_STATES:
			return legacy_machine->settings->paths.path_state_directory;
		default:
		{
			lmc_trace(LMC_LOG_ERRORS, "Supplied path parameter does not exist");
			LMC_SetLastError(LMC_ERR_INV_PATH);
			return "";
		}
	}
}

/**************************************************************************************************
 * LegacyMachine Timing Management
 *************************************************************************************************/

/* Get the number of microseconds since last epoch. */
uint64_t LMC_GetTimeElapsed(void)
{
	return (uint64_t)cpu_features_get_time_usec();
}

/* Suspend execution for a fixed time. */
void LMC_SetDelay(uint32_t time)
{
	retro_sleep(time);
}