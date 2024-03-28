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
* Credits: 
*
* LegacyMachine uses the LibRetro API and LibRetro SDK.
* 
* Copyright (C) 2010-2024 The RetroArch team
* 
* LegacyMachine is deliberately modeled after Tilengine and several
* functions and enumerations mirror that of Tilengine's exactly. For
* simplicity, LegacyMachine is released under the same terms of the Mozilla
* Public License that Tilengine is released under.
*
* Tilengine - The 2D retro graphics engine with raster effects.
* Copyright (C) 2015-2024 Marc Palacios Domenech <mailto:megamarc@hotmail.com>
* All rights reserved
* */

#ifndef _LEGACY_MACHINE_H
#define _LEGACY_MACHINE_H

/* Legacy Machine Library API */
#if defined _MSC_VER
#if defined RETRO_LIB_EXPORTS
#define LMCAPI __declspec(dllexport)
#else
#define LMCAPI __declspec(dllimport)
#endif

#else
#if defined RETRO_LIB_EXPORTS
#define LMCAPI __attribute__((visibility("default")))
#else
#define LMCAPI
#endif
#endif

#include <stdio.h>

#include <retro_library.h>

#if defined HAVE_MENU
#include <Tilengine.h>
#endif

/* Version */
#define LEGACY_MACHINE_VER_MAJ	0
#define LEGACY_MACHINE_VER_MIN	0
#define LEGACY_MACHINE_VER_REV	9
#define LEGACY_MACHINE_HEADER_VERSION ((LEGACY_MACHINE_VER_MAJ << 16) | (LEGACY_MACHINE_VER_MIN << 8) | LEGACY_MACHINE_VER_REV)

/*! Types of built-in CRT effect for \ref LMC_ConfigCRTEffect. */
typedef enum
{
	LMC_CRT_SLOT,		/*!< Slot mask without scanlines, similar to legacy effect. */
	LMC_CRT_APERTURE,	/*!< Aperture grille with scanlines (matrix-like dot arrangement). */
	LMC_CRT_SHADOW,		/*!< Shadow mask with scanlines, diagonal subpixel arrangement. */
}
LMC_CRT;

typedef struct MainEngine* LMC_Engine;		/*!< Engine context. */

/* Callbacks */

typedef void(*LMC_AutoConfigureJoypadCallback)(void);

/*! Standard paths. */
typedef enum
{
	LMC_MAIN_PATH,		/*!< Application executable path. */
	LMC_SETTING_PATH,	/*!< Settings path. */
	LMC_ASSET_PATH,		/*!< Assets path. */
	LMC_CORE_PATH,		/*!< LibRetro core path. */
	LMC_CONTENT_PATH,	/*!< LibRetro content path. */
	LMC_SYSTEM_PATH,	/*!< LibRetro system files path. */
	LMC_SAVE_PATH,		/*!< SRAM and memory card data save path.*/
	LMC_STATE_PATH		/*!< State save path. */
}
LMC_Path;

/*! Player index for input assignment functions. */
typedef enum
{
	LMC_PLAYER1,	/*!< Player 1 */
	LMC_PLAYER2,	/*!< Player 2 */
	LMC_PLAYER3,	/*!< Player 3 */
	LMC_PLAYER4,	/*!< Player 4 */
	LMC_PLAYER5,	/*!< Player 5 */
	LMC_PLAYER6,	/*!< Player 6 */
	LMC_PLAYER7,	/*!< Player 7 */
	LMC_PLAYER8		/*!< Player 8 */
}
LMC_Player;

