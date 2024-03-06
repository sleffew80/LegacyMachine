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

#ifndef _AUDIO_DRIVER_H
#define _AUDIO_DRIVER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"

/**************************************************************************************************
 * AudioDriver Structure
 *************************************************************************************************/

typedef struct AudioDriver
{
	void		(*cb_init)(int);
	size_t		(*cb_write)(const int16_t*, unsigned);
	void		(*cb_deinit)(void);
	bool		initialized;
}
AudioDriver;

/**************************************************************************************************
 * AudioDriver Array
 *************************************************************************************************/

extern const AudioDriver* audio_drivers[];

/**************************************************************************************************
 * AudioDriver Contexts
 *************************************************************************************************/

extern AudioDriver sdl2_audio_driver;

/**************************************************************************************************
 * AudioDriver Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

AudioDriver* InitializeAudioDriver(void);

RETRO_END_DECLS

#endif