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

#ifndef __COMMON_H_
#define __COMMON_H_

/**************************************************************************************************
 * Common Structures
 *************************************************************************************************/

typedef struct
{
    /* Red value (range 0-255). */
    int r;

    /* Green value (range 0-255). */
    int g;

    /* Blue value (range 0-255). */
    int b;

    /* Alpha value (range 0-255). */
    int a;
}
Color;

typedef struct
{
    /* Top left x coordinate. */
    int x;

    /* Top left y coordinate. */
    int y;

    /* Rectangle width. */
    int width;

    /* Rectangle height. */
    int height;
}
RectangleArea;

typedef struct
{
    /* Object width in pixels. */
	int width;

    /* Object height in pixels. */
	int height;
}
Size2D;

#endif