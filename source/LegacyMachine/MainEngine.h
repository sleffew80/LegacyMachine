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

#ifndef _MAIN_ENGINE_H
#define _MAIN_ENGINE_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"
#include "SettingsManager.h"
#include "Platform/PlatformDriver.h"
#include "Window/WindowDriver.h"
#include "Video/VideoDriver.h"
#include "Audio/AudioDriver.h"
#include "Input/InputDriver.h"
#if defined HAVE_MENU
#include "Menu/MenuManager.h"
#endif
#include "SystemManager.h"
#include "CoreLibrary.h"

/**************************************************************************************************
 * MainEngine Structure
 *************************************************************************************************/

/* Main Engine */
typedef struct MainEngine
{
	uint32_t				header;		/* Object signature to identify as engine context */

	SettingsManager*		settings;	/* Pointer to settings manager. */
	PlatformDriver*			platform;	/* Pointer to platform driver. */
#if defined HAVE_MENU
	MenuManager*			menu;		/* Pointer to frontend menu manager. */
#endif
	SystemManager*			system;		/* Pointer to libretro system manager. */	
	WindowDriver*			window;		/* Pointer to window driver. */
	VideoDriver*			video;		/* Pointer to video driver. */
	AudioDriver*			audio;		/* Pointer to audio driver. */
	InputDriver*			input;		/* Pointer to input driver. */

	LMC_Error				error;		/* Last error code. */
	LMC_LogLevel			log_level;	/* Logging level. */

	int						frame;		/* Current frame number. */
}
MainEngine;

/**************************************************************************************************
 * MainEngine Context
 *************************************************************************************************/

extern MainEngine* legacy_machine;

#endif