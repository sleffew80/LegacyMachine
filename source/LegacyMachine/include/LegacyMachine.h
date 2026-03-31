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

/*
* Credits: 
*
* LegacyMachine uses the Libretro API and Libretro SDK.
* 
* Copyright (C) 2010-2026 The RetroArch team
* 
* LegacyMachine uses Tilengine for rendering menus.
* For simplicity, LegacyMachine is released under the same terms of the 
* Mozilla Public License that Tilengine is released under.
*
* Tilengine - The 2D retro graphics engine with raster effects.
* Copyright (C) 2015-2024 Marc Palacios Domenech <mailto:megamarc@hotmail.com>
* All rights reserved
* */

#ifndef _LEGACY_MACHINE_H
#define _LEGACY_MACHINE_H

/* LegacyMachine Library API */
#if defined LEGACY_MACHINE_STATIC
#define LMCAPI
#elif defined _MSC_VER
#if defined LEGACY_MACHINE_EXPORTS
#define LMCAPI __declspec(dllexport)
#else
#define LMCAPI __declspec(dllimport)
#endif
#else
#if defined LEGACY_MACHINE_EXPORTS
#define LMCAPI __attribute__((visibility("default")))
#else
#define LMCAPI
#endif
#endif

#include <stdio.h>

#include <libretro.h>
#include <retro_common_api.h>
#include <retro_miscellaneous.h>

#if defined HAVE_MENU
#include <Tilengine.h>
#endif

#include "../LibraryVersion.h"

/* Version */
#define LEGACY_MACHINE_HEADER_VERSION ((LEGACY_MACHINE_VER_MAJ << 16) | (LEGACY_MACHINE_VER_MIN << 8) | LEGACY_MACHINE_VER_REV)

/*! 
 * \brief
 * Types of built-in CRT masks for the CRT effect.
 *
 * \see 
 * LMC_SetCRTMask()
 * 
 */
typedef enum
{
	LMC_CRT_SLOT = 0,	/*!< Slot mask without scanlines. */
	LMC_CRT_APERTURE,	/*!< Aperture grille with scanlines (matrix-like dot arrangement). */
	LMC_CRT_SHADOW,		/*!< Shadow mask with scanlines, diagonal subpixel arrangement. */
}
LMC_CRTMask;

typedef struct MainEngine* LMC_Engine;		/*!< Engine context. */

/*! 
 * \brief
 * Standard paths. 
 *
 * \see 
 * LMC_GetPath()
 * 
 */
typedef enum
{
	LMC_PATH_MAIN = 0,	/*!< Application executable path. */
	LMC_PATH_SETTINGS,	/*!< Settings path. */
	LMC_PATH_ASSETS,	/*!< Assets path. */
	LMC_PATH_CORE,		/*!< Libretro core path. */
	LMC_PATH_CONTENT,	/*!< Libretro content path. */
	LMC_PATH_SYSTEM,	/*!< Libretro system files path. */
	LMC_PATH_SAVES,		/*!< SRAM and memory card data save path.*/
	LMC_PATH_STATES,	/*!< State save path. */
	MAX_SYSTEM_PATHS
}
LMC_Path;

/*! 
 * \brief Standard save state slots. 
 *
 * \see
 * LMC_SaveState(), LMC_LoadState()
 * 
 */
typedef enum
{
	LMC_SLOT_QUICK = 0,
	LMC_SLOT_1,
	LMC_SLOT_2,
	LMC_SLOT_3,
	LMC_SLOT_4,
	MAX_STATE_SLOTS
}
LMC_Slot;

/*! 
 * \brief
 * Player index for input assignment functions.
 *
 * \see
 * LMC_DefineJoypadKeyInput(), LMC_DefineJoypadButtonInput(), LMC_AssignJoypadHatIndex(), 
 * LMC_AssignJoypadAxisIndex()
 * 
 */
typedef enum
{
	LMC_PLAYER_1 = 0,	/*!< Player 1 */
	LMC_PLAYER_2,		/*!< Player 2 */
	LMC_PLAYER_3,		/*!< Player 3 */
	LMC_PLAYER_4,		/*!< Player 4 */
	LMC_PLAYER_5,		/*!< Player 5 */
	LMC_PLAYER_6,		/*!< Player 6 */
	LMC_PLAYER_7,		/*!< Player 7 */
	LMC_PLAYER_8		/*!< Player 8 */
}
LMC_Player;

/*! 
 * \brief 
 * Standard key codes for setting keyboard input. 
 *
 * \see
 * LMC_DefineKeyboardKeyInput()
 * 
 */
