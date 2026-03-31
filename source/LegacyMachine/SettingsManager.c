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

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <file/file_path.h>
#include <file/config_file.h>
#include <string/stdstring.h>

#include "SettingsManager.h"
#include "Logging.h"

/**************************************************************************************************
 * SettingsManager Default Definitions
 *************************************************************************************************/

#define SETTINGS_FILE_NAME "Settings.cfg"

/**************************************************************************************************
 * DefaultInfo Context
 *************************************************************************************************/

static DefaultInfo default_initialization_info = { 0 };

/**************************************************************************************************
 * SettingsManager Context
 *************************************************************************************************/

static SettingsManager settings_manager = { 0 };

/**************************************************************************************************
 * SettingsManager Types/Structures
 *************************************************************************************************/

struct BoolSetting
{
	const char* id;
	bool* ptr;
	bool default_value;
	bool use_default;
};

struct UnsignedSetting
{
	const char* id;
	unsigned* ptr;
	unsigned minimum_value;
	unsigned maximum_value;
	unsigned default_value;
	bool use_default;
};

struct FloatSetting
{
	const char* id;
	float* ptr;
	float minimum_value;
	float maximum_value;
	float default_value;
	bool use_default;
};

struct StringSetting
{
	const char* id;
	char* ptr;
	char* default_value;
	bool use_default;
};

/**************************************************************************************************
 * SettingsManager Macro Definitions
 *************************************************************************************************/

/* Initializes a numeric setting, keeping it within the range specified. */
#define InitializeNumericSetting(key, setting_value, enable_default, default_setting, minimum_setting, maximum_setting) \
{ \
	settings_list[count].id				 = key; \
	settings_list[count].ptr			 = setting_value; \
	settings_list[count].minimum_value	 = minimum_setting; \
	settings_list[count].maximum_value	 = maximum_setting; \
	settings_list[count].use_default	 = enable_default; \
	if (enable_default) \
		settings_list[count].default_value = default_setting; \
	count++; \
}

/* Initializes a general setting. */
#define InitializeGeneralSetting(key, setting_value, enable_default, default_setting) \
{ \
	settings_list[count].id				 = key; \
	settings_list[count].ptr			 = setting_value; \
	settings_list[count].use_default	 = enable_default; \
	if (enable_default) \
		settings_list[count].default_value = default_setting; \
	count++; \
}

/* Initializes a bool setting. */
#define InitializeBoolSetting(key, setting_value, enable_default, default_setting) \
	InitializeGeneralSetting(key, setting_value, enable_default, default_setting)

/* Initializes an unsigned setting. */
#define InitializeUnsignedSetting(key, setting_value, enable_default, default_setting, minimum_setting, maximum_setting) \
	InitializeNumericSetting(key, setting_value, enable_default, default_setting, minimum_setting, maximum_setting)

/* Initializes a float setting. */
#define InitializeFloatSetting(key, setting_value, enable_default, default_setting, minimum_setting, maximum_setting) \
	InitializeNumericSetting(key, setting_value, enable_default, default_setting, minimum_setting, maximum_setting)

/* Initializes a string setting. */
#define InitializeStringSetting(key, setting_value, enable_default, default_setting) \
	InitializeGeneralSetting(key, setting_value, enable_default, default_setting)

/* Verifies that a numeric setting is valid and within a defined range. Sets it to the defined
   minimum/maximum value if not. */
#define VerifyNumericSetting(value, setting) \
{ \
	if (value < setting.minimum_value) \
	{ \
		value = setting.minimum_value; \
		settings->modified = true; \
	} \
	if (value > setting.maximum_value) \
	{ \
		value = setting.maximum_value; \
		settings->modified = true; \
	} \
}

/* Verifies that a string setting is valid. Sets it to the default value if not. */
#define VerifyStringSetting(value, setting) \
{ \
	VerifyStringSettings(&value, &setting); \
}

/**************************************************************************************************
 * SettingsManager Static Functions
 *************************************************************************************************/

/* Initialize bool type settings. Set defaults, if necessary. */
static struct BoolSetting* InitializeBoolSettings(int* size)
{
	SettingsManager* settings = GetSettingsManagerContext();
	DefaultInfo* defaults = GetDefaultInfoContext();
	struct BoolSetting* settings_list =
		(struct BoolSetting*)calloc(1, (*size + 1) * sizeof(struct BoolSetting));
	unsigned count = 0;

	if (!settings_list)
		return NULL;

#if defined(HAVE_COMPRESSION) && defined(HAVE_ZLIB)
	InitializeBoolSetting("compress_state_files",
		&settings->bools.compress_state_files,
		true,
		COMPRESS_STATE_FILES_DEFAULT);

	InitializeBoolSetting("compress_save_files",
		&settings->bools.compress_save_files,
		true,
		COMPRESS_SAVE_FILES_DEFAULT);
#endif
	InitializeBoolSetting("input_keyboard_enable_joypad_input",
		&settings->bools.input_keyboard_enable_joypad_input,
		true,
		INPUT_KEYBOARD_ENABLE_JOYPAD_INPUT_DEFAULT);

	InitializeBoolSetting("video_fullscreen",
		&settings->bools.video_fullscreen,
		true,
		defaults->fullscreen);

	InitializeBoolSetting("video_vsync",
		&settings->bools.video_vsync,
		true,
		defaults->vsync);

