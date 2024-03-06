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

#ifndef _PLATFORM_DRIVER_H
#define _PLATFORM_DRIVER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"

/**************************************************************************************************
 * PlatformDriver Structure
 *************************************************************************************************/

typedef struct PlatformDriver
{
	void		(*cb_get_env)(void);
	bool		initialized;
}
PlatformDriver;

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
 * PlatformDriver Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

PlatformDriver* InitializePlatformDriver(void);

RETRO_END_DECLS

#endif