typedef enum
{
	LMC_KEY_UNKNOWN = 0,		/*!< Unknown key. */
	LMC_KEY_NONE = 0,			/*!< No key. */
	LMC_KEY_BACKSPACE,			/*!< Backspace key. */
	LMC_KEY_TAB,				/*!< Tab key. */
	LMC_KEY_CLEAR,				/*!< Clear key. */
	LMC_KEY_RETURN,				/*!< Return/Enter key. */
	LMC_KEY_PAUSE,				/*!< Pause key. */
	LMC_KEY_ESCAPE,				/*!< Escape key. */
	LMC_KEY_SPACE,				/*!< Space key. */
	LMC_KEY_EXCLAIM,			/*!< Exclamation mark key. */
	LMC_KEY_QUOTEDBL,			/*!< Double quote key. */
	LMC_KEY_HASH,				/*!< Hash mark key. */
	LMC_KEY_DOLLAR,				/*!< Dollar sign key. */
	LMC_KEY_AMPERSAND,			/*!< Ampersand key. */
	LMC_KEY_QUOTE,				/*!< Single quote key. */
	LMC_KEY_LEFTPAREN,			/*!< Left parenthesis key. */
	LMC_KEY_RIGHTPAREN,			/*!< Right parenthesis key. */
	LMC_KEY_ASTERISK,			/*!< Asterisk key. */
	LMC_KEY_PLUS,				/*!< Plus sign key. */
	LMC_KEY_COMMA,				/*!< Comma key. */
	LMC_KEY_MINUS,				/*!< Minus sign key. */
	LMC_KEY_PERIOD,				/*!< Period key. */
	LMC_KEY_SLASH,				/*!< Forward slash key. */
	LMC_KEY_0,					/*!< Number 0 key. */
	LMC_KEY_1,					/*!< Number 1 key. */
	LMC_KEY_2,					/*!< Number 2 key. */
	LMC_KEY_3,					/*!< Number 3 key. */
	LMC_KEY_4,					/*!< Number 4 key. */
	LMC_KEY_5,					/*!< Number 5 key. */
	LMC_KEY_6,					/*!< Number 6 key. */
	LMC_KEY_7,					/*!< Number 7 key. */
	LMC_KEY_8,					/*!< Number 8 key. */
	LMC_KEY_9,					/*!< Number 9 key. */
	LMC_KEY_COLON,				/*!< Colon key. */
	LMC_KEY_SEMICOLON,			/*!< Semi-colon key. */
	LMC_KEY_LESS,				/*!< Less than sign key. */
	LMC_KEY_EQUALS,				/*!< Equals sign key. */
	LMC_KEY_GREATER,			/*!< Greater than sign key. */
	LMC_KEY_QUESTION,			/*!< Question mark key. */
	LMC_KEY_AT,					/*!< At sign key. */
	LMC_KEY_LEFTBRACKET,		/*!< Left bracket key. */
	LMC_KEY_BACKSLASH,			/*!< Back slash key. */
	LMC_KEY_RIGHTBRACKET,		/*!< Right bracket key. */
	LMC_KEY_CARET,				/*!< Caret key. */
	LMC_KEY_UNDERSCORE,			/*!< Underscore key. */
	LMC_KEY_BACKQUOTE,			/*!< Back quote key. */
	LMC_KEY_A,					/*!< Letter a key. */
	LMC_KEY_B,					/*!< Letter b key. */
	LMC_KEY_C,					/*!< Letter c key. */
	LMC_KEY_D,					/*!< Letter d key. */
	LMC_KEY_E,					/*!< Letter e key. */
	LMC_KEY_F,					/*!< Letter f key. */
	LMC_KEY_G,					/*!< Letter g key. */
	LMC_KEY_H,					/*!< Letter h key. */
	LMC_KEY_I,					/*!< Letter i key. */
	LMC_KEY_J,					/*!< Letter j key. */
	LMC_KEY_K,					/*!< Letter k key. */
	LMC_KEY_L,					/*!< Letter l key. */
	LMC_KEY_M,					/*!< Letter m key. */
	LMC_KEY_N,					/*!< Letter n key. */
	LMC_KEY_O,					/*!< Letter o key. */
	LMC_KEY_P,					/*!< Letter p key. */
	LMC_KEY_Q,					/*!< Letter q key. */
	LMC_KEY_R,					/*!< Letter r key. */
	LMC_KEY_S,					/*!< Letter s key. */
	LMC_KEY_T,					/*!< Letter t key. */
	LMC_KEY_U,					/*!< Letter u key. */
	LMC_KEY_V,					/*!< Letter v key. */
	LMC_KEY_W,					/*!< Letter w key. */
	LMC_KEY_X,					/*!< Letter x key. */
	LMC_KEY_Y,					/*!< Letter y key. */
	LMC_KEY_Z,					/*!< Letter z key. */
	LMC_KEY_LEFTBRACE,			/*!< Left brace key. */
	LMC_KEY_BAR,				/*!< Bar key. */
	LMC_KEY_RIGHTBRACE,			/*!< Right brace key. */
	LMC_KEY_TILDE,				/*!< Tilde key. */
	LMC_KEY_DELETE,				/*!< Delete key. */
	LMC_KEY_KP0,				/*!< Keypad number 0 key. */
	LMC_KEY_KP1,				/*!< Keypad number 1 key. */
	LMC_KEY_KP2,				/*!< Keypad number 2 key. */
	LMC_KEY_KP3,				/*!< Keypad number 3 key. */
	LMC_KEY_KP4,				/*!< Keypad number 4 key. */
	LMC_KEY_KP5,				/*!< Keypad number 5 key. */
	LMC_KEY_KP6,				/*!< Keypad number 6 key. */
	LMC_KEY_KP7,				/*!< Keypad number 7 key. */
	LMC_KEY_KP8,				/*!< Keypad number 8 key. */
	LMC_KEY_KP9,				/*!< Keypad number 9 key. */
	LMC_KEY_KP_PERIOD,			/*!< Keypad period key. */
	LMC_KEY_KP_DIVIDE,			/*!< Keypad division sign key. */
	LMC_KEY_KP_MULTIPLY,		/*!< Keypad multiplication sign key. */
	LMC_KEY_KP_MINUS,			/*!< Keypad minus sign key. */
	LMC_KEY_KP_PLUS,			/*!< Keypad plus sign key. */
	LMC_KEY_KP_ENTER,			/*!< Keypad enter key. */
	LMC_KEY_KP_EQUALS,			/*!< Keypad equal sign key. */
	LMC_KEY_UP,					/*!< Up key. */
	LMC_KEY_DOWN,				/*!< Down key. */
	LMC_KEY_RIGHT,				/*!< Right key. */
	LMC_KEY_LEFT,				/*!< Left key. */
	LMC_KEY_INSERT,				/*!< Insert key. */
	LMC_KEY_HOME,				/*!< Home key. */
	LMC_KEY_END,				/*!< End key. */
	LMC_KEY_PAGEUP,				/*!< Page up key. */
	LMC_KEY_PAGEDOWN,			/*!< Page down key. */
	LMC_KEY_F1,					/*!< Function 1 key. */
	LMC_KEY_F2,					/*!< Function 2 key. */
	LMC_KEY_F3,					/*!< Function 3 key. */
	LMC_KEY_F4,					/*!< Function 4 key. */
	LMC_KEY_F5,					/*!< Function 5 key. */
	LMC_KEY_F6,					/*!< Function 6 key. */
	LMC_KEY_F7,					/*!< Function 7 key. */
	LMC_KEY_F8,					/*!< Function 8 key. */
	LMC_KEY_F9,					/*!< Function 9 key. */
	LMC_KEY_F10,				/*!< Function 10 key. */
	LMC_KEY_F11,				/*!< Function 11 key. */
	LMC_KEY_F12,				/*!< Function 12 key. */
	LMC_KEY_F13,				/*!< Function 13 key. */
	LMC_KEY_F14,				/*!< Function 14 key. */
	LMC_KEY_F15,				/*!< Function 15 key. */
	LMC_KEY_NUMLOCK,			/*!< Num lock key. */
	LMC_KEY_CAPSLOCK,			/*!< Caps lock key. */
	LMC_KEY_SCROLLOCK,			/*!< Scroll lock key. */
	LMC_KEY_RSHIFT,				/*!< Right shift key. */
	LMC_KEY_LSHIFT,				/*!< Left shift key. */
	LMC_KEY_RCTRL,				/*!< Right control key. */
	LMC_KEY_LCTRL,				/*!< Left control key. */
	LMC_KEY_RALT,				/*!< Right alt key. */
	LMC_KEY_LALT,				/*!< Left alt key. */
	LMC_KEY_RMETA,				/*!< Right meta key. */
	LMC_KEY_LMETA,				/*!< Left meta key. */
	LMC_KEY_LSUPER,				/*!< Left windows/command key. */
	LMC_KEY_RSUPER,				/*!< Right windows/command key. */
	LMC_KEY_MODE,				/*!< Mode key. */
	LMC_KEY_COMPOSE,			/*!< Compose key. */
	LMC_KEY_HELP,				/*!< Help key. */
	LMC_KEY_PRINT,				/*!< Print screen key. */
	LMC_KEY_SYSREQ,				/*!< System request key. */
	LMC_KEY_BREAK,				/*!< Break key. */
	LMC_KEY_MENU,				/*!< Menu key. */
	LMC_KEY_POWER,				/*!< Power key. */
	LMC_KEY_EURO,				/*!< Euro sign key. */
	LMC_KEY_UNDO,				/*!< Undo key. */
	LMC_KEY_OEM_102,			/*!< Angle bracket key or the backslash key. */
	MAX_KEY_INPUTS,				/*!< Total key inputs. */
	SIZE_KEY_INPUTS = INT_MAX	/* Ensure enum is size of int. */
}
LMC_KeyCode;

