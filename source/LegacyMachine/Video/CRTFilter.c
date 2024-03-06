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
#include "CRTFilter.h"

/**************************************************************************************************
 * Effect Patterns
 *************************************************************************************************/

const uint8_t pattern_slot[] =
{
	RED,   GREEN, BLUE,  RED,   GREEN, BLUE,
	RED,   GREEN, BLUE,  BLACK, BLACK, BLACK,
	RED,   GREEN, BLUE,  RED,   GREEN, BLUE,
	BLACK, BLACK, BLACK, RED,   GREEN, BLUE,
};

const uint8_t pattern_aperture[] =
{
	RED, GREEN, BLUE
};

const uint8_t pattern_shadow[] =
{
	RED, GREEN, BLUE,
	GREEN, BLUE, RED,
	BLUE, RED, GREEN,
};

const uint8_t pattern_scanline[] =
{
	WHITE,
	SCAN, SCAN, SCAN, 0xFF
};

Pattern patterns[] =
{
	{pattern_slot, 6, 4, 140},
	{pattern_aperture, 3, 1, 204},
	{pattern_shadow, 3, 3, 204},
};