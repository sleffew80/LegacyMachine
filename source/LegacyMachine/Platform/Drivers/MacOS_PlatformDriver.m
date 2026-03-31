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
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string/stdstring.h>
#include <file/file_path.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFArray.h>
#include <Foundation/NSPathUtilities.h>

#include "../PlatformManager.h"
#include "../../MainEngine.h"

/**************************************************************************************************
 * MacOS Platform Internal Functions
 *************************************************************************************************/

/* Resolve a path to its full path, optionally resolving symbolic links. */
static char* ResolveRealPath(char* path, size_t size, bool resolve_symlinks)
{
	char temp_path[PATH_MAX_LENGTH];
	size_t path_length;
	char* ptr;
	const char* next;
	const char* buffer_end;

	if (resolve_symlinks)
	{
		strlcpy(temp_path, path, sizeof(temp_path));

		if (!realpath(temp_path, path))
		{
			strlcpy(path, temp_path, size);
			return NULL;
		}

		return path;
	}

	path_length = 0;
	buffer_end = path + strlen(path);

	if (!path_is_absolute(path))
	{
		size_t length;

		if (!getcwd(temp_path, PATH_MAX_LENGTH - 1))
			return NULL;

		length = strlen(temp_path);
		path_length += length;

		if (temp_path[length - 1] != '/')
			temp_path[path_length++] = '/';

		if (string_is_empty(path))
		{
			temp_path[path_length] = '\0';
			strlcpy(path, temp_path, size);
			return path;
		}

		ptr = path;
	}
	else
	{
		for (ptr = path; *ptr == '/'; ptr++)
			temp_path[path_length++] = '/';
	}

	do
	{
		next = strchr(ptr, '/');
		if (!next)
			next = buffer_end;

		if ((next - ptr == 2 && ptr[0] == '.' && ptr[1] == '.'))
		{
			ptr += 3;

			if (path_length == 1 || temp_path[path_length - 2] == '/')
				return NULL;

			path_length = path_length - 2;
			while (temp_path[path_length] != '/')
				path_length--;
			path_length++;
		}
		else if (next - ptr == 1 && ptr[0] == '.')
			ptr += 2;
		else if (next - ptr == 0)
			ptr += 1;
		else
		{
			if (path_length + next - ptr + 1 > PATH_MAX_LENGTH - 1)
				return NULL;

			while (ptr <= next)
				temp_path[path_length++] = *ptr++;
		}

	} while (next < buffer_end);

	temp_path[path_length] = '\0';
	strlcpy(path, temp_path, size);
	return path;
}

/* Get and allocate a string for an environment path */
static char* GetEnvironmentPath(const char* environment_var)
{
	char* path = NULL;
	const char* src_path = getenv(environment_var);

	if (src_path)
	{
		size_t path_length = strlen(src_path);
		if (path_length != 0)
		{
			char* dst_path = (char*)malloc(path_length + 1);
			if (!dst_path)
				return NULL;
			strcpy_literal(dst_path, src_path);
			path = dst_path;
		}
	}
	else
		return NULL;

	return path;
}

/**************************************************************************************************
 * MacOS Platform Functions
 *************************************************************************************************/

/* Get environment and set directories. */
static bool MacOS_DriverGetEnvironment(void)
{
	SettingsManager* settings = GetSettingsManagerContext();

	char base_path[PATH_MAX] = {0};
	const char* home_path = GetEnvironmentPath("HOME");
	const char* temp_path = GetEnvironmentPath("TMPDIR");

	if (!home_path)
		return false;
	if (!temp_path)
		return false;

	CFURLRef bundle_url;
	CFStringRef bundle_path;
	char bundle_path_buffer[PATH_MAX] = {0};
	char documents_path_buffer[PATH_MAX] = {0};
	char application_data_path[PATH_MAX] = {0};

	CFBundleRef bundle = CFBundleGetMainBundle();

	if(!bundle)
		return false;

	bundle_url = CFBundleCopyBundleURL(bundle);
	bundle_path = CFURLCopyFileSystemPath(bundle_url, kCFURLPOSIXPathStyle);
	CFStringGetCString(bundle_path, bundle_path_buffer, sizeof(bundle_path_buffer), kCFStringEncodingUTF8);
	CFRelease(bundle_path);
	CFRelease(bundle_url);
	ResolveRealPath(bundle_path_buffer, sizeof(bundle_path_buffer), true);

	if(home_path)
	{
		strlcpy(base_path, home_path, PATH_MAX);
		strlcat(base_path, "/Library/Application Support/", sizeof(base_path));
		fill_pathname_join(application_data_path, base_path,
			settings->program_name, sizeof(application_data_path));
	}

	strlcpy(settings->paths.path_temp_directory, temp_path, sizeof(settings->paths.path_temp_directory));

	strlcpy(settings->paths.path_main_directory, bundle_path_buffer, sizeof(settings->paths.path_main_directory));
	strlcat(settings->paths.path_main_directory, "/Contents/MacOS", sizeof(settings->paths.path_main_directory));
	fill_pathname_join(settings->paths.path_asset_directory, application_data_path,
		"Assets", sizeof(settings->paths.path_asset_directory));
	fill_pathname_join(settings->paths.path_setting_directory, application_data_path,
		"Settings", sizeof(settings->paths.path_setting_directory));
	fill_pathname_join(settings->paths.path_core_directory, application_data_path,
		"Cores", sizeof(settings->paths.path_core_directory));
	fill_pathname_join(settings->paths.path_content_directory, application_data_path,
		"Content", sizeof(settings->paths.path_content_directory));
	fill_pathname_join(settings->paths.path_system_directory, application_data_path,
		"System", sizeof(settings->paths.path_system_directory));
	fill_pathname_join(settings->paths.path_save_directory, application_data_path,
		"Saves", sizeof(settings->paths.path_save_directory));
	fill_pathname_join(settings->paths.path_state_directory, application_data_path,
		"States", sizeof(settings->paths.path_state_directory));

	free(home_path);
	free(temp_path);

	return true;
}

/**************************************************************************************************
 * MacOS Platform Driver
 *************************************************************************************************/

PlatformDriver macos_platform_driver = {
	MacOS_DriverGetEnvironment,
	PLATFORM_DRIVER_MACOS
};
