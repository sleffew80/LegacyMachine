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
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"

/**************************************************************************************************
 * Definitions
 *************************************************************************************************/

/* Input definitions. */

#define MAX_PLAYERS		   8	/* Number of unique players. */
#define MAX_INPUTS		  32	/* Number of inputs per player. */
#define MAX_HATS		   2	/* Number of hats per player. */
#define MAX_AXES		   8	/* Number of axes per player. */
#define INPUT_MASK	(MAX_INPUTS - 1)

/**************************************************************************************************
 * Common Types/Structures
 *************************************************************************************************/

/* Structure for storing a color value. */
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

/* Structure for storing a rectangular area of a screen. */
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

/* Structure for storing 2D dimensions. */
typedef struct
{
    /* Object width in pixels. */
	int width;

    /* Object height in pixels. */
	int height;
}
Size2D;

/**************************************************************************************************
 * Common Video Types/Structures
 *************************************************************************************************/

/* Union for parsing CWF flags into separate values. */
typedef union
{
    uint8_t value;
    struct
    {
        bool fullscreen : 1;
        bool vsync : 1;
        uint8_t factor : 4;
        bool nearest : 1;
    };
}
WindowFlags;

/* Structure for storing parameter info directly related to managing a window or
screen which gets rendered to. */
typedef struct WindowInfo
{
    char*           title;              /* Window title. */
    float			override_aspect;    /* Override window aspect to force. */
    int				override_width;     /* Override window width to force. */
    int				override_height;    /* Override window height to force. */
    int				width;              /* Window width. */
    int				height;             /* Window height. */
    int				factor;             /* Window scale factor. */
    int				identifier;         /* Window identifier. */
    int				instances;          /* Instances of window. */
    volatile int	return_value;       
    bool			running;            /* Is window currently running. */
}
WindowInfo;

/* Structure for storing viewport dimensions. */
typedef struct ViewportInfo
{
    int x, y;
    int w, h;
}
ViewportInfo;

/* Structure for storing a frame's pixel data and related info. */
typedef struct FrameInfo
{
    uint8_t* data;      /* Frame image pixel data. */
    unsigned width;     /* Width of frame image in pixels. */
    unsigned height;    /* Height of frame image in pixels. */
    unsigned pitch;     /* Pitch of frame image. */
    unsigned format;    /* Pixel format of frame image. */
    unsigned type;      /* Pixel type of frame image. */
    unsigned depth;     /* Color depth of frame image in bits per pixel. */
}
FrameInfo;

/* Structure for storing info directly related to rendering video. */
typedef struct VideoInfo
{
    float       aspect_ratio;   /* Aspect ratio to render in. */
    unsigned    max_width;      /* Maximum expected width to render. */
    unsigned    max_height;     /* Maximum expected height to render. */
    FrameInfo*  frame;          /* Frame data and related info. */
    bool        fullscreen;     /* Render in fullscreen. */
    bool        vsync;          /* Enable vertical syncing. */
    bool        smooth;         /* Smooth rendered frames. */
}
VideoInfo;

/**************************************************************************************************
 * Common Input Types/Structures
 *************************************************************************************************/

/* Structure for managing the state of a joypad input device. */
typedef struct JoypadInputState
{
    uint32_t    key_map[MAX_INPUTS];
    uint32_t    inputs;
    int32_t     product;
    int32_t     vendor;
    uint8_t     button_map[MAX_INPUTS];
    uint8_t     hat_map[MAX_HATS][MAX_HAT_INPUTS];
    uint8_t     axis_map[MAX_AXES][MAX_AXIS_INPUTS];
    uint8_t     buttons;
    uint8_t     axes;
    uint8_t     hats;
    uint8_t     identifier;
    const char* name;
    bool        keyboard_enabled;
    bool        connected;
}
JoypadInputState;

#endif