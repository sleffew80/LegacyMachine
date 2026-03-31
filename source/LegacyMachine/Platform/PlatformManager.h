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

#ifndef _PLATFORM_MANAGER_H
#define _PLATFORM_MANAGER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"

/**************************************************************************************************
 * Platform Definitions
 *************************************************************************************************/

#define PLATFORM_DRIVER_FIRST (PlatformDriverID)0

/**************************************************************************************************
 * PlatformDriver Enumeration
 *************************************************************************************************/

typedef enum
{
#if defined(__linux__) || (defined(BSD) && !defined(__MACH__))
	PLATFORM_DRIVER_UNIX,
#endif
#if defined(__APPLE__) && defined(__MACH__)
	PLATFORM_DRIVER_MACOS,
#endif
#if defined(_WIN32) && !defined(_XBOX) && !defined(__WINRT__)
	PLATFORM_DRIVER_WIN32,
#endif
	PLATFORM_DRIVER_NONE,
	MAX_PLATFORM_DRIVERS = PLATFORM_DRIVER_NONE
}
PlatformDriverID;

/**************************************************************************************************
 * PlatformDriver Structure
 *************************************************************************************************/

typedef struct PlatformDriver
{
	bool (*cb_get_env)(void);			/* Get platform environment and set directory paths. */
	PlatformDriverID id;				/* Platform driver ID. */
}
PlatformDriver;

/**************************************************************************************************
 * PlatformManager Structure
 *************************************************************************************************/

typedef struct PlatformManager
{
	bool	initialized;	/* Whether the platform manager is initialized. */
}
PlatformManager;

/**************************************************************************************************
 * PlatformDriver Array
 *************************************************************************************************/

extern const PlatformDriver* platform_drivers[];

/**************************************************************************************************
 * PlatformDriver Contexts
 *************************************************************************************************/

extern PlatformDriver unix_platform_driver;
extern PlatformDriver macos_platform_driver;
extern PlatformDriver win32_platform_driver;

/**************************************************************************************************
 * Platform Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

/* Platform Management Prototypes */

PlatformManager* GetPlatformManagerContext(void);

PlatformDriverID GetPlatformDriverID(void);

bool InitializePlatform(PlatformDriverID driver_id);
void DeinitializePlatform(void);

RETRO_END_DECLS

#endif