	InitializeBoolSetting("video_smooth",
		&settings->bools.video_smooth,
		true,
		defaults->smooth);

	*size = count;

	return settings_list;
}

/* Initializes an unsigned setting. Set defaults and ensure values fall within the provided
   minimum/maximum range, if necessary. */
static struct UnsignedSetting* InitializeUnsignedSettings(int* size)
{
	SettingsManager* settings = GetSettingsManagerContext();
	struct UnsignedSetting* settings_list =
		(struct UnsignedSetting*)calloc(1, (*size + 1) * sizeof(struct UnsignedSetting));
	unsigned count = 0;

	if (!settings_list)
		return NULL;

	InitializeUnsignedSetting("audio_latency",
		&settings->uints.audio_latency,
		true,
		AUDIO_LATENCY_DEFAULT,
		AUDIO_LATENCY_MIN,
		AUDIO_LATENCY_MAX);

	*size = count;

	return settings_list;
}

/* Initialize float type settings. Set defaults and ensure values fall within the provided
   minimum/maximum range, if necessary. */
static struct FloatSetting* InitializeFloatSettings(int* size)
{
	SettingsManager* settings = GetSettingsManagerContext();
	DefaultInfo* defaults = GetDefaultInfoContext();
	struct FloatSetting* settings_list =
		(struct FloatSetting*)calloc(1, (*size + 1) * sizeof(struct FloatSetting));
	unsigned count = 0;

	if (!settings_list)
		return NULL;

	InitializeFloatSetting("video_aspect_ratio",
		&settings->floats.video_aspect_ratio,
		true,
		defaults->aspect_ratio,
		VIDEO_ASPECT_MIN,
		VIDEO_ASPECT_MAX);

	*size = count;

	return settings_list;
}

/* Initialize string type settings. Set defaults, if necessary. */
static struct StringSetting* InitializeStringSettings(int* size)
{
	SettingsManager* settings = GetSettingsManagerContext();
	struct StringSetting* settings_list =
		(struct StringSetting*)calloc(1, (*size + 1) * sizeof(struct StringSetting));
	unsigned count = 0;

	if (!settings_list)
		return NULL;

	InitializeStringSetting("video_output_type",
		&settings->strings.video_output_type,
		true,
		VIDEO_OUTPUT_DEFAULT);

	InitializeStringSetting("video_crt_effect_mask_type",
		&settings->strings.video_crt_effect_mask_type,
		true,
		VIDEO_CRT_EFFECT_MASK_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_b_key",
		&settings->strings.input_keyboard_player1_b_key,
		true,
		INPUT_KEYBOARD_B_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_y_key",
		&settings->strings.input_keyboard_player1_y_key,
		true,
		INPUT_KEYBOARD_Y_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_select_key",
		&settings->strings.input_keyboard_player1_select_key,
		true,
		INPUT_KEYBOARD_SELECT_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_start_key",
		&settings->strings.input_keyboard_player1_start_key,
		true,
		INPUT_KEYBOARD_START_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_up_key",
		&settings->strings.input_keyboard_player1_up_key,
		true,
		INPUT_KEYBOARD_UP_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_down_key",
		&settings->strings.input_keyboard_player1_down_key,
		true,
		INPUT_KEYBOARD_DOWN_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_left_key",
		&settings->strings.input_keyboard_player1_left_key,
		true,
		INPUT_KEYBOARD_LEFT_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_right_key",
		&settings->strings.input_keyboard_player1_right_key,
		true,
		INPUT_KEYBOARD_RIGHT_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_a_key",
		&settings->strings.input_keyboard_player1_a_key,
		true,
		INPUT_KEYBOARD_A_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_x_key",
		&settings->strings.input_keyboard_player1_x_key,
		true,
		INPUT_KEYBOARD_X_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_l_key",
		&settings->strings.input_keyboard_player1_l_key,
		true,
		INPUT_KEYBOARD_L_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_r_key",
		&settings->strings.input_keyboard_player1_r_key,
		true,
		INPUT_KEYBOARD_R_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_l2_key",
		&settings->strings.input_keyboard_player1_l2_key,
		true,
		INPUT_KEYBOARD_L2_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_r2_key",
		&settings->strings.input_keyboard_player1_r2_key,
		true,
		INPUT_KEYBOARD_R2_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_l3_key",
		&settings->strings.input_keyboard_player1_l3_key,
		true,
		INPUT_KEYBOARD_L3_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_r3_key",
		&settings->strings.input_keyboard_player1_r3_key,
		true,
		INPUT_KEYBOARD_R3_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_l_analog_up_key",
		&settings->strings.input_keyboard_player1_l_analog_up_key,
		true,
		INPUT_KEYBOARD_L_ANALOG_UP_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_l_analog_down_key",
		&settings->strings.input_keyboard_player1_l_analog_down_key,
		true,
		INPUT_KEYBOARD_L_ANALOG_DOWN_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_l_analog_left_key",
		&settings->strings.input_keyboard_player1_l_analog_left_key,
		true,
		INPUT_KEYBOARD_L_ANALOG_LEFT_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_l_analog_right_key",
		&settings->strings.input_keyboard_player1_l_analog_right_key,
		true,
		INPUT_KEYBOARD_L_ANALOG_RIGHT_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_r_analog_up_key",
		&settings->strings.input_keyboard_player1_r_analog_up_key,
		true,
		INPUT_KEYBOARD_R_ANALOG_UP_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_r_analog_down_key",
		&settings->strings.input_keyboard_player1_r_analog_down_key,
		true,
		INPUT_KEYBOARD_R_ANALOG_DOWN_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_r_analog_left_key",
		&settings->strings.input_keyboard_player1_r_analog_left_key,
		true,
		INPUT_KEYBOARD_R_ANALOG_LEFT_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_player1_r_analog_right_key",
		&settings->strings.input_keyboard_player1_r_analog_right_key,
		true,
		INPUT_KEYBOARD_R_ANALOG_RIGHT_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_quit_key",
		&settings->strings.input_keyboard_quit_key,
		true,
		INPUT_KEYBOARD_QUIT_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_home_key",
		&settings->strings.input_keyboard_home_key,
		true,
		INPUT_KEYBOARD_HOME_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_state_save_key",
		&settings->strings.input_keyboard_state_save_key,
		true,
		INPUT_KEYBOARD_SAVE_STATE_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_state_load_key",
		&settings->strings.input_keyboard_state_load_key,
		true,
		INPUT_KEYBOARD_LOAD_STATE_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_toggle_fullscreen_key",
		&settings->strings.input_keyboard_toggle_fullscreen_key,
		true,
		INPUT_KEYBOARD_TOGGLE_FULLSCREEN_KEY_DEFAULT);

