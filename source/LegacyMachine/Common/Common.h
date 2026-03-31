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

#ifndef _COMMON_H
#define _COMMON_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#if defined HAVE_THREADS
#include <rthreads/rthreads.h>
#endif

#include "LegacyMachine.h"

/**************************************************************************************************
 * Definitions
 *************************************************************************************************/

/* Common value definitions. */

#define VALUE_MAX_LENGTH 32		/* Maximum length of string values. */

/* Audio definitions. */

#define MAX_LATENCY 512         /* Maximum audio latency in milliseconds. */

/* Video definitions. */

#define ADJ_VRES 480		/* Standard vertical resolution to adjust to for frame normalization. */

#define ADJ_3_2_HRES 720	/* Adjusted horizontal resolution for 3:2 aspect ratio. */
#define ADJ_4_3_HRES 640	/* Adjusted horizontal resolution for 4:3 aspect ratio. */
#define ADJ_5_4_HRES 600	/* Adjusted horizontal resolution for 5:4 aspect ratio. */
#define ADJ_16_9_HRES 854	/* Adjusted horizontal resolution for 16:9 aspect ratio. */

/* Input definitions. */

#define MAX_PLAYERS		   8	/* Number of unique players. */
#define MAX_INPUTS		  32	/* Number of inputs per player. */
#define MAX_HATS		   2	/* Number of hats per player. */
#define MAX_AXES		   8	/* Number of axes per player. */
#define INPUT_MASK	(MAX_INPUTS - 1)

#define MOUSE_INPUT_LEFT   1	/* Left mouse button. */
#define MOUSE_INPUT_MIDDLE 2	/* Middle mouse button. */
#define MOUSE_INPUT_RIGHT  3	/* Right mouse button. */
#define MOUSE_INPUT_X1     4	/* Extra mouse button 1. */
#define MOUSE_INPUT_X2     5	/* Extra mouse button 2. */

/**************************************************************************************************
 * Common Types/Structures
 *************************************************************************************************/

/* Structure for storing a color value. */
typedef struct Color
{
	uint8_t r; /* Red value (range 0-255). */
	uint8_t g; /* Green value (range 0-255). */
	uint8_t b; /* Blue value (range 0-255). */
	uint8_t a; /* Alpha value (range 0-255). */
}
Color;

/* Structure for storing 2 dimensions. */
typedef struct
{
	int width;  /* Object width in pixels. */
	int height; /* Object height in pixels. */
}
Size2D;

/* Structure for storing vertices in 2D. */
typedef struct
{
	float x;
	float y;
}
Vector2D;

/**************************************************************************************************
 * Common Audio Structures
 *************************************************************************************************/

typedef struct AudioInfo
{
	unsigned	frequency;			/* Audio frequency in hertz. */
	unsigned	latency;			/* Audio latency in milliseconds. */
	unsigned	minimum_latency;	/* Minimum audio latency in milliseconds. */
}
AudioInfo;

/**************************************************************************************************
 * Common Video Types/Structures
 *************************************************************************************************/

typedef enum
{
	/* Raw frame output stretched to menu or core provided aspect ratio. */
	OUTPUT_RAW = 0,
	/* Adjust output to a standard height and stretch horizontally to an aspect ratio.
	   of 3:2. */
	OUTPUT_ADJUSTED_3_2,
	/* Adjust output to a standard height and stretch horizontally to an aspect ratio.
	   of 4:3. */
	OUTPUT_ADJUSTED_4_3,
	/* Adjust output to a standard height and stretch horizontally to an aspect ratio.
	   of 5:4. */
	OUTPUT_ADJUSTED_5_4,
	/* Adjust output to a standard height and stretch horizontally to an aspect ratio.
	   of 16:9. */
	OUTPUT_ADJUSTED_16_9,
}
VideoOutput;

