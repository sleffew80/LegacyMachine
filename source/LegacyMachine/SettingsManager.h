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

#ifndef _SETTINGS_MANAGER_H
#define _SETTINGS_MANAGER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"

/**************************************************************************************************
 * SettingsManager Structure
 *************************************************************************************************/

typedef struct SettingsManager
{
	char program_name[NAME_MAX_LENGTH];
	char main_directory[PATH_MAX_LENGTH];
	char setting_directory[PATH_MAX_LENGTH];
	char asset_directory[PATH_MAX_LENGTH];
	char core_directory[PATH_MAX_LENGTH];
	char content_directory[PATH_MAX_LENGTH];
	char system_directory[PATH_MAX_LENGTH];
	char save_directory[PATH_MAX_LENGTH];
	char state_directory[PATH_MAX_LENGTH];
}
SettingsManager;

/**************************************************************************************************
 * SettingsManager Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

SettingsManager* GetSettingsManagerContext(void);

RETRO_END_DECLS

#endif