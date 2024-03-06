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
#include <string/stdstring.h>

#include "../Logging.h"
#include "MenuManager.h"

/**************************************************************************************************
 * MenuManager Context
 *************************************************************************************************/

static MenuManager menu_manager = { 0 };

/**************************************************************************************************
 * MenuManager Functions
 *************************************************************************************************/

/* Returns the current frontend menu manager context. */
MenuManager* GetMenuManagerContext(void)
{
	return &menu_manager;
}

/* Gets the frontend menu's Audio/Video info struct actively being used for rendering. */
struct retro_system_av_info GetMenuAudioVideoInfo(void)
{
	MenuManager* menu = GetMenuManagerContext();

	LMC_SetLastError(LMC_ERR_OK);
	return menu->av_info;
}
