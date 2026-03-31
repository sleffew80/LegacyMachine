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

#ifndef _SETTINGS_MANAGER_H
#define _SETTINGS_MANAGER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"
#include "Common/Common.h"

/**************************************************************************************************
 * SettingsManager Default Definitions
 *************************************************************************************************/

#define AUDIO_LATENCY_MIN 16
#define AUDIO_LATENCY_MAX MAX_LATENCY
#define AUDIO_LATENCY_DEFAULT 64

#define AUDIO_FREQUENCY_MIN 8000
#define AUDIO_FREQUENCY_MAX 192000
#define AUDIO_FREQUENCY_DEFAULT 48000

#define VIDEO_ASPECT_MIN 0.25f
#define VIDEO_ASPECT_MAX 3.0f

#define VIDEO_OUTPUT_DEFAULT "adjusted_4-3"
#define VIDEO_CRT_EFFECT_MASK_DEFAULT "slot"

#define INPUT_KEYBOARD_ENABLE_JOYPAD_INPUT_DEFAULT true
#define INPUT_KEYBOARD_B_KEY_DEFAULT "z"
#define INPUT_KEYBOARD_Y_KEY_DEFAULT "a"
#define INPUT_KEYBOARD_SELECT_KEY_DEFAULT "right_shift"
#define INPUT_KEYBOARD_START_KEY_DEFAULT "enter"
#define INPUT_KEYBOARD_UP_KEY_DEFAULT "up"
#define INPUT_KEYBOARD_DOWN_KEY_DEFAULT "down"
#define INPUT_KEYBOARD_LEFT_KEY_DEFAULT "left"
#define INPUT_KEYBOARD_RIGHT_KEY_DEFAULT "right"
#define INPUT_KEYBOARD_A_KEY_DEFAULT "x"
#define INPUT_KEYBOARD_X_KEY_DEFAULT "s"
#define INPUT_KEYBOARD_L_KEY_DEFAULT "q"
#define INPUT_KEYBOARD_R_KEY_DEFAULT "w"
#define INPUT_KEYBOARD_L2_KEY_DEFAULT "null"
#define INPUT_KEYBOARD_R2_KEY_DEFAULT "null"
#define INPUT_KEYBOARD_L3_KEY_DEFAULT "null"
#define INPUT_KEYBOARD_R3_KEY_DEFAULT "null"
#define INPUT_KEYBOARD_L_ANALOG_UP_KEY_DEFAULT "null"
#define INPUT_KEYBOARD_L_ANALOG_DOWN_KEY_DEFAULT "null"
#define INPUT_KEYBOARD_L_ANALOG_LEFT_KEY_DEFAULT "null"
#define INPUT_KEYBOARD_L_ANALOG_RIGHT_KEY_DEFAULT "null"
#define INPUT_KEYBOARD_R_ANALOG_UP_KEY_DEFAULT "null"
#define INPUT_KEYBOARD_R_ANALOG_DOWN_KEY_DEFAULT "null"
#define INPUT_KEYBOARD_R_ANALOG_LEFT_KEY_DEFAULT "null"
#define INPUT_KEYBOARD_R_ANALOG_RIGHT_KEY_DEFAULT "null"
#define INPUT_KEYBOARD_QUIT_KEY_DEFAULT "escape"
#define INPUT_KEYBOARD_HOME_KEY_DEFAULT "home"
#define INPUT_KEYBOARD_SAVE_STATE_KEY_DEFAULT "f1"
#define INPUT_KEYBOARD_LOAD_STATE_KEY_DEFAULT "f2"
#define INPUT_KEYBOARD_TOGGLE_CRT_KEY_DEFAULT "tab"
#define INPUT_KEYBOARD_TOGGLE_FULLSCREEN_KEY_DEFAULT "f11"

#if defined(HAVE_COMPRESSION) && defined(HAVE_ZLIB)
#define COMPRESS_STATE_FILES_DEFAULT false
#define COMPRESS_SAVE_FILES_DEFAULT false
#endif

/**************************************************************************************************
 * Macro Definitions
 *************************************************************************************************/

#define SetBoolSetting(settings, value, new_value) \
{ \
	settings->modified = true; \
	value = new_value; \
}

#define SetFloatSetting(settings, value, new_value) \
{ \
	settings->modified = true; \
	value = new_value; \
}

