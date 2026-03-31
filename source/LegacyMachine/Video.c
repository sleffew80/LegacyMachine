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
#include <compat/strl.h>

#include "LegacyMachine.h"
#include "MainEngine.h"

/**************************************************************************************************
 * Internal Static Functions
 *************************************************************************************************/

/* Get Output setting string from the enumeration value. */
static const char* GetVideoOutputSettingFromEnum(LMC_VideoOutput type)
{
	switch (type)
	{
		case LMC_VIDEO_OUTPUT_RAW:
			return "raw";
		case LMC_VIDEO_OUTPUT_ADJUSTED_3_2:
			return "adjusted_3-2";
		case LMC_VIDEO_OUTPUT_ADJUSTED_4_3:
			return "adjusted_4-3";
		case LMC_VIDEO_OUTPUT_ADJUSTED_5_4:
			return "adjusted_5-4";
		case LMC_VIDEO_OUTPUT_ADJUSTED_16_9:
			return "adjusted_16-9";
		default:
			return "raw";
	}
}

/* Get Output setting string from the enumeration value. */
static const char* GetCRTMaskSettingFromEnum(LMC_CRTMask type)
{
	switch (type)
	{
		case LMC_CRT_SLOT:
			return "slot";
		case LMC_CRT_SHADOW:
			return "shadow";
		case LMC_CRT_APERTURE:
			return "aperture";
		default:
			return "slot";
	}
}

/**************************************************************************************************
 * LegacyMachine Video Management
 *************************************************************************************************/

/* Sets the active video output type. */
void LMC_SetVideoOutput(LMC_VideoOutput type)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	SettingsManager* settings = GetSettingsManagerContext();

	if (type != (LMC_VideoOutput)legacy_machine->video->output.type)
	{
		legacy_machine->video->output.type = (VideoOutput)type;

		SetStringSetting(settings, settings->strings.video_output_type, GetVideoOutputSettingFromEnum(type));

		legacy_machine->window->params.override_width = 0;
		legacy_machine->window->params.override_height = 0;
		legacy_machine->window->params.override_aspect = 0.0f;
		legacy_machine->window->params.factor = 0;

		SetBaseOverrideDimensions(type);

		if (legacy_machine->window->initialized)
		{
			/* Reinitialize window to apply new override dimensions and aspect ratio. */
			WindowDriverID driver_id = GetWindowDriverID();
			DeinitializeWindow();
			InitializeWindow(driver_id);
		}
	}
}

/**************************************************************************************************
 * LegacyMachine CRT Effect Management
 *************************************************************************************************/

/* Sets the mask type to use with the CRT effect. */
void LMC_SetCRTMask(LMC_CRTMask type, bool update_settings)
{
	CRTEffect* crt_effect = GetCRTEffect();
	SettingsManager* settings = GetSettingsManagerContext();

	crt_effect->type = (CRTMask)type;

	if (settings->initialized)
		if (update_settings)
			SetStringSetting(settings, settings->strings.video_crt_effect_mask_type, GetCRTMaskSettingFromEnum(type));
}

/* Gets the mask type currently being used by the CRT effect. */
LMC_CRTMask LMC_GetCRTMask(void)
{
	CRTEffect* crt_effect = GetCRTEffect();

	return (LMC_CRTMask)crt_effect->type;
}