/*!
 * \brief
 * Standard keyboard input actions for assigning as input actions.
 *
 * \see
 * LMC_DefineKeyboardKeyInput()
 *
 */
typedef enum
{
	LMC_KEYBOARD_INPUT_NONE = 0,		/*!< No input. */
	LMC_KEYBOARD_INPUT_QUIT,			/*!< Close LegacyMachine. */
	LMC_KEYBOARD_INPUT_HOME,			/*!< Return to menu from loaded content.*/
	LMC_KEYBOARD_INPUT_SAVE,			/*!< Save state. */
	LMC_KEYBOARD_INPUT_LOAD,			/*!< Load state. */
	LMC_KEYBOARD_INPUT_CRT,				/*!< CRT toggle. */
	LMC_KEYBOARD_INPUT_FULLSCREEN,		/*!< Fullscreen toggle. */
	MAX_KEYBOARD_INPUTS					/*!< Total keyboard input actions. */
}
LMC_KeyboardInput;

/*!
 * \brief
 * Standard input actions for querying the state of a given joypad input.
 *
 * \remarks
 * 
 * \see
 * LMC_GetJoypadInput(), LMC_DefineJoypadKeyInput(), LMC_DefineJoypadButtonInput(), 
 * LMC_DefineJoypadHatInput(), LMC_DefineJoypadAxisInput()
 *
 */
