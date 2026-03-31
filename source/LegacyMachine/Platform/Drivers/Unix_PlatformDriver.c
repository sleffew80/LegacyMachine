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
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <compat/strl.h>
#include <string/stdstring.h>
#include <file/file_path.h>

#include "../PlatformManager.h"
#include "../../MainEngine.h"

/**************************************************************************************************
 * Unix Platform Internal Functions
 *************************************************************************************************/

/* Fill the application path */
static void FillApplicationPath(char* path, size_t size)
{
	if (!size)
		return;

	pid_t pid;
	static const char* exts[] = { "exe", "file", "path/a.out" };
	char link_path[255];

	link_path[0] = *path = '\0';
	pid = getpid();

	for (size_t i = 0; i < ARRAY_SIZE(exts); i++)
	{
		ssize_t ret;

		snprintf(link_path, sizeof(link_path), "/proc/%u/%s",
			(unsigned)pid, exts[i]);
		ret = readlink(link_path, path, size - 1);

		if (ret >= 0)
		{
			path[ret] = '\0';
			return;
		}
	}
}

/* Fill the application directory */
static void FillApplicationDirectory(char* path, size_t size)
{
	FillApplicationPath(path, size);
	path_basedir_wrapper(path);
}

/* Get and allocate a string for an environment path */
static char* GetEnvironmentPath(const char* environment_var)
{
	char* path = NULL;
	const char* src_path = NULL;

	char* env_directory = getenv(environment_var);
	if (env_directory)
		src_path = env_directory;

	if (!env_directory && string_is_equal(environment_var, "TMPDIR"))
		src_path = "/tmp";

	if (src_path)
	{
		size_t   path_length = strlen(src_path);
		if (path_length != 0)
		{
			char* dst_path = (char*)malloc(path_length + 1);
			if (!dst_path)
				return NULL;
			strlcpy(dst_path, src_path, PATH_MAX);
			path = dst_path;
		}
	}
	else
		return NULL;

	return path;
}

/**************************************************************************************************
 * Unix Platform Functions
 *************************************************************************************************/

/* Get environment and set directories. */
static bool Unix_DriverGetEnvironment(void)
{
	SettingsManager* settings = GetSettingsManagerContext();

	char base_path[PATH_MAX] = { 0 };
	char local_path[PATH_MAX] = { 0 };
	const char* xdg_path = GetEnvironmentPath("XDG_CONFIG_HOME");
	const char* home_path = GetEnvironmentPath("HOME");
	const char* temp_path = GetEnvironmentPath("TMPDIR");

	if (!xdg_path && !home_path)
		return false;

	if (!temp_path)
		return false;

	if (xdg_path)
	{
		strlcpy(base_path, xdg_path, PATH_MAX);
		strlcat(base_path, "/", PATH_MAX);
		strlcat(base_path, settings->program_name, PATH_MAX);
	}
	else if (home_path)
	{
		strlcpy(base_path, home_path, PATH_MAX);
		strlcat(base_path, "/.config/", PATH_MAX);
		strlcat(base_path, settings->program_name, PATH_MAX);
	}
	else
	{
		strlcpy(base_path, settings->program_name, PATH_MAX);
	}

	strlcpy(settings->paths.path_temp_directory, temp_path, sizeof(settings->paths.path_temp_directory));

	FillApplicationDirectory(local_path, sizeof(local_path));
	FillApplicationPath(settings->paths.path_main_directory,
		sizeof(settings->paths.path_main_directory));
	fill_pathname_join(settings->paths.path_asset_directory, base_path,
		"assets", sizeof(settings->paths.path_asset_directory));
	fill_pathname_join(settings->paths.path_setting_directory, base_path,
		"settings", sizeof(settings->paths.path_setting_directory));
	fill_pathname_join(settings->paths.path_core_directory, base_path,
		"cores", sizeof(settings->paths.path_core_directory));
	fill_pathname_join(settings->paths.path_content_directory, base_path,
		"content", sizeof(settings->paths.path_content_directory));
	fill_pathname_join(settings->paths.path_system_directory, base_path,
		"system", sizeof(settings->paths.path_system_directory));
	fill_pathname_join(settings->paths.path_save_directory, base_path,
		"saves", sizeof(settings->paths.path_save_directory));
	fill_pathname_join(settings->paths.path_state_directory, base_path,
		"states", sizeof(settings->paths.path_state_directory));

	if (xdg_path)
		free(xdg_path);
	if (home_path)
		free(home_path);
	if (temp_path)
		free(temp_path);

	return true;
}

/**************************************************************************************************
 * Unix Platform Driver
 *************************************************************************************************/

PlatformDriver unix_platform_driver = {
	Unix_DriverGetEnvironment,
	PLATFORM_DRIVER_UNIX
};