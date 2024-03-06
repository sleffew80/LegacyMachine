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
#include <string/stdstring.h>
#include <compat/strl.h>
#include <file/file_path.h>

#include "../PlatformDriver.h"
#include "../../MainEngine.h"

/* Get environment and set directories. */
static void Unix_GetEnvironment(void)
{
	SettingsManager* settings = GetSettingsManagerContext();

	char base_path[PATH_MAX] = {0};
	char local_path[PATH_MAX] = {0};
	const char *xdg_path = getenv("XDG_CONFIG_HOME");
	const char *home_path = getenv("HOME");

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
		strcpy_literal(base_path, settings->program_name);
	}

	fill_pathname_application_dir(local_path, sizeof(local_path));
	fill_pathname_application_path(settings->main_directory,
		sizeof(settings->main_directory));
	fill_pathname_join(settings->asset_directory, local_path,
		"assets", sizeof(settings->asset_directory));
	fill_pathname_join(settings->setting_directory, base_path,
		"settings", sizeof(settings->setting_directory));
	fill_pathname_join(settings->core_directory, base_path,
		"cores", sizeof(settings->core_directory));
	fill_pathname_join(settings->content_directory, base_path,
		"content", sizeof(settings->content_directory));
	fill_pathname_join(settings->system_directory, base_path,
		"system", sizeof(settings->system_directory));
	fill_pathname_join(settings->save_directory, base_path,
      	"saves", sizeof(settings->save_directory));
	fill_pathname_join(settings->state_directory, base_path,
      	"states", sizeof(settings->state_directory));
}

/**************************************************************************************************
 * Unix Platform Driver
 *************************************************************************************************/

PlatformDriver unix_platform_driver = {
	Unix_GetEnvironment,
	false
};