#define SetStringSetting(settings, value, new_value) \
{ \
	settings->modified = true; \
	strlcpy(value, new_value, sizeof(value)); \
}

/**************************************************************************************************
 * DefaultInfo Structure
 *************************************************************************************************/

/* Structure for storing default initialization info which can later be overridden
   by configuration settings. */
typedef struct DefaultInfo
{
	float	aspect_ratio;	/* Default override aspect ratio. */
	bool	fullscreen;		/* Default fullscreen state. */
	bool	vsync;			/* Default vertical sync state. */
	bool	smooth;			/* Default frame smoothing state. */
}
DefaultInfo;

/**************************************************************************************************
 * SettingsManager Structure
 *************************************************************************************************/

typedef struct SettingsManager
{
	char program_name[NAME_MAX_LENGTH];		/* Name of program using the LegacyMachine library. */

	struct
	{
		char path_main_directory[PATH_MAX_LENGTH];
		char path_setting_directory[PATH_MAX_LENGTH];
		char path_option_directory[PATH_MAX_LENGTH];
		char path_option_content_directory[PATH_MAX_LENGTH];
		char path_asset_directory[PATH_MAX_LENGTH];
		char path_core_directory[PATH_MAX_LENGTH];
		char path_content_directory[PATH_MAX_LENGTH];
		char path_system_directory[PATH_MAX_LENGTH];
		char path_save_directory[PATH_MAX_LENGTH];
		char path_state_directory[PATH_MAX_LENGTH];
		char path_settings_config_file[PATH_MAX_LENGTH];
		char path_temp_directory[PATH_MAX_LENGTH];
	}
	paths;				/* Directory and file paths used by the engine. */

	struct
	{
		char video_output_type[VALUE_MAX_LENGTH];
		char video_crt_effect_mask_type[VALUE_MAX_LENGTH];
		char input_keyboard_player1_b_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_y_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_select_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_start_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_up_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_down_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_left_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_right_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_a_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_x_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_l_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_r_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_l2_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_r2_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_l3_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_r3_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_l_analog_up_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_l_analog_down_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_l_analog_left_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_l_analog_right_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_r_analog_up_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_r_analog_down_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_r_analog_left_key[VALUE_MAX_LENGTH];
		char input_keyboard_player1_r_analog_right_key[VALUE_MAX_LENGTH];
		char input_keyboard_quit_key[VALUE_MAX_LENGTH];
		char input_keyboard_home_key[VALUE_MAX_LENGTH];
		char input_keyboard_state_save_key[VALUE_MAX_LENGTH];
		char input_keyboard_state_load_key[VALUE_MAX_LENGTH];
		char input_keyboard_toggle_fullscreen_key[VALUE_MAX_LENGTH];
		char input_keyboard_toggle_crt_effect_key[VALUE_MAX_LENGTH];
		char reference;
	}
	strings;			/* String-based settings values. */

	struct
	{
		float video_aspect_ratio;
		float reference;
	}
	floats;				/* Float-based settings values. */

	struct
	{
		unsigned audio_latency;
		unsigned audio_frequency;
		unsigned reference;
	}
	uints;				/* Unsigned integer settings values. */

	struct
	{
#if defined(HAVE_COMPRESSION) && defined(HAVE_ZLIB)
		bool compress_state_files;
		bool compress_save_files;
#endif
		bool input_keyboard_enable_joypad_input;
		bool video_fullscreen;
		bool video_vsync;
		bool video_smooth;
		bool reference;
	}
	bools;				/* Boolean settings values. */

	bool initialized;	/* Whether the settings manager is initialized. */
	bool modified;		/* Whether any settings have been modified. */
}
SettingsManager;

/**************************************************************************************************
 * SettingsManager Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

DefaultInfo* GetDefaultInfoContext(void);
SettingsManager* GetSettingsManagerContext(void);

LMC_VideoOutput GetOutputEnumFromSetting(void);
LMC_CRTMask GetCRTMaskEnumFromSetting(void);
LMC_KeyCode GetKeyCodeEnumFromString(const char* key_string);

bool InitializeSettings(void);
void SetSettingsDefaults(void);
bool SaveSettingsConfigFile(const char* path);

RETRO_END_DECLS

#endif