typedef enum
{
	LMC_JOYPAD_INPUT_NONE = 0,			/*!< No input. */
	LMC_JOYPAD_INPUT_B,					/*!< B action button. */
	LMC_JOYPAD_INPUT_Y,					/*!< Y action button. */
	LMC_JOYPAD_INPUT_SELECT,			/*!< Select button. */
	LMC_JOYPAD_INPUT_START,				/*!< Start button. */
	LMC_JOYPAD_INPUT_UP,				/*!< Up direction. */
	LMC_JOYPAD_INPUT_DOWN,				/*!< Down direction. */
	LMC_JOYPAD_INPUT_LEFT,				/*!< Left direction. */
	LMC_JOYPAD_INPUT_RIGHT,				/*!< Right direction. */
	LMC_JOYPAD_INPUT_A,					/*!< A action button. */
	LMC_JOYPAD_INPUT_X,					/*!< X action button. */
	LMC_JOYPAD_INPUT_L,					/*!< L action button. */
	LMC_JOYPAD_INPUT_R,					/*!< R action button. */
	LMC_JOYPAD_INPUT_L2,				/*!< L2 action button. */
	LMC_JOYPAD_INPUT_R2,				/*!< R2 action button. */
	LMC_JOYPAD_INPUT_L3,				/*!< L3 action button. */
	LMC_JOYPAD_INPUT_R3,				/*!< R3 action button. */
	LMC_JOYPAD_INPUT_HOME,				/*!< Home action button.*/
	LMC_JOYPAD_INPUT_L_ANALOG_UP,		/*!< Left analog up direction. */
	LMC_JOYPAD_INPUT_L_ANALOG_DOWN,		/*!< Left analog down direction. */
	LMC_JOYPAD_INPUT_L_ANALOG_LEFT,		/*!< Left analog left direction. */
	LMC_JOYPAD_INPUT_L_ANALOG_RIGHT,	/*!< Left analog right direction. */
	LMC_JOYPAD_INPUT_R_ANALOG_UP,		/*!< Right analog up direction. */
	LMC_JOYPAD_INPUT_R_ANALOG_DOWN,		/*!< Right analog down direction. */
	LMC_JOYPAD_INPUT_R_ANALOG_LEFT,		/*!< Right analog left direction. */
	LMC_JOYPAD_INPUT_R_ANALOG_RIGHT,	/*!< Right analog right direction. */

	LMC_JOYPAD_INPUT_P1 = (LMC_PLAYER_1 << 5),  /*!< Request player 1 input (default). */
	LMC_JOYPAD_INPUT_P2 = (LMC_PLAYER_2 << 5),	/*!< Request player 2 input. */
	LMC_JOYPAD_INPUT_P3 = (LMC_PLAYER_3 << 5),	/*!< Request player 3 input. */
	LMC_JOYPAD_INPUT_P4 = (LMC_PLAYER_4 << 5),	/*!< Request player 4 input. */
	LMC_JOYPAD_INPUT_P5 = (LMC_PLAYER_5 << 5),	/*!< Request player 5 input. */
	LMC_JOYPAD_INPUT_P6 = (LMC_PLAYER_6 << 5),	/*!< Request player 6 input. */
	LMC_JOYPAD_INPUT_P7 = (LMC_PLAYER_7 << 5),	/*!< Request player 7 input. */
	LMC_JOYPAD_INPUT_P8 = (LMC_PLAYER_8 << 5),	/*!< Request player 8 input. */
}
LMC_JoypadInput;

/*! 
 * \brief
 * Standard hat direction inputs for assigning as input actions.
 * 
 * \see
 * LMC_DefineJoypadHatInput()
 * 
 */
typedef enum
{
	LMC_HAT_CENTERED = 0,	/*!< Hat centered (no direction) action. */
	LMC_HAT_UP,				/*!< Hat up action. */
	LMC_HAT_RIGHT,			/*!< Hat right action. */
	LMC_HAT_DOWN,			/*!< Hat down action. */
	LMC_HAT_LEFT,			/*!< Hat left action. */
	MAX_HAT_INPUTS			/*!< Total hat inputs. */
}
LMC_HatDirection;

/*!
 * \brief
 * Standard axis direction position inputs for assigning as input actions.
 *
 * \see
 * LMC_DefineJoypadAxisInput()
 *
 */
typedef enum
{
	LMC_AXIS_NEG = 0,	/*!< Axis negative direction action. */
	LMC_AXIS_POS,		/*!< Axis positive direction action. */
	MAX_AXIS_INPUTS		/*!< Total Axis inputs. */
}
LMC_AxisDirection;

/*!
 * \brief
 * Standard axis indices.
 *
 * \see
 * LMC_AssignJoypadAxisIndex(), LMC_DefineJoypadAxisInput()
 *
 */
typedef enum
{
	LMC_AXIS_LEFT_X = 0,	/*!< Left analog X-axis. */
	LMC_AXIS_LEFT_Y,		/*!< Left analog Y-axis. */
	LMC_AXIS_RIGHT_X,		/*!< Right analog X-axis. */
	LMC_AXIS_RIGHT_Y		/*!< Right analog Y-axis. */
}
LMC_AxisIndex;

/*!
 * \brief
 * Standard axis types - analog or digital.
 *
 * \see
 *
 *
 */
typedef enum
{
	LMC_AXIS_ANALOG = 0,	/*!< Treat axis as analog input. */
	LMC_AXIS_DIGITAL,		/*!< Treat axis as digital input. */
}
LMC_AxisType;

/*! 
 * \brief
 * Standard video output types. 
 *
 * \see
 * LMC_SetVideoOutput()
 * 
 */
typedef enum
{
	LMC_VIDEO_OUTPUT_RAW = 0,			/*!< Raw video output stretched horizontally to aspect. */
	LMC_VIDEO_OUTPUT_ADJUSTED_3_2,	/*!< Adjust video output height and stretch horizontally to a 3:2 aspect. */
	LMC_VIDEO_OUTPUT_ADJUSTED_4_3,	/*!< Adjust video output height and stretch horizontally to a 4:3 aspect. */
	LMC_VIDEO_OUTPUT_ADJUSTED_5_4,	/*!< Adjust video output height and stretch horizontally to a 5:4 aspect. */
	LMC_VIDEO_OUTPUT_ADJUSTED_16_9,	/*!< Adjust video output height and stretch horizontally to a 16:9 aspect. */
}
LMC_VideoOutput;

