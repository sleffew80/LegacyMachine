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

#ifndef _CRT_EFFECT_H
#define _CRT_EFFECT_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"
#include "Blitters/Blitters.h"

/**************************************************************************************************
 * Definitions
 *************************************************************************************************/

#define ZERO	0x10
#define SCAN	0x20
#define RED		0xFF,ZERO,ZERO,0xFF
#define GREEN	ZERO,0xFF,ZERO,0xFF
#define BLUE	ZERO,ZERO,0xFF,0xFF
#define BLACK	0x00,0x00,0x00,0xFF
#define WHITE	0xFF,0xFF,0xFF,0xFF

/**************************************************************************************************
 * Effect Patterns
 *************************************************************************************************/

extern const uint8_t pattern_slot[];
extern const uint8_t pattern_aperture[];
extern const uint8_t pattern_shadow[];
extern const uint8_t pattern_scanline[];

/**************************************************************************************************
 * Pattern Structure
 *************************************************************************************************/

typedef struct Pattern
{
	const uint8_t*	mask;	/* Pointer to the pattern pixel data. */
	int				width;	/* Pattern tile width in pixels. */
	int				height;	/* Pattern tile height in pixels. */
	int				glow;	/* Glow intensity value. */
}
Pattern;

extern Pattern patterns[];

/**************************************************************************************************
 * CRTEffect Structure
 *************************************************************************************************/

typedef enum
{
	CRT_SLOT = 0,
	CRT_APERTURE,
	CRT_SHADOW,
}
CRTMask;

typedef struct CRTEffect
{
	void		(*cb_toggle_crt)(void);	/* Callback to toggle the CRT effect on/off. */
	CRTMask		type;					/* Active CRT mask type. */
	bool		enabled;				/* Whether the CRT effect is currently enabled. */
}
CRTEffect;

#endif