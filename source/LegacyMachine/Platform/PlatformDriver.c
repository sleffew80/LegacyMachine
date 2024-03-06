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
#include "PlatformDriver.h"

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
 * PlatformDriver Initialization
 *************************************************************************************************/

/* Initialize the platform driver for managing platform specific tasks. */
PlatformDriver* InitializePlatformDriver(void)
{
	return (PlatformDriver*)platform_drivers[0];
}