/*! 
 * \brief 
 * CreateWindow flags. Can be none or a combination of the following: 
 * 
 * \see
 * LMC_CreateWindow()
 * 
 */
enum
{
	LMC_CWF_FULLSCREEN = (1 << 0),/*!< Create a fullscreen window. */
	LMC_CWF_VSYNC = (1 << 1),	  /*!< Sync frame updates with vertical retrace. */
	LMC_CWF_NEAREST = (1 << 2),	  /*!< Unfiltered upscaling. */
};

/*! 
 * \brief 
 * Standard error codes. 
 * 
 * \see
 * LMC_SetLastError()
 * 
 */
typedef enum
{
	LMC_ERR_OK = 0,				/*!< No error. */
	LMC_ERR_OUT_OF_MEMORY,		/*!< Not enough memory. */
	LMC_ERR_NULL_POINTER,		/*!< Null pointer as argument. */
	LMC_ERR_INV_PARAM,			/*!< Invalid parameter. */
	LMC_ERR_INV_PATH,			/*!< Invalid path. */
	LMC_ERR_FAIL_PLATORM_INIT,	/*!< Failed to initialize platform. */
	LMC_ERR_FAIL_CONFIG_INIT,	/*!< Failed to initialize config settings. */
	LMC_ERR_FAIL_WINDOW_INIT,	/*!< Failed to create window. */
	LMC_ERR_FAIL_VIDEO_INIT,	/*!< Failed to initialize video. */
	LMC_ERR_FAIL_AUDIO_INIT,	/*!< Failed to initialize audio. */
	LMC_ERR_FAIL_INPUT_INIT,	/*!< Failed to initialize input. */
	LMC_ERR_LIBRETRO,			/*!< Error with Libretro Core. */
	LMC_ERR_TILENGINE,			/*!< Error with Tilengine. */
	LMC_ERR_EXTERNAL,			/*!< Error set from a resulting program that is using the LegacyMachine library. */
	LMC_ERR_UNSUPPORTED,		/*!< Unsupported function. */
	MAX_ERRORS
}
LMC_Error;

/*! 
 * \brief 
 * Standard log levels.
 *
 * \see
 * LMC_SetLogLevel()
 * 
 */
typedef enum
{
	LMC_LOG_NONE = 0,	/*!< Don't print anything (default). */
	LMC_LOG_ERRORS,		/*!< Print only runtime errors. */
	LMC_LOG_VERBOSE		/*!< Print everything. */
}
LMC_LogLevel;

/* Callbacks */

typedef void(*LMC_AutoConfigureJoypadCallback)(LMC_Player player, char* name, uint32_t vendor, uint32_t product);

RETRO_BEGIN_DECLS

/*****************************************************************************
 * Basic Engine Setup & Management
 ****************************************************************************/
#if defined HAVE_MENU

/*!
 * \brief
 * Initializes LegacyMachine.
 *
 * \param program_name
 * Internal name of the program being created. Used as base directory 
 * name in environments where it is required.
 *
 * \param base_width
 * Frontend menu's horizontal resolution in pixels.
 *
 * \param base_height
 * Frontend menu's vertical resolution in pixels.
 *
 * \param max_width
 * Framebuffer's maximum horizontal resolution in pixels.
 *
 * \param max_height
 * Framebuffer's maximum vertical resolution in pixels.
 *
 * \param aspect_ratio
 * Frontend menu's aspect ratio.
 *
 * \param fps
 * Frontend menu's refresh rate in frames per second.
 *
 * \param layers
 * Frontend menu's number of supported layers.
 *
 * \param sprites
 * Frontend menu's number of supported sprites.
 *
 * \param animations
 * Frontend menu's number of supported palette animation slots.
 * 
 * \returns
 * Returns true if initialization was successful, false otherwise.
 *
 * \remarks
 * Performs initialization of the main engine and it's drivers.
 * 
 */
LMCAPI bool LMC_Init(const char* program_name,
	int base_width, int base_height,
	int max_width, int max_height,
	float aspect_ratio, double fps,
	int layers, int sprites, int animations);

#else

/*!
 * \brief
 * Initializes LegacyMachine.
 * 
 * \returns
 * Returns true if initialization was successful, false otherwise.
 *
 * \remarks
 * Performs initialization of the main engine and it's drivers.
 */
LMCAPI bool LMC_Init(void);

#endif

/*!
 * \brief
 * Deinitializes LegacyMachine and frees used resources.
 *
 */
LMCAPI void LMC_Deinit(void);

/*!
 * \brief
 * Gets the current LegacyMachine engine context.
 * 
 * \returns
 * Returns the current LMC_Engine context pointer.
 *
 * \see
 * LMC_Init(), LMC_Deinit()
 * 
 */
LMCAPI LMC_Engine LMC_GetContext(void);

/*!
 * \brief
 * Gets the current LegacyMachine library version.
 *
 * \returns
 * Returns a 32-bit integer containing three packed numbers:
 * bits 23:16 -> major version,
 * bits 15: 8 -> minor version,
 * bits  7: 0 -> bugfix revision
 *
 */
