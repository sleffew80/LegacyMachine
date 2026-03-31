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

#include "PlatformManager.h"
#include "../MainEngine.h"

/**************************************************************************************************
 * PlatformManager Context
 *************************************************************************************************/

static PlatformManager platform_manager = { 0 };

/**************************************************************************************************
 * PlatformDriver Context Array
 *************************************************************************************************/

const PlatformDriver* platform_drivers[] = {
#if defined(__linux__) || (defined(BSD) && !defined(__MACH__))
	&unix_platform_driver,
#endif
#if defined(__APPLE__) && defined(__MACH__)
	&macos_platform_driver,
#endif
#if defined(_WIN32) && !defined(_XBOX) && !defined(__WINRT__)
	&win32_platform_driver,
#endif
	NULL
};

/**************************************************************************************************
 * PlatformDriver Context
 *************************************************************************************************/

static PlatformDriver* platform_driver = NULL;

/**************************************************************************************************
 * PlatformDriver Functions
 *************************************************************************************************/

/* Verify that PlatformDriverID enum values match their corresponding indices in the
   platform_drivers array. */
static void AssertPlatformDriverEnumValues(void)
{
	for (unsigned i = 0; i < MAX_PLATFORM_DRIVERS; ++i)
	{
		retro_assert((PlatformDriverID)i == platform_drivers[i]->id);
	}
}

/* Initialize the platform driver for managing platform specific tasks. */
static PlatformDriver* InitializePlatformDriver(PlatformDriverID driver_id)
{
#if defined _DEBUG
	/* Verify PlatformDriverID values when debugging. */
	AssertPlatformDriverEnumValues();
#endif
	return (PlatformDriver*)platform_drivers[driver_id];
}

/* Get the current platform driver context. */
static PlatformDriver* GetPlatformDriverContext(void)
{
	return platform_driver;
}

/**************************************************************************************************
 * PlatformManager Functions
 *************************************************************************************************/

/* Get the current platform manager context. */
PlatformManager* GetPlatformManagerContext(void)
{
	return &platform_manager;
}

/* Get the current platform driver ID. */
PlatformDriverID GetPlatformDriverID(void)
{
	PlatformDriver* driver = GetPlatformDriverContext();
	return driver ? driver->id : PLATFORM_DRIVER_NONE;
}

/* Initialize the platform manager and execute platform-specific environment setup. */
bool InitializePlatform(PlatformDriverID driver_id)
{
	PlatformManager* platform = GetPlatformManagerContext();
	PlatformDriver* driver = InitializePlatformDriver(driver_id);
	SettingsManager* settings = GetSettingsManagerContext();

	if (platform->initialized)
		return true;

	if (!driver || !driver->cb_get_env())
		return false;

	platform_driver = driver;

	/* Create required directories if they don't already exist. */
	if (!path_is_directory(settings->paths.path_setting_directory))
	{
		if (!path_mkdir(settings->paths.path_setting_directory))
			return false;
	}
	if (!path_is_directory(settings->paths.path_core_directory))
	{
		if (!path_mkdir(settings->paths.path_core_directory))
			return false;
	}
	if (!path_is_directory(settings->paths.path_content_directory))
	{
		if (!path_mkdir(settings->paths.path_content_directory))
			return false;
	}
	if (!path_is_directory(settings->paths.path_system_directory))
	{
		if (!path_mkdir(settings->paths.path_system_directory))
			return false;
	}
	if (!path_is_directory(settings->paths.path_save_directory))
	{
		if (!path_mkdir(settings->paths.path_save_directory))
			return false;
	}
	if (!path_is_directory(settings->paths.path_state_directory))
	{
		if (!path_mkdir(settings->paths.path_state_directory))
			return false;
	}

	return true;
}

/* Deinitialize the platform manager and its driver. */
void DeinitializePlatform(void)
{
	PlatformManager* platform = GetPlatformManagerContext();

	platform_driver = NULL;

	memset(platform, 0, sizeof(PlatformManager));
}