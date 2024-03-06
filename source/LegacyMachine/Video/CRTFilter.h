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

#ifndef _CRT_FILTER_H
#define _CRT_FILTER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"
#include "../Common/Common.h"

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

typedef struct
{
	const uint8_t* mask;
	int width;
	int height;
	int glow;
}
Pattern;

extern Pattern patterns[];

/**************************************************************************************************
 * CRTFilter Structure
 *************************************************************************************************/

typedef enum
{
	CRT_SLOT,
	CRT_APERTURE,
	CRT_SHADOW,
}
CRTType;

typedef struct CRTFilter
{
	bool		(*cb_config_crt)(LMC_CRT, bool);
	void		(*cb_enable_rf)(bool);
	void		(*cb_toggle_crt)(void);
	void		(*cb_deinit_crt)(void);
	CRTType		type;
	bool		blur;
	bool		enabled;
}
CRTFilter;

RETRO_BEGIN_DECLS

void RFBlur(uint8_t* scan, int width, int height, int pitch);

RETRO_END_DECLS

#endif