LMCAPI uint32_t LMC_GetVersion(void);

/*!
 * \brief
 * Updates the menu or runs a single loop of a Libretro core and then
 * draws a single frame.
 *
 * \param frame
 * Optional frame number. Set to 0 to autoincrement from previous value.
 *
 */
LMCAPI void LMC_UpdateFrame(int frame);

/*!
 * \brief
 * Gets the number of milliseconds since last epoch.
 *
 * \returns
 * Returns the number of milliseconds since last epoch.
 *
 */
LMCAPI uint64_t LMC_GetTimeElapsed(void);

/*!
 * \brief
 * Suspends execution for a fixed time.
 *
 * \param time
 * Number of milliseconds to wait.
 *
 */
LMCAPI void LMC_SetDelay(uint32_t time);

/*****************************************************************************
 * Callbacks
 ****************************************************************************/

 /*!
  * \brief
  * Sets the callback function for custom joypad input configuration.
  *
  */
LMCAPI void LMC_SetAutoConfigureJoypadCallback(LMC_AutoConfigureJoypadCallback callback);

/*****************************************************************************
 * Path Management
 ****************************************************************************/

 /*!
  * \brief
  * Gets the path to a standard directory.
  *
  * \param path_type
  * LMC_Path enumeration value specifying the standard path to get.
  *
  */
LMCAPI const char* LMC_GetPath(LMC_Path path_type);

/*****************************************************************************
 * Disk Management
 ****************************************************************************/

 /*!
  * \brief
  * Checks the current state of Libretro core's disk control.
  *
  * \returns
  * Returns true if disk control is enabled, false otherwise.
  *
  */
LMCAPI bool LMC_IsDiskControlEnabled(void);

/*!
 * \brief
 * Gets the total number of disks associated with the currently loaded
 * Libretro content.
 *
 * \returns
 * Returns the total number of disks associated with the currently loaded
 * content. Returns 0 when disk control isn't active.
 *
 * \remarks
 * Associated disks are usually represented within an m3u playlist file.
 * In this case, the number of disks are represented by the number of
 * entries in the playlist file.
 *
 */
LMCAPI unsigned LMC_GetDiskCount(void);

/*!
 * \brief
 * Gets the current disk index of the the currently loaded Libretro
 * content
 *
 * \returns
 * Returns the index of the currently active disk. Returns 0 when disk
 * control isn't active.
 *
 */
LMCAPI unsigned LMC_GetDiskIndex(void);

/*!
 * \brief
 * Switches to a given disk index for the currently loaded Libretro
 * content.
 *
 * \param index
 * Index of the disk to switch to.
 *
 * \returns
 * Returns true if disk was switched successfully, false otherwise.
 *
 */
LMCAPI bool LMC_SwitchDisk(unsigned index);


/*****************************************************************************
 * Core and Content Management
 ****************************************************************************/

/*! 
 * \brief
 * Checks if a Libretro core is currently loaded and running.
 *
 * \returns
 * Returns true if a core is loaded and running, false otherwise.
 *
 */
LMCAPI bool LMC_IsCoreRunning(void);

/*! 
 * \brief
 * Loads and initializes a Libretro core from a given filename.
 *
 * \param filename
 * Path to the Libretro core file to load.
 *
 * \param suppress_kb_ui_input
 * If true, disables keyboard UI input and using the keyboard for joypad
 * input while the core is running. Reverts back to prior behavior upon 
 * core close.
 *
 * \returns
 * Returns true if the core was loaded successfully, false otherwise.
 *
 */
LMCAPI bool LMC_LoadCore(const char* filename, bool suppress_kb_ui_input);

/*! 
 * \brief
 * Loads content into the currently loaded Libretro core.
 *
 * \param filename
 * Path to the content file to load.
 *
 * \returns
 * Returns true if the content was loaded successfully, false otherwise.
 *
 */
LMCAPI bool LMC_LoadContent(const char* filename);

/*! 
 * \brief
 * Closes the currently loaded Libretro core and unloads its content.
 *
 */
LMCAPI void LMC_CloseCore(void);

/*!
 * \brief
 * Sets the controller device type for a given port. If the core is
 * currently running, the change takes effect immediately by calling
 * retro_set_controller_port_device. This should be called after
 * LMC_LoadContent to override the default device type selected from
 * the core's SET_CONTROLLER_INFO list.
 *
 * \param port
 * The zero-based controller port index.
 *
 * \param device
 * The libretro device type ID (e.g. RETRO_DEVICE_JOYPAD,
 * RETRO_DEVICE_ANALOG, or a core-specific subclass).
 *
 * \returns
 * Returns true if the port is valid and the device type was set.
 *
 */
LMCAPI bool LMC_SetControllerPortDevice(unsigned port, unsigned device);

/*!
 * \brief
 * Saves the current state of the Libretro core.
 *
 * \param slot
 * The save state slot to save to.
 *
 */
LMCAPI void LMC_SaveState(LMC_Slot slot);

/*!
 * \brief
 * Loads a saved state into the Libretro core.
 *
 * \param slot
 * The save state slot to load from.
 *
 */
LMCAPI void LMC_LoadState(LMC_Slot slot);


/*****************************************************************************
 * Window and Video Management
 ****************************************************************************/

