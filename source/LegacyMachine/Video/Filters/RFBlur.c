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

/*
* Credits - RFBlur adapted from Tilengine.
*
* Tilengine - The 2D retro graphics engine with raster effects.
* Copyright (C) 2015-2022 Marc Palacios Domenech <mailto:megamarc@hotmail.com>
* All rights reserved
* */

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "../CRTFilter.h"

/**************************************************************************************************
 * CRTFilter RFBlur Function
 *************************************************************************************************/

/* basic horizontal blur emulating RF blurring */
void RFBlur(uint8_t* scan, int width, int height, int pitch)
{
	int x, y;
	uint8_t* pixel;

	width -= 1;
	for (y = 0; y < height; y++)
	{
		pixel = scan;
		for (x = 0; x < width; x++)
		{
			pixel[0] = (pixel[0] + pixel[4]) >> 1;
			pixel[1] = (pixel[1] + pixel[5]) >> 1;
			pixel[2] = (pixel[2] + pixel[6]) >> 1;
			pixel += 4;
		}
		scan += pitch;
	}
}