/* Union for parsing CWF flags into separate values. */
typedef union
{
	uint8_t value;
	struct
	{
		bool fullscreen : 1;
		bool vsync : 1;
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
	volatile int	return_value;       /* Return value of window. */
}
WindowInfo;

/* Structure for storing viewport dimensions. */
typedef struct ViewportInfo
{
	int x, y;
	int w, h;
}
ViewportInfo;

/* Structure for storing adjustment info for frame normalization. */
typedef struct AdjustmentInfo
{
	int     upper_pad_lines; /* Upper padding in lines (negative = crop). */
	int     lower_pad_lines; /* Lower padding in lines (negative = crop). */
	int     upper_pad_size;  /* Upper padding size in bytes. */
	int     lower_pad_size;  /* Lower padding size in bytes. */
	bool    adjust;          /* Frame adjustment required. */
}
AdjustmentInfo;

/* Structure for storing a frame's pixel data and related info. */
typedef struct FrameInfo
{
	uint8_t* data;      /* Frame image pixel data. */
	unsigned width;     /* Width of frame image in pixels. */
	unsigned height;    /* Height of frame image in pixels. */
	unsigned pitch;     /* Pitch of frame image. */
	unsigned format;    /* Pixel format of frame image. */
	unsigned depth;     /* Color depth of frame image in bits per pixel. */
	unsigned size;      /* Frame data size in bytes. */
}
FrameInfo;

/* Structure for storing info directly related to rendering video. */
typedef struct RenderInfo
{
	/* Contains pointer to raw frame data supplied by tilengine menu, libretro core or, optionally,
	   an additional framebuffer for pixel conversions. */
	FrameInfo       raw_frame;

	/* Contains pointer to normalized adjusted frame data. Unless the output type is set to "raw"
	   and the crt effect is disabled, all frames with a vertical resolution less than 525px get
	   padded or cropped to a resolution that is a multiple of 60 - i.e. 60, 120, 240, and 480.
	   In cases where the raw frame does not require any adjustments, this simply points to the
	   raw frame. */
	FrameInfo       source_frame;

	/* Contains pointer to final output frame to display on screen. Could be a pointer directly
	   to the source frame or a pointer to a framebuffer for CRT effect filter image post
	   processing. */
	FrameInfo       output_frame;

	AdjustmentInfo  frame_adjust;   /* Frame adjustment info - if required. */
	VideoOutput     type;           /* How video is output to display - raw or adjusted. */
	float           aspect_ratio;   /* Aspect ratio to render in. */
	unsigned        max_width;      /* Maximum expected width to render. */
	unsigned        max_height;     /* Maximum expected height to render. */
	unsigned        size;           /* Initial size of raw frame from libretro or menu. */
	bool		    bpp32;          /* Driver requires 32 bits per pixel. */
	bool            fullscreen;     /* Render in fullscreen. */
	bool            vsync;          /* Enable vertical syncing. */
	bool            smooth;         /* Smooth rendered frames. */
	bool            current;        /* Framebuffers and values are current and have all been
									   initialized. */
}
RenderInfo;

/**************************************************************************************************
 * Common Input Types/Structures
 *************************************************************************************************/

/* Structure containing information about a keyboard device. */
typedef struct KeyboardInfo
{
	const char* name; /* Name of keyboard device. */
	int keys;         /* Number of keys. */
}
KeyboardInfo;

/* Structure for managing the state of the keyboard. */
typedef struct KeyboardInputState
{
	/* Information about the keyboard device. */
	KeyboardInfo info;
	/* Map of internal key codes to driver's key codes. */
	uint32_t* key_codes;
	/* Map of logical menu interface keys to driver provided keycodes. */
	uint32_t key_map[MAX_KEYBOARD_INPUTS];
	/* Pressed/Released state of each key. */
	bool key_state[RETROK_LAST];
}
KeyboardInputState;

/* Structure containing information about a mouse device. */
typedef struct MouseInfo
{
	const char* name; /* Name of mouse device. */
}
MouseInfo;

/* Structure for managing the state of a mouse input device. */
typedef struct MouseInputState
{
	MouseInfo info;         /* Information about the mouse device. */
	int16_t position_x;     /* Current X position of the mouse. */
	int16_t position_y;     /* Current Y position of the mouse. */
	int16_t position_abs_x; /* Absolute X position of the mouse. */
	int16_t position_abs_y; /* Absolute Y position of the mouse. */
	int16_t button_left;    /* State of the left mouse button. */
	int16_t button_middle;  /* State of the middle mouse button. */
	int16_t button_right;   /* State of the right mouse button. */
	int16_t button_4;       /* State of the 4th mouse button. */
	int16_t button_5;       /* State of the 5th mouse button. */
	int16_t wheel_up;       /* State of the mouse wheel (up). */
	int16_t wheel_down;     /* State of the mouse wheel (down). */
	int16_t wheel_left;     /* State of the mouse wheel (left/horizontal negative). */
	int16_t wheel_right;    /* State of the mouse wheel (right/horizontal positive). */
	uint8_t identifier;     /* Unique identifier for the mouse. */
	bool connected;         /* Connection status of the mouse. */
}
MouseInputState;

/* Structure containing information about a joypad device. */
typedef struct JoypadInfo
{
	const char* name;		/* Name of joypad device. */
	int32_t     product;	/* Product ID of joypad device. */
	int32_t     vendor;		/* Vendor ID of joypad device. */
	uint8_t     buttons;	/* Number of buttons. */
	uint8_t     axes;		/* Number of axes. */
	uint8_t     hats;		/* Number of hat switches. */
}
JoypadInfo;

/* Structure for managing the state of a joypad input device. */
typedef struct JoypadInputState
{
	/* Information about the joypad device. */
	JoypadInfo  info;
	/* Map of physical keyboard keys, via internal key codes, to logical joypad actions. */
	uint32_t    key_map[MAX_INPUTS];
	/* Map of physical joypad buttons to logical joypad actions. */
	uint8_t     button_map[MAX_INPUTS];
	/* Map of physical hat switch positions, by index, to logical joypad actions. */
	uint8_t     hat_map[MAX_HATS][MAX_HAT_INPUTS];
	/* Map of physical axis states, by index, to logical joypad actions
	   (used for maping axis directions as digital inputs). */
	uint8_t     axis_map[MAX_AXES][MAX_AXIS_INPUTS];
	/* Map of physical axis indices to logical axis indices
	   (used for mapping axes as analog inputs). */
	uint8_t	    axis_indices[MAX_AXES];
	/* Array indicating whether each axis is analog or digital. */
	bool        axis_types[MAX_AXES];
	/* Deadzone value for analog input. */
	int16_t     analog_deadzone;
	/* Threshold value when using analog input as digital input. */
	int16_t     analog_threshold;
	/* Bitfield state of currently active inputs. */
	uint32_t    digital_inputs;
	/* Array of current values of each analog axis. */
	int16_t		analog_values[MAX_AXES];
	/* Unique identifier for the joypad. */
	int32_t     identifier;
	/* Connection status of the joypad. */
	bool        connected;
}
JoypadInputState;

#endif