/*! Standard inputs query for libretro cores and LMC_GetInput(). */
typedef enum
{
	LMC_INPUT_NONE,		/*!< No input. */
	LMC_INPUT_B,		/*!< B action button. */
	LMC_INPUT_Y,		/*!< Y action button. */
	LMC_INPUT_SELECT,	/*!< Select button. */
	LMC_INPUT_START,	/*!< Start button. */
	LMC_INPUT_UP,		/*!< Up direction. */
	LMC_INPUT_DOWN,		/*!< Down direction. */
	LMC_INPUT_LEFT,		/*!< Left direction. */
	LMC_INPUT_RIGHT,	/*!< Right direction. */
	LMC_INPUT_A,		/*!< A action button. */
	LMC_INPUT_X,		/*!< X action button. */
	LMC_INPUT_L,		/*!< L action button. */
	LMC_INPUT_R,		/*!< R action button. */
	LMC_INPUT_L2,		/*!< L2 action button. */
	LMC_INPUT_R2,		/*!< R2 action button. */
	LMC_INPUT_L3,		/*!< L3 action button. */
	LMC_INPUT_R3,		/*!< R3 action button. */
	LMC_INPUT_QUIT,		/*!< Window close (Player 1 keyboard only). */
	LMC_INPUT_CRT,		/*!< CRT toggle (Player 1 keyboard only). */

	LMC_INPUT_P1 = (LMC_PLAYER1 << 5), 	/*!< Request player 1 input (default). */
	LMC_INPUT_P2 = (LMC_PLAYER2 << 5),	/*!< Request player 2 input. */
	LMC_INPUT_P3 = (LMC_PLAYER3 << 5),	/*!< Request player 3 input. */
	LMC_INPUT_P4 = (LMC_PLAYER4 << 5),	/*!< Request player 4 input. */
	LMC_INPUT_P5 = (LMC_PLAYER5 << 5),	/*!< Request player 5 input. */
	LMC_INPUT_P6 = (LMC_PLAYER6 << 5),	/*!< Request player 6 input. */
	LMC_INPUT_P7 = (LMC_PLAYER7 << 5),	/*!< Request player 7 input. */
	LMC_INPUT_P8 = (LMC_PLAYER8 << 5),	/*!< Request player 8 input. */
}
LMC_Input;

/*! Standard hat inputs query for libretro cores and LMC_GetInput(). */
typedef enum
{
	LMC_HAT_UP,		/*!< Hat up action. */
	LMC_HAT_RIGHT,	/*!< Hat right action. */
	LMC_HAT_DOWN,	/*!< Hat down action. */
	LMC_HAT_LEFT,	/*!< Hat left action. */
	MAX_HAT_INPUTS
}
LMC_HatDirection;

/*! Standard axis inputs query for libretro cores and LMC_GetInput(). */
typedef enum
{
	LMC_AXIS_NEG,		/*!< Axis negative direction action. */
	LMC_AXIS_POS,		/*!< Axis positive direction action. */
	MAX_AXIS_INPUTS
}
LMC_AxisDirection;

/*! CreateWindow flags. Can be none or a combination of the following: */
enum
{
	LMC_CWF_FULLSCREEN = (1 << 0),/*!< Create a fullscreen window. */
	LMC_CWF_VSYNC = (1 << 1),	  /*!< Sync frame updates with vertical retrace. */
	LMC_CWF_S1 = (1 << 2),		  /*!< Create a window the same size as the framebuffer. */
	LMC_CWF_S2 = (2 << 2),		  /*!< Create a window 2x the size the framebuffer. */
	LMC_CWF_S3 = (3 << 2),		  /*!< Create a window 3x the size the framebuffer. */
	LMC_CWF_S4 = (4 << 2),		  /*!< Create a window 4x the size the framebuffer. */
	LMC_CWF_S5 = (5 << 2),		  /*!< Create a window 5x the size the framebuffer. */
	LMC_CWF_NEAREST = (1 << 6),	  /*!< Unfiltered upscaling. */
};

/*! Error codes */
typedef enum
{
	LMC_ERR_OK,					/*!< No error. */
	LMC_ERR_OUT_OF_MEMORY,		/*!< Not enough memory. */
	LMC_ERR_NULL_POINTER,		/*!< Null pointer as argument. */
	LMC_ERR_INV_PARAM,			/*!< Invalid parameter. */
	LMC_ERR_INV_PATH,			/*!< Invalid path. */
	LMC_ERR_FAIL_WINDOW_INIT,	/*!< Failed to create window. */
	LMC_ERR_FAIL_VIDEO_INIT,	/*!< Failed to initialize video. */
	LMC_ERR_FAIL_AUDIO_INIT,	/*!< Failed to initialize audio. */
	LMC_ERR_FAIL_INPUT_INIT,	/*!< Failed to initialize input. */
	LMC_ERR_LIBRETRO,			/*!< Error with Libretro Core. */
	LMC_ERR_TILENGINE,			/*!< Error with Tilengine. */
	LMC_ERR_UNSUPPORTED,		/*!< Unsupported function. */
	MAX_ERRORS
}
LMC_Error;