	InitializeStringSetting("input_keyboard_toggle_crt_effect_key",
		&settings->strings.input_keyboard_toggle_crt_effect_key,
		true,
		INPUT_KEYBOARD_TOGGLE_CRT_KEY_DEFAULT);

	*size = count;

	return settings_list;
}

/* Verify output string setting is a valid value and set to default if it is not. */
static void VerifyOutputSetting(const char* value)
{
	SettingsManager* settings = GetSettingsManagerContext();

	if (!string_is_equal(string_to_lower(value), "raw") &&
		!string_is_equal(string_to_lower(value), "adjusted_3-2") &&
		!string_is_equal(string_to_lower(value), "adjusted_4-3") &&
		!string_is_equal(string_to_lower(value), "adjusted_5-4") &&
		!string_is_equal(string_to_lower(value), "adjusted_16-9"))
	{
		SetStringSetting(settings, value, VIDEO_OUTPUT_DEFAULT);
	}
}

/* Verify crt mask type string setting is a valid value and set to default if it is not. */
static void VerifyCRTMaskSetting(const char* value)
{
	SettingsManager* settings = GetSettingsManagerContext();

	if (!string_is_equal(string_to_lower(value), "slot") &&
		!string_is_equal(string_to_lower(value), "shadow") &&
		!string_is_equal(string_to_lower(value), "aperture"))
	{
		SetStringSetting(settings, value, VIDEO_CRT_EFFECT_MASK_DEFAULT);
	}
}

/* Verify string has corresponding keyboard key. */
static bool IsValidKeyboardKeyString(const char* value)
{
	/* empty / NULL -> invalid */
	if (!value || value[0] == '\0')
		return false;

	/* "null" -> valid (no key assigned). */
	if (string_is_equal(string_to_lower(value), "null"))
		return true;

	/* Normalize/lookup via the existing mapping function.
	   GetKeyCodeFromSetting returns LMC_KEY_UNKNOWN for unknown values. */
	return (GetKeyCodeEnumFromString(string_to_lower(value)) != LMC_KEY_UNKNOWN);
}

/* Verify keyboard key string setting is a valid value and set to default if it is not. */
static void VerifyKeyboardKeySetting(const char* value, const char* default_value)
{
	SettingsManager* settings = GetSettingsManagerContext();

	if (!IsValidKeyboardKeyString(value))
	{
		SetStringSetting(settings, value, default_value);
	}
}

/* Verifies that a string setting is valid. */
static void VerifyStringSettings(const char* value, struct StringSetting* setting)
{
	if (string_is_equal(setting->id, "video_output_type"))
	{
		VerifyOutputSetting(value);
	}
	if (string_is_equal(setting->id, "video_crt_effect_mask_type"))
	{
		VerifyCRTMaskSetting(value);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_b_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_B_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_y_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_Y_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_select_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_SELECT_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_start_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_START_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_up_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_UP_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_down_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_DOWN_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_left_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_LEFT_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_right_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_RIGHT_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_a_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_A_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_x_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_X_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_l_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_L_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_r_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_R_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_l2_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_L2_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_r2_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_R2_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_l3_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_L3_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_r3_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_R3_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_l_analog_up_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_L_ANALOG_UP_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_l_analog_down_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_L_ANALOG_DOWN_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_l_analog_left_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_L_ANALOG_LEFT_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_l_analog_right_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_L_ANALOG_RIGHT_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_r_analog_up_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_R_ANALOG_UP_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_r_analog_down_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_R_ANALOG_DOWN_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_r_analog_left_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_R_ANALOG_LEFT_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_player1_r_analog_right_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_R_ANALOG_RIGHT_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_quit_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_QUIT_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_home_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_HOME_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_state_save_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_SAVE_STATE_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_state_load_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_LOAD_STATE_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_toggle_fullscreen_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_TOGGLE_FULLSCREEN_KEY_DEFAULT);
	}
	if (string_is_equal(setting->id, "input_keyboard_toggle_crt_effect_key"))
	{
		VerifyKeyboardKeySetting(value, INPUT_KEYBOARD_TOGGLE_CRT_KEY_DEFAULT);
	}
}

