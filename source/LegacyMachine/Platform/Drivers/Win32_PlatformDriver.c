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
#include <file/file_path.h>

#include "../PlatformDriver.h"
#include "../../MainEngine.h"

/* Get environment and set directories. */
static void Win32_GetEnvironment(void)
{
	SettingsManager* settings = GetSettingsManagerContext();

	fill_pathname_expand_special(settings->main_directory,
		":", sizeof(settings->main_directory));
	fill_pathname_expand_special(settings->asset_directory,
		":\\Assets", sizeof(settings->asset_directory));
	fill_pathname_expand_special(settings->setting_directory,
		":\\Settings", sizeof(settings->setting_directory));
	fill_pathname_expand_special(settings->core_directory,
		":\\Cores", sizeof(settings->core_directory));
	fill_pathname_expand_special(settings->content_directory,
		":\\Content", sizeof(settings->content_directory));
	fill_pathname_expand_special(settings->system_directory,
		":\\System", sizeof(settings->system_directory));
	fill_pathname_expand_special(settings->save_directory,
		":\\Saves", sizeof(settings->save_directory));
	fill_pathname_expand_special(settings->state_directory,
		":\\States", sizeof(settings->state_directory));
}

/**************************************************************************************************
 * Win32 Platform Driver
 *************************************************************************************************/

PlatformDriver win32_platform_driver = {
	Win32_GetEnvironment,
	false
};