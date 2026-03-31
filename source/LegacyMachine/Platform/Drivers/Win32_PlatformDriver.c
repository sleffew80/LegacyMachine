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
#include <encodings/utf.h>
#include <file/file_path.h>

#include "../PlatformManager.h"
#include "../../MainEngine.h"

#if defined(_WIN32_WINNT) && _WIN32_WINNT < 0x0500 || defined(_XBOX)
#ifndef LEGACY_WIN32
#define LEGACY_WIN32
#endif
#endif

/**************************************************************************************************
 * Win32 Platform Functions
 *************************************************************************************************/

/* Get environment and set directories. */
static bool Win32_DriverGetEnvironment(void)
{
	SettingsManager* settings = GetSettingsManagerContext();
	char* temp_path = NULL;

#if defined LEGACY_WIN32
	DWORD temp_path_length = GetTempPath(0, NULL) + 1;

	if (!(temp_path = (char*)malloc(temp_path_length * sizeof(char))))
		return false;

	temp_path[temp_path_length - 1] = 0;
	GetTempPath(temp_path_length, temp_path);
#else
	DWORD temp_path_length = GetTempPathW(0, NULL) + 1;
	wchar_t* wide_string = (wchar_t*)malloc(temp_path_length * sizeof(wchar_t));

	if (!wide_string)
		return false;

	wide_string[temp_path_length - 1] = 0;
	GetTempPathW(temp_path_length, wide_string);

	temp_path = utf16_to_utf8_string_alloc(wide_string);

	if (!temp_path)
	{
		free(wide_string);
		return false;
	}

	free(wide_string);
#endif
	strlcpy(settings->paths.path_temp_directory, temp_path, sizeof(settings->paths.path_temp_directory));

	free(temp_path);

	/* Resolve application directory directly via OS API to build base paths. */
#if defined LEGACY_WIN32
	char exe_path[PATH_MAX_LENGTH];
	exe_path[0] = '\0';
	GetModuleFileName(NULL, exe_path, PATH_MAX_LENGTH);
	fill_pathname_basedir(settings->paths.path_main_directory, exe_path,
		sizeof(settings->paths.path_main_directory));
#else
	wchar_t wide_exe_path[PATH_MAX_LENGTH];
	char* exe_path_utf8;
	wide_exe_path[0] = L'\0';
	GetModuleFileNameW(NULL, wide_exe_path, PATH_MAX_LENGTH);
	exe_path_utf8 = utf16_to_utf8_string_alloc(wide_exe_path);
	if (exe_path_utf8)
	{
		fill_pathname_basedir(settings->paths.path_main_directory, exe_path_utf8,
			sizeof(settings->paths.path_main_directory));
		free(exe_path_utf8);
	}
#endif

	fill_pathname_join(settings->paths.path_asset_directory,
		settings->paths.path_main_directory, "Assets", sizeof(settings->paths.path_asset_directory));
	fill_pathname_join(settings->paths.path_setting_directory,
		settings->paths.path_main_directory, "Settings", sizeof(settings->paths.path_setting_directory));
	fill_pathname_join(settings->paths.path_core_directory,
		settings->paths.path_main_directory, "Cores", sizeof(settings->paths.path_core_directory));
	fill_pathname_join(settings->paths.path_content_directory,
		settings->paths.path_main_directory, "Content", sizeof(settings->paths.path_content_directory));
	fill_pathname_join(settings->paths.path_system_directory,
		settings->paths.path_main_directory, "System", sizeof(settings->paths.path_system_directory));
	fill_pathname_join(settings->paths.path_save_directory,
		settings->paths.path_main_directory, "Saves", sizeof(settings->paths.path_save_directory));
	fill_pathname_join(settings->paths.path_state_directory,
		settings->paths.path_main_directory, "States", sizeof(settings->paths.path_state_directory));

	return true;
}

/**************************************************************************************************
 * Win32 Platform Driver
 *************************************************************************************************/

PlatformDriver win32_platform_driver = {
	Win32_DriverGetEnvironment,
	PLATFORM_DRIVER_WIN32
};