/* Load the settings configuration file if one is present or create a new file if it is not. */
static bool LoadSettingsConfigFile(const char* path)
{
	SettingsManager* settings = GetSettingsManagerContext();
	config_file_t* settings_config = config_file_new_from_path_to_string(path);
	unsigned index;
	bool save_new_config_file = false;

	/* If the settings configuration file was found, parse the settings into the settings manager. */
	if (settings_config)
	{
		int bool_settings_size = sizeof(settings->bools) / sizeof(settings->bools.reference);
		int unsigned_settings_size = sizeof(settings->uints) / sizeof(settings->uints.reference);
		int float_settings_size = sizeof(settings->floats) / sizeof(settings->floats.reference);
		int string_settings_size = sizeof(settings->strings) / sizeof(settings->strings.reference);
		struct BoolSetting* bool_settings = NULL;
		struct UnsignedSetting* unsigned_settings = NULL;
		struct FloatSetting* float_settings = NULL;
		struct StringSetting* string_settings = NULL;

		bool_settings = InitializeBoolSettings(&bool_settings_size);

		for (index = 0; index < (unsigned)bool_settings_size; index++)
		{
			bool value = false;
			if (config_get_bool(settings_config, bool_settings[index].id, &value))
				*bool_settings[index].ptr = value;
		}

		unsigned_settings = InitializeUnsignedSettings(&unsigned_settings_size);

		for (index = 0; index < (unsigned)unsigned_settings_size; index++)
		{
			unsigned value = 0;
			if (config_get_uint(settings_config, unsigned_settings[index].id, &value))
			{
				VerifyNumericSetting(value, unsigned_settings[index]);

				*unsigned_settings[index].ptr = value;
			}
		}

		float_settings = InitializeFloatSettings(&float_settings_size);

		for (index = 0; index < (unsigned)float_settings_size; index++)
		{
			float value = 0.0;
			if (config_get_float(settings_config, float_settings[index].id, &value))
			{
				VerifyNumericSetting(value, float_settings[index]);

				*float_settings[index].ptr = value;
			}
		}

		string_settings = InitializeStringSettings(&string_settings_size);

		for (index = 0; index < (unsigned)string_settings_size; index++)
		{
			char value[VALUE_MAX_LENGTH];
			value[0] = '\0';

			if (config_get_array(settings_config, string_settings[index].id, value, sizeof(value)))
			{
				VerifyStringSetting(value, string_settings[index]);

				strlcpy(string_settings[index].ptr, value, VALUE_MAX_LENGTH);
			}
		}

		if (bool_settings)
			free(bool_settings);
		if (unsigned_settings)
			free(unsigned_settings);
		if (float_settings)
			free(float_settings);
		if (string_settings)
			free(string_settings);
	}
	else
	{
		save_new_config_file = true;
	}

	if (settings_config)
		config_file_free(settings_config);

	/* If no settings configuration file was found, create a new one with default settings. */
	if (save_new_config_file)
	{
		if(!SaveSettingsConfigFile(path))
		{
			lmc_trace(LMC_LOG_ERRORS,
				"[Settings]: Failed to create new settings configuration file at: \"%s\".\n",
				path);

			return false;
		}

		lmc_trace(LMC_LOG_VERBOSE,
			"[Settings]: Creating new settings configuration file at: \"%s\".\n",
			path);
	}

	return true;
}

/* Load the settings configuration file and parse the values into the settings manager. */
static bool ParseSettingsConfigFile(void)
{
	SettingsManager* settings = GetSettingsManagerContext();

	if (!LoadSettingsConfigFile(settings->paths.path_settings_config_file))
	{
		lmc_trace(LMC_LOG_ERRORS,
			"[Settings]: Failed to initialize settings configuration file at: \"%s\".\n",
			settings->paths.path_settings_config_file);

		LMC_SetLastError(LMC_ERR_FAIL_CONFIG_INIT);

		return false;
	}

	return true;
}

/**************************************************************************************************
 * SettingsManager Functions
 *************************************************************************************************/

/* Returns the current default initialization info context. */
DefaultInfo* GetDefaultInfoContext(void)
{
	return &default_initialization_info;
}

/* Returns the current settings manager context. */
SettingsManager* GetSettingsManagerContext(void)
{
	return &settings_manager;
}

/* Get Output enumeration value from the setting string. */
LMC_VideoOutput GetOutputEnumFromSetting(void)
{
	SettingsManager* settings = GetSettingsManagerContext();

	if (string_is_equal(settings->strings.video_output_type, "raw"))
		return LMC_VIDEO_OUTPUT_RAW;
	else if (string_is_equal(settings->strings.video_output_type, "adjusted_3-2"))
		return LMC_VIDEO_OUTPUT_ADJUSTED_3_2;
	else if (string_is_equal(settings->strings.video_output_type, "adjusted_4-3"))
		return LMC_VIDEO_OUTPUT_ADJUSTED_4_3;
	else if (string_is_equal(settings->strings.video_output_type, "adjusted_5-4"))
		return LMC_VIDEO_OUTPUT_ADJUSTED_5_4;
	else if (string_is_equal(settings->strings.video_output_type, "adjusted_16-9"))
		return LMC_VIDEO_OUTPUT_ADJUSTED_16_9;
	else
	{
		lmc_trace(LMC_LOG_VERBOSE,
			"[Settings]: \"%s\" is not a valid output setting. Setting to \"adjusted_4-3\" instead.",
			settings->strings.video_output_type);

		return LMC_VIDEO_OUTPUT_ADJUSTED_4_3;
	}
}