/*!
 * \brief
 * Creates a window for rendering.
 *
 * \param cwf_flags
 * Mask of the possible window creation flags:
 * LMC_CWF_FULLSCREEN, LMC_CWF_VSYNC, LMC_CWF_NEAREST.
 *
 * \returns
 * Returns true if window was created successfully or false if an 
 * error occurred.
 *
 * \remarks
 * Creates a host window for LegacyMachine. If fullscreen, it uses the
 * desktop resolution and stretches the output resolution with aspect
 * correction, letterboxing or pillarboxing as needed. If windowed, it
 * creates a centered window that is the maximum possible integer
 * multiple of either the resolution configured at LMC_Init() or from
 * the core/content loaded from LMC_LoadCore() and LMC_LoadContent().
 *
 * \see
 * LMC_DeleteWindow(), LMC_ProcessWindow()
 * 
 */
LMCAPI bool LMC_CreateWindow(int flags);

/*!
 * \brief
 * Deletes the window and all associated data (Video, Audio, and Input)
 * previoulsy created with LMC_CreateWindow().
 *
 * \see
 * LMC_CreateWindow()
 *
 */
LMCAPI void LMC_DeleteWindow(void);

/*!
 * \brief
 * Processes all events related to a window created with
 * LMC_CreateWindow().
 *
 * \returns
 * Returns true if window is active or false if the user has requested
 * to end the application (by pressing the key assigned to quit).
 *
 * \see
 * LMC_CreateWindow()
 * 
 */
LMCAPI bool LMC_ProcessWindow(void);

/*!
 * \brief
 * Checks whether the window is active and running.
 *
 * \returns
 * Returns true if window is active and running or false if the user
 * has requested to end the application (by pressing the key assigned
 * to quit).
 *
 * \see
 * LMC_CreateWindow()
 * 
 */
LMCAPI bool LMC_IsWindowActive(void);

/*!
 * \brief
 * Sets window title.
 *
 * \param title
 * Text with the title to set.
 *
 */
LMCAPI void LMC_SetWindowTitle(const char* window_title);

/*!
 * \brief
 * Gets the horizontal dimension of window after scaling.
 * 
 * \returns
 * Returns the horizontal dimension of the window in pixels.
 * 
 */
LMCAPI int LMC_GetWindowWidth(void);

/*!
 * \brief
 * Gets the vertical dimension of window after scaling.
 *
 * \returns
 * Returns the vertical dimension of the window in pixels.
 * 
 */
LMCAPI int LMC_GetWindowHeight(void);

/*!
 * \brief
 * Sets the active video output type.
 *
 * \param type 
 * LMC_VideoOutput enumeration value of the output type to set.
 * 
 */
LMCAPI void LMC_SetVideoOutput(LMC_VideoOutput type);

/*!
 * \brief
 * Sets the mask type to use with the CRT effect.
 *
 * \param type 
 * LMC_CRTMask enumeration value of the mask type to set.
 * 
 * \param update_settings 
 * Specifies whether to save the mask type to the settings configuration
 * file - must be called after LMC_CreateWindow().
 * 
 */
LMCAPI void LMC_SetCRTMask(LMC_CRTMask type, bool update_settings);

/*!
 * \brief
 * Gets the mask type currently being used by the CRT effect.
 *
 * \returns 
 * Returns the active LMC_CRTMask value.
 * 
 */
LMCAPI LMC_CRTMask LMC_GetCRTMask(void);

/*****************************************************************************
 * Input Management
 ****************************************************************************/

/*!
 * \brief
 * Gets the state of a given input.
 *
 * \param input
 * Input to check state. It can be one of the following values:
 *	 * LMC_JOYPAD_INPUT_UP
 *	 * LMC_JOYPAD_INPUT_DOWN
 *	 * LMC_JOYPAD_INPUT_LEFT
 *	 * LMC_JOYPAD_INPUT_RIGHT
 *	 * LMC_JOYPAD_INPUT_A
 *   * LMC_JOYPAD_INPUT_B
 *   * LMC_JOYPAD_INPUT_X
 *   * LMC_JOYPAD_INPUT_Y
 *   * LMC_JOYPAD_INPUT_L
 *   * LMC_JOYPAD_INPUT_R
 *   * LMC_JOYPAD_INPUT_L2
 *   * LMC_JOYPAD_INPUT_R2
 *   * LMC_JOYPAD_INPUT_L3
 *   * LMC_JOYPAD_INPUT_R3
 *   * LMC_JOYPAD_INPUT_SELECT
 *	 * LMC_JOYPAD_INPUT_START
 *   * LMC_JOYPAD_INPUT_HOME
 *   * LMC_JOYPAD_INPUT_L_ANALOG_UP
 *   * LMC_JOYPAD_INPUT_L_ANALOG_DOWN
 *   * LMC_JOYPAD_INPUT_L_ANALOG_LEFT
 *   * LMC_JOYPAD_INPUT_L_ANALOG_RIGHT
 *   * LMC_JOYPAD_INPUT_R_ANALOG_UP
 *   * LMC_JOYPAD_INPUT_R_ANALOG_DOWN
 *   * LMC_JOYPAD_INPUT_R_ANALOG_LEFT
 *   * LMC_JOYPAD_INPUT_R_ANALOG_RIGHT
 *	 * Optionally combine with LMC_JOYPAD_INPUT_P1 to LMC_JOYPAD_INPUT_P8
 *     to request input for specific player
 *
 * \returns
 * Returns true if that input is pressed or false if not.
 *
 * \remarks
 * If a window has been created with LMC_CreateWindow(), it provides
 * basic user input. It mirrors libretro's JOYPAD setup where
 * LMC_JOYPAD_INPUT_(button name) corresponds to RETRO_DEVICE_ID_JOYPAD_(button name).
 * By default directional buttons are mapped to the keyboard arrows and
 * the action buttons (B, A, X, Y) are mapped to the keys Z, X, A, S.
 * L and R are mapped to the keys Q and W by default.
 *
 * \see
 * LMC_CreateWindow(), LMC_DefineJoypadKeyInput(), LMC_DefineJoypadButtonInput(),
 * LMC_DefineJoypadHatInput(), LMC_DefineJoypadAxisInput()
 * 
 */
