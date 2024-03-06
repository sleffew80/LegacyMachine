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

#ifndef _MENU_MANAGER_H
#define _MENU_MANAGER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"

/**************************************************************************************************
 * MenuManager Structure
 *************************************************************************************************/

typedef struct
{
	TLN_Engine tile_engine;

	void (*cb_update)(int);		/* Menu update callback. */

	struct retro_system_av_info av_info; /* Menu audio/video information. */

	struct
	{
		int	   pitch;			
		void*  data;
	}
	framebuffer;				/* Framebuffer for menu. */

	retro_time_t frame_delay;	/* Menu frame delay interval. */
	retro_time_t frame_trigger; /* Menu frame trigger to update next frame. */
	retro_time_t frame_time;	/* Menu total frame time elapsed. */
	bool		 available;		/* True if menu is configured and available. Otherwise, false. */
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