/* Get CRTMask enumeration value from the setting string. */
LMC_CRTMask GetCRTMaskEnumFromSetting(void)
{
	SettingsManager* settings = GetSettingsManagerContext();

	if (string_is_equal(settings->strings.video_crt_effect_mask_type, "slot"))
		return LMC_CRT_SLOT;
	else if (string_is_equal(settings->strings.video_crt_effect_mask_type, "shadow"))
		return LMC_CRT_SHADOW;
	else if (string_is_equal(settings->strings.video_crt_effect_mask_type, "aperture"))
		return LMC_CRT_APERTURE;
	else
		return LMC_CRT_SLOT;
}

/* Get KeyCode enumeration value from the provided key string. */
LMC_KeyCode GetKeyCodeEnumFromString(const char* key_string)
{
	LMC_KeyCode key_code = LMC_KEY_UNKNOWN;
	if (key_string && (key_string[0] != '\0'))
	{
		if (string_is_equal(key_string, "return") || string_is_equal(key_string, "enter"))
			key_code = LMC_KEY_RETURN;
		else if (string_is_equal(key_string, "space"))
			key_code = LMC_KEY_SPACE;
		else if (string_is_equal(key_string, "up"))
			key_code = LMC_KEY_UP;
		else if (string_is_equal(key_string, "down"))
			key_code = LMC_KEY_DOWN;
		else if (string_is_equal(key_string, "right"))
			key_code = LMC_KEY_RIGHT;
		else if (string_is_equal(key_string, "left"))
			key_code = LMC_KEY_LEFT;
		else if (string_is_equal(key_string, "escape"))
			key_code = LMC_KEY_ESCAPE;
		else if (string_is_equal(key_string, "backspace"))
			key_code = LMC_KEY_BACKSPACE;
		else if (string_is_equal(key_string, "tab"))
			key_code = LMC_KEY_TAB;
		else if (string_is_equal(key_string, "right_shift"))
			key_code = LMC_KEY_RSHIFT;
		else if (string_is_equal(key_string, "left_shift"))
			key_code = LMC_KEY_LSHIFT;
		else if (string_is_equal(key_string, "right_control"))
			key_code = LMC_KEY_RCTRL;
		else if (string_is_equal(key_string, "left_control"))
			key_code = LMC_KEY_LCTRL;
		else if (string_is_equal(key_string, "right_alt"))
			key_code = LMC_KEY_RALT;
		else if (string_is_equal(key_string, "left_alt"))
			key_code = LMC_KEY_LALT;
		else if (string_is_equal(key_string, "right_meta"))
			key_code = LMC_KEY_RMETA;
		else if (string_is_equal(key_string, "left_meta"))
			key_code = LMC_KEY_LMETA;
		else if (string_is_equal(key_string, "left_super"))
			key_code = LMC_KEY_LSUPER;
		else if (string_is_equal(key_string, "right_super"))
			key_code = LMC_KEY_RSUPER;
		else if (string_is_equal(key_string, "delete"))
			key_code = LMC_KEY_DELETE;
		else if (string_is_equal(key_string, "insert"))
			key_code = LMC_KEY_INSERT;
		else if (string_is_equal(key_string, "home"))
			key_code = LMC_KEY_HOME;
		else if (string_is_equal(key_string, "end"))
			key_code = LMC_KEY_END;
		else if (string_is_equal(key_string, "page_up"))
			key_code = LMC_KEY_PAGEUP;
		else if (string_is_equal(key_string, "page_down"))
			key_code = LMC_KEY_PAGEDOWN;
		else if (string_is_equal(key_string, "a"))
			key_code = LMC_KEY_A;
		else if (string_is_equal(key_string, "b"))
			key_code = LMC_KEY_B;
		else if (string_is_equal(key_string, "c"))
			key_code = LMC_KEY_C;
		else if (string_is_equal(key_string, "d"))
			key_code = LMC_KEY_D;
		else if (string_is_equal(key_string, "e"))
			key_code = LMC_KEY_E;
		else if (string_is_equal(key_string, "f"))
			key_code = LMC_KEY_F;
		else if (string_is_equal(key_string, "g"))
			key_code = LMC_KEY_G;
		else if (string_is_equal(key_string, "h"))
			key_code = LMC_KEY_H;
		else if (string_is_equal(key_string, "i"))
			key_code = LMC_KEY_I;
		else if (string_is_equal(key_string, "j"))
			key_code = LMC_KEY_J;
		else if (string_is_equal(key_string, "k"))
			key_code = LMC_KEY_K;
		else if (string_is_equal(key_string, "l"))
			key_code = LMC_KEY_L;
		else if (string_is_equal(key_string, "m"))
			key_code = LMC_KEY_M;
		else if (string_is_equal(key_string, "n"))
			key_code = LMC_KEY_N;
		else if (string_is_equal(key_string, "o"))
			key_code = LMC_KEY_O;
		else if (string_is_equal(key_string, "p"))
			key_code = LMC_KEY_P;
		else if (string_is_equal(key_string, "q"))
			key_code = LMC_KEY_Q;
		else if (string_is_equal(key_string, "r"))
			key_code = LMC_KEY_R;
		else if (string_is_equal(key_string, "s"))
			key_code = LMC_KEY_S;
		else if (string_is_equal(key_string, "t"))
			key_code = LMC_KEY_T;
		else if (string_is_equal(key_string, "u"))
			key_code = LMC_KEY_U;
		else if (string_is_equal(key_string, "v"))
			key_code = LMC_KEY_V;
		else if (string_is_equal(key_string, "w"))
			key_code = LMC_KEY_W;
		else if (string_is_equal(key_string, "x"))
			key_code = LMC_KEY_X;
		else if (string_is_equal(key_string, "y"))
			key_code = LMC_KEY_Y;
		else if (string_is_equal(key_string, "z"))
			key_code = LMC_KEY_Z;
		else if (string_is_equal(key_string, "0"))
			key_code = LMC_KEY_0;
		else if (string_is_equal(key_string, "1"))
			key_code = LMC_KEY_1;
		else if (string_is_equal(key_string, "2"))
			key_code = LMC_KEY_2;
		else if (string_is_equal(key_string, "3"))
			key_code = LMC_KEY_3;
		else if (string_is_equal(key_string, "4"))
			key_code = LMC_KEY_4;
		else if (string_is_equal(key_string, "5"))
			key_code = LMC_KEY_5;
		else if (string_is_equal(key_string, "6"))
			key_code = LMC_KEY_6;
		else if (string_is_equal(key_string, "7"))
			key_code = LMC_KEY_7;
		else if (string_is_equal(key_string, "8"))
			key_code = LMC_KEY_8;
		else if (string_is_equal(key_string, "9"))
			key_code = LMC_KEY_9;
		else if (string_is_equal(key_string, "f1"))
			key_code = LMC_KEY_F1;
		else if (string_is_equal(key_string, "f2"))
			key_code = LMC_KEY_F2;
		else if (string_is_equal(key_string, "f3"))
			key_code = LMC_KEY_F3;
		else if (string_is_equal(key_string, "f4"))
			key_code = LMC_KEY_F4;
		else if (string_is_equal(key_string, "f5"))
			key_code = LMC_KEY_F5;
		else if (string_is_equal(key_string, "f6"))
			key_code = LMC_KEY_F6;
		else if (string_is_equal(key_string, "f7"))
			key_code = LMC_KEY_F7;
		else if (string_is_equal(key_string, "f8"))
			key_code = LMC_KEY_F8;
		else if (string_is_equal(key_string, "f9"))
			key_code = LMC_KEY_F9;
		else if (string_is_equal(key_string, "f10"))
			key_code = LMC_KEY_F10;
		else if (string_is_equal(key_string, "f11"))
			key_code = LMC_KEY_F11;
		else if (string_is_equal(key_string, "f12"))
			key_code = LMC_KEY_F12;
		else if (string_is_equal(key_string, "f13"))
			key_code = LMC_KEY_F13;
		else if (string_is_equal(key_string, "f14"))
			key_code = LMC_KEY_F14;
		else if (string_is_equal(key_string, "f15"))
			key_code = LMC_KEY_F15;
		else if (string_is_equal(key_string, "keypad_0"))
			key_code = LMC_KEY_KP0;
		else if (string_is_equal(key_string, "keypad_1"))
			key_code = LMC_KEY_KP1;
		else if (string_is_equal(key_string, "keypad_2"))
			key_code = LMC_KEY_KP2;
		else if (string_is_equal(key_string, "keypad_3"))
			key_code = LMC_KEY_KP3;
		else if (string_is_equal(key_string, "keypad_4"))
			key_code = LMC_KEY_KP4;
		else if (string_is_equal(key_string, "keypad_5"))
			key_code = LMC_KEY_KP5;
		else if (string_is_equal(key_string, "keypad_6"))
			key_code = LMC_KEY_KP6;
		else if (string_is_equal(key_string, "keypad_7"))
			key_code = LMC_KEY_KP7;
		else if (string_is_equal(key_string, "keypad_8"))
			key_code = LMC_KEY_KP8;
		else if (string_is_equal(key_string, "keypad_9"))
			key_code = LMC_KEY_KP9;
		else if (string_is_equal(key_string, "keypad_period"))
			key_code = LMC_KEY_KP_PERIOD;
		else if (string_is_equal(key_string, "keypad_divide"))
			key_code = LMC_KEY_KP_DIVIDE;
		else if (string_is_equal(key_string, "keypad_multiply"))
			key_code = LMC_KEY_KP_MULTIPLY;
		else if (string_is_equal(key_string, "keypad_minus"))
			key_code = LMC_KEY_KP_MINUS;
		else if (string_is_equal(key_string, "keypad_plus"))
			key_code = LMC_KEY_KP_PLUS;
		else if (string_is_equal(key_string, "keypad_enter"))
			key_code = LMC_KEY_KP_ENTER;
		else if (string_is_equal(key_string, "keypad_equals"))
			key_code = LMC_KEY_KP_EQUALS;
		else if (string_is_equal(key_string, "clear"))
			key_code = LMC_KEY_CLEAR;
		else if (string_is_equal(key_string, "pause"))
			key_code = LMC_KEY_PAUSE;
		else if (string_is_equal(key_string, "exclamation"))
			key_code = LMC_KEY_EXCLAIM;
		else if (string_is_equal(key_string, "double_quote"))
			key_code = LMC_KEY_QUOTEDBL;
		else if (string_is_equal(key_string, "hash"))
			key_code = LMC_KEY_HASH;
		else if (string_is_equal(key_string, "dollar"))
			key_code = LMC_KEY_DOLLAR;
		else if (string_is_equal(key_string, "ampersand"))
			key_code = LMC_KEY_AMPERSAND;
		else if (string_is_equal(key_string, "quote"))
			key_code = LMC_KEY_QUOTE;
		else if (string_is_equal(key_string, "left_parenthesis"))
			key_code = LMC_KEY_LEFTPAREN;
		else if (string_is_equal(key_string, "right_parenthesis"))
			key_code = LMC_KEY_RIGHTPAREN;
		else if (string_is_equal(key_string, "asterisk"))
			key_code = LMC_KEY_ASTERISK;
		else if (string_is_equal(key_string, "plus"))
			key_code = LMC_KEY_PLUS;
		else if (string_is_equal(key_string, "comma"))
			key_code = LMC_KEY_COMMA;
		else if (string_is_equal(key_string, "minus"))
			key_code = LMC_KEY_MINUS;
		else if (string_is_equal(key_string, "period"))
			key_code = LMC_KEY_PERIOD;
		else if (string_is_equal(key_string, "slash"))
			key_code = LMC_KEY_SLASH;
		else if (string_is_equal(key_string, "colon"))
			key_code = LMC_KEY_COLON;
		else if (string_is_equal(key_string, "semicolon"))
			key_code = LMC_KEY_SEMICOLON;
		else if (string_is_equal(key_string, "less_than"))
			key_code = LMC_KEY_LESS;
		else if (string_is_equal(key_string, "equals"))
			key_code = LMC_KEY_EQUALS;
		else if (string_is_equal(key_string, "greater_than"))
			key_code = LMC_KEY_GREATER;
		else if (string_is_equal(key_string, "question"))
			key_code = LMC_KEY_QUESTION;
		else if (string_is_equal(key_string, "at"))
			key_code = LMC_KEY_AT;
		else if (string_is_equal(key_string, "left_bracket"))
			key_code = LMC_KEY_LEFTBRACKET;
		else if (string_is_equal(key_string, "backslash"))
			key_code = LMC_KEY_BACKSLASH;
		else if (string_is_equal(key_string, "right_bracket"))
			key_code = LMC_KEY_RIGHTBRACKET;
		else if (string_is_equal(key_string, "caret"))
			key_code = LMC_KEY_CARET;
		else if (string_is_equal(key_string, "underscore"))
			key_code = LMC_KEY_UNDERSCORE;
		else if (string_is_equal(key_string, "backquote"))
			key_code = LMC_KEY_BACKQUOTE;
		else if (string_is_equal(key_string, "num_lock"))
			key_code = LMC_KEY_NUMLOCK;
		else if (string_is_equal(key_string, "caps_lock"))
			key_code = LMC_KEY_CAPSLOCK;
		else if (string_is_equal(key_string, "scroll_lock"))
			key_code = LMC_KEY_SCROLLOCK;
		else if (string_is_equal(key_string, "mode"))
			key_code = LMC_KEY_MODE;
		else if (string_is_equal(key_string, "help"))
			key_code = LMC_KEY_HELP;
		else if (string_is_equal(key_string, "menu"))
			key_code = LMC_KEY_MENU;
		else if (string_is_equal(key_string, "print"))
			key_code = LMC_KEY_PRINT;
		else if (string_is_equal(key_string, "left_brace"))
			key_code = LMC_KEY_LEFTBRACE;
		else if (string_is_equal(key_string, "bar"))
			key_code = LMC_KEY_BAR;
		else if (string_is_equal(key_string, "right_brace"))
			key_code = LMC_KEY_RIGHTBRACE;
		else if (string_is_equal(key_string, "tilde"))
			key_code = LMC_KEY_TILDE;
		else if (string_is_equal(key_string, "compose"))
			key_code = LMC_KEY_COMPOSE;
		else if (string_is_equal(key_string, "break"))
			key_code = LMC_KEY_BREAK;
		else if (string_is_equal(key_string, "undo"))
			key_code = LMC_KEY_UNDO;
		else if (string_is_equal(key_string, "euro"))
			key_code = LMC_KEY_EURO;
		else if (string_is_equal(key_string, "power"))
			key_code = LMC_KEY_POWER;
		else if (string_is_equal(key_string, "system_request"))
			key_code = LMC_KEY_SYSREQ;
		else if (string_is_equal(key_string, "oem_102"))
			key_code = LMC_KEY_OEM_102;
	}

	return key_code;
}

