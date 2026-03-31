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

#ifndef _MENU_MANAGER_H
#define _MENU_MANAGER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"

/**************************************************************************************************
 * MenuManager Structure
 *************************************************************************************************/

typedef struct MenuManager
{
	/* Define may seem redundant but it is necessary to satisfy certain compilers. */
#if defined HAVE_MENU
	TLN_Engine	tile_engine;					/* Tilengine instance for menu rendering. */
#endif

	void		(*cb_update)(int);				/* Menu update callback. */

	struct retro_system_av_info		av_info;	/* Menu audio/video information. */

	struct
	{
		int		pitch;	/* Frame pitch in bytes (bytes per row). */
		void*	data;	/* Frame pixel data buffer. */
	}
	frame;						/* Frame data for menu. */

	bool		available;		/* True if menu is configured and available. Otherwise, false. */
}
MenuManager;

/**************************************************************************************************
 * MenuManager Function Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

MenuManager* GetMenuManagerContext(void);
struct retro_system_av_info GetMenuAudioVideoInfo(void);

RETRO_END_DECLS

#endif