/*! Debug level */
typedef enum
{
	LMC_LOG_NONE,		/*!< Don't print anything (default). */
	LMC_LOG_ERRORS,		/*!< Print only runtime errors. */
	LMC_LOG_VERBOSE		/*!< Print everything. */
}
LMC_LogLevel;

RETRO_BEGIN_DECLS

/*****************************************************************************
 * Basic Engine Setup & Management
 ****************************************************************************/
#if defined HAVE_MENU
LMCAPI LMC_Engine LMC_Init(const char* program_name,
	int base_width, int base_height,
	int max_width, int max_height,
	float aspect_ratio, double fps,
	int layers, int sprites, int animations);
#else
LMCAPI LMC_Engine LMC_Init(void);
#endif
LMCAPI void LMC_Deinit(void);
LMCAPI bool LMC_DeleteContext(LMC_Engine context);
LMCAPI bool LMC_SetContext(LMC_Engine context);
LMCAPI LMC_Engine LMC_GetContext(void);
LMCAPI uint32_t LMC_GetVersion(void);
LMCAPI bool LMC_IsCoreRunning(void);
LMCAPI bool LMC_LoadCore(const char* filename);
LMCAPI bool LMC_LoadContent(const char* filename);
LMCAPI void LMC_CloseCore(void);
LMCAPI void LMC_UpdateFrame(int frame);

/*****************************************************************************
 * Callbacks
 ****************************************************************************/
LMCAPI void LMC_SetAutoConfigureJoypadCallback(LMC_AutoConfigureJoypadCallback);

/*****************************************************************************
 * Window and Video Management
 ****************************************************************************/
LMCAPI bool LMC_CreateWindow(int flags);
LMCAPI void LMC_DeleteWindow(void);
LMCAPI bool LMC_ProcessWindow(void);
LMCAPI bool LMC_IsWindowActive(void);
LMCAPI void LMC_SetWindowTitle(const char* window_title);
LMCAPI void LMC_SetBaseDimensionOverrides(int width, int height);
LMCAPI int LMC_GetWindowWidth(void);
LMCAPI int LMC_GetWindowHeight(void);
LMCAPI uint64_t LMC_GetTicks(void);
LMCAPI void LMC_Delay(uint32_t time);
LMCAPI void LMC_EnableRFBlur(bool mode);
LMCAPI void LMC_ConfigCRTEffect(LMC_CRT type, bool blur);
LMCAPI void LMC_DisableCRTEffect(void);

/*****************************************************************************
 * Path Management
 ****************************************************************************/
LMCAPI const char* LMC_GetPath(LMC_Path path_type);

/*****************************************************************************
 * Input Management
 ****************************************************************************/
LMCAPI bool LMC_GetInput(LMC_Input id);
LMCAPI void LMC_EnableKeyboardAsJoypadInput(LMC_Player player, bool enable);
LMCAPI void LMC_AssignInputJoypad(LMC_Player player, int index);
LMCAPI void LMC_DefineJoypadInputKey(LMC_Player player, LMC_Input input,
	uint32_t keycode);
LMCAPI void LMC_DefineJoypadInputButton(LMC_Player player, LMC_Input input,
	uint8_t joybutton);
LMCAPI void LMC_DefineJoypadInputHat(LMC_Player player, int hat_index,
	LMC_Input input, LMC_HatDirection hat_direction);
LMCAPI void LMC_DefineJoypadInputAxis(LMC_Player player, int axis_index,
	LMC_Input input, LMC_AxisDirection axis_direction);

/*****************************************************************************
 * Error Management
 ****************************************************************************/
LMCAPI void LMC_SetLogLevel(LMC_LogLevel log_level);
LMCAPI void LMC_SetLastError(LMC_Error error);
LMCAPI LMC_Error LMC_GetLastError(void);
LMCAPI const char* LMC_GetErrorString(LMC_Error error);

RETRO_END_DECLS

#endif