/* Loads a settings config file if present or sets defaults if not. */
bool InitializeSettings(void)
{
	SettingsManager* settings = GetSettingsManagerContext();
	bool result = false;

	/* Resolve and store path to settings.cfg file. */
	fill_pathname_join(settings->paths.path_settings_config_file,
		settings->paths.path_setting_directory,
		SETTINGS_FILE_NAME,
		sizeof(settings->paths.path_settings_config_file));

	/* Initialize all settings to defaults. */
	SetSettingsDefaults();

	/* Set settings from settings configuration file (if present). */
	result = ParseSettingsConfigFile();

	if (result)
	{
		lmc_trace(LMC_LOG_VERBOSE, "[Settings]: Settings initialized successfully.");
	}

	return result;
}

/* Sets all settings to default values. */
void SetSettingsDefaults(void)
{
	SettingsManager* settings = GetSettingsManagerContext();
	int bool_settings_size = sizeof(settings->bools) / sizeof(settings->bools.reference);
	int unsigned_settings_size = sizeof(settings->uints) / sizeof(settings->uints.reference);
	int float_settings_size = sizeof(settings->floats) / sizeof(settings->floats.reference);
	int string_settings_size = sizeof(settings->strings) / sizeof(settings->strings.reference);
	unsigned i;

	struct BoolSetting* bool_settings = InitializeBoolSettings(&bool_settings_size);
	struct UnsignedSetting* unsigned_settings = InitializeUnsignedSettings(&unsigned_settings_size);
	struct FloatSetting* float_settings = InitializeFloatSettings(&float_settings_size);
	struct StringSetting* string_settings = InitializeStringSettings(&string_settings_size);

	if (bool_settings && (bool_settings_size > 0))
	{
		for (i = 0; i < (unsigned)bool_settings_size; i++)
		{
			if (bool_settings[i].use_default)
				*bool_settings[i].ptr = bool_settings[i].default_value;
		}

		free(bool_settings);
	}

	if (unsigned_settings && (unsigned_settings_size > 0))
	{
		for (i = 0; i < (unsigned)unsigned_settings_size; i++)
		{
			if (unsigned_settings[i].use_default)
				*unsigned_settings[i].ptr = unsigned_settings[i].default_value;
		}
		free(unsigned_settings);
	}

	if (float_settings && (float_settings_size > 0))
	{
		for (i = 0; i < (unsigned)float_settings_size; i++)
		{
			if (float_settings[i].use_default)
				*float_settings[i].ptr = float_settings[i].default_value;
		}

		free(float_settings);
	}

	if (string_settings && (string_settings_size > 0))
	{
		for (i = 0; i < (unsigned)string_settings_size; i++)
		{
			if (string_settings[i].use_default)
			{
				strlcpy(string_settings[i].ptr, string_settings[i].default_value, VALUE_MAX_LENGTH);
			}
		}

		free(string_settings);
	}
}