LMCAPI bool LMC_GetJoypadInput(LMC_JoypadInput id);

/*! 
 * \brief
 * Defines a physical keyboard key for a given keyboard input action.
 *
 * \param input
 * LMC_KeyboardInput enumeration value specifying the input action to
 * define.
 *
 * \param keycode
 * LMC_KeyCode enumeration value specifying the key to assign to the
 * input action.
 *
 * \param update_settings 
 * Specifies whether to save the key assignment to the settings 
 * configuration file - must be called after LMC_CreateWindow().
 * 
 */
LMCAPI void LMC_DefineKeyboardKeyInput(LMC_KeyboardInput input,
	LMC_KeyCode keycode, bool update_settings);

/*!
 * \brief
 * Assigns a logical axis index to a physical joypad axis.
 *
 * \param player
 * Player number to configure (PLAYER1 - PLAYER4).
 *
 * \param logical_axis
 * LMC_AxisInput enumeration value specifying the logical axis to assign.
 *
 * \param physical_axis
 * Joypad axis index to assign the logical axis to.
 * 
 */
LMCAPI void LMC_AssignJoypadAxisIndex(LMC_Player player, LMC_AxisIndex logical_axis, uint8_t physical_axis);

/*!
 * \brief
 * Defines a physical keyboard key for a given joypad input action.
 *
 * \param input
 * LMC_JoypadInput enumeration value specifying the input action to
 * define.
 *
 * \param keycode
 * LMC_KeyCode enumeration value specifying the key to assign the
 * input action to.
 *
 * \param update_settings
 * Specifies whether to save the key assignment to the settings
 * configuration file - must be called after LMC_CreateWindow().
 *
 */
LMCAPI void LMC_DefineJoypadKeyInput(LMC_JoypadInput input,
	LMC_KeyCode keycode, bool update_settings);

/*!
 * \brief
 * Defines a physical joypad button for a given joypad input action.
 *
 * \param player
 * Player number to configure (LMC_PLAYER1 - LMC_PLAYER4).
 *
 * \param input
 * LMC_JoypadInput enumeration value specifying the input action to
 * define.
 *
 * \param button
 * Joypad button index to assign the input action to.
 *
 */
LMCAPI void LMC_DefineJoypadButtonInput(LMC_Player player, 
	LMC_JoypadInput input, uint8_t button);

/*!
 * \brief
 * Defines a physical joypad hat direction for a given joypad input action.
 * 
 * \param player
 * Player number to configure (LMC_PLAYER1 - LMC_PLAYER4).
 * 
 * \param hat_index
 * Joypad hat index to assign the input action to.
 * 
 * \param input
 * LMC_JoypadInput enumeration value specifying the input action to
 * define.
 * 
 * \param hat_direction
 * LMC_HatDirection enumeration value specifying the hat direction
 * to assign the input action to.
 * 
 */
LMCAPI void LMC_DefineJoypadHatInput(LMC_Player player, int hat_index,
	LMC_JoypadInput input, LMC_HatDirection hat_direction);

/*!
 * \brief
 * Defines a physical joypad axis direction for a given joypad input action.
 * 
 * \param player
 * Player number to configure (LMC_PLAYER1 - LMC_PLAYER4).
 * 
 * \param axis_index
 * Joypad axis index to assign the input action to.
 * 
 * \param input
 * LMC_JoypadInput enumeration value specifying the input action to
 * define.
 * 
 * \param axis_direction
 * LMC_AxisDirection enumeration value specifying the axis direction
 * to assign the input action to.
 * 
 */
LMCAPI void LMC_DefineJoypadAxisInput(LMC_Player player, int axis_index,
	LMC_JoypadInput input, LMC_AxisDirection axis_direction);

/*****************************************************************************
 * Error Management
 ****************************************************************************/

/*!
 * \brief
 * Sets logging level for current instance.
 *
 * \param log_level
 * Value to set, member of the LMC_LogLevel enumeration.
 *
 */
LMCAPI void LMC_SetLogLevel(LMC_LogLevel log_level);

/*!
 * \brief
 * Sets the global error code of LegacyMachine.
 *
 * \param error
 * Error code to set.
 *
 */
LMCAPI void LMC_SetLastError(LMC_Error error);

/*!
 * \brief
 * Gets the last error after an invalid operation.
 *
 * \returns
 * Returns the last error after an invalid operation.
 *
 */
LMCAPI LMC_Error LMC_GetLastError(void);

/*!
 * \brief
 * Gets the string description of the specified error code.
 *
 * \param error
 * Error code to get description.
 *
 * \returns
 * Returns the string description of the specified error code.
 *
 */
LMCAPI const char* LMC_GetErrorString(LMC_Error error);

/*!
 * \brief
 * Logs a message to the LegacyMachine log output.
 *
 * \param log_level
 * Log level of the message.
 *
 * \param format
 * Format string.
 *
 */
LMCAPI void LMC_Log(LMC_LogLevel log_level, const char* format, ...);

RETRO_END_DECLS

#endif