/* Save current settings to file. */
bool SaveSettingsConfigFile(const char* path)
{
	SettingsManager* settings = GetSettingsManagerContext();
	config_file_t* settings_config = config_file_new_from_path_to_string(path);
	bool result = false;
	unsigned i;

	struct BoolSetting* bool_settings = NULL;
	struct UnsignedSetting* unsigned_settings = NULL;
	struct FloatSetting* float_settings = NULL;
	struct StringSetting* string_settings = NULL;

	int bool_settings_size = sizeof(settings->bools) / sizeof(settings->bools.reference);
	int unsigned_settings_size = sizeof(settings->uints) / sizeof(settings->uints.reference);
	int float_settings_size = sizeof(settings->floats) / sizeof(settings->floats.reference);
	int string_settings_size = sizeof(settings->strings) / sizeof(settings->strings.reference);

	if (!settings_config)
	{
		settings_config = config_file_new_alloc();
		if (!settings_config)
		{
			lmc_trace(LMC_LOG_ERRORS,
				"[Settings]: Failed to allocate memory for new settings configuration");
			return false;
		}
	}

	/* Initialize, verify, and set ALL settings by type. */
	bool_settings = InitializeBoolSettings(&bool_settings_size);

	if (bool_settings && (bool_settings_size > 0))
	{
		for (i = 0; i < (unsigned)bool_settings_size; i++)
		{
			config_set_bool(settings_config, bool_settings[i].id, *bool_settings[i].ptr);
		}

		free(bool_settings);
	}

	unsigned_settings = InitializeUnsignedSettings(&unsigned_settings_size);

	if (unsigned_settings && (unsigned_settings_size > 0))
	{
		for (i = 0; i < (unsigned)unsigned_settings_size; i++)
		{
			VerifyNumericSetting(*unsigned_settings[i].ptr, unsigned_settings[i]);

			config_set_uint(settings_config, unsigned_settings[i].id, *unsigned_settings[i].ptr);
		}

		free(unsigned_settings);
	}

	float_settings = InitializeFloatSettings(&float_settings_size);

	if (float_settings && (float_settings_size > 0))
	{
		for (i = 0; i < (unsigned)float_settings_size; i++)
		{
			VerifyNumericSetting(*float_settings[i].ptr, float_settings[i]);

			config_set_float(settings_config, float_settings[i].id, *float_settings[i].ptr);
		}

		free(float_settings);
	}

	string_settings = InitializeStringSettings(&string_settings_size);

	if (string_settings && (string_settings_size > 0))
	{
		for (i = 0; i < (unsigned)string_settings_size; i++)
		{
			VerifyStringSetting(*string_settings[i].ptr, string_settings[i]);

			config_set_string(settings_config, string_settings[i].id, string_settings[i].ptr);
		}

		free(string_settings);
	}

	/* Write settings configuration to file. */
	result = config_file_write(settings_config, path, true);
	config_file_free(settings_config);

	if (result)
		settings->modified = false;

	return result;
}
