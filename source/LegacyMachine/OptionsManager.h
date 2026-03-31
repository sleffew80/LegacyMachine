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

#ifndef _OPTIONS_MANAGER_H
#define _OPTIONS_MANAGER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <file/config_file.h>
#include <lists/string_list.h>

#include "LegacyMachine.h"

/**************************************************************************************************
 * CoreOption Structure
 *************************************************************************************************/

/* Structure for storing a single core option's values. */
typedef struct CoreOption
{
	char*				key;			/* Option key identifier. */
	char*				description;	/* Human-readable option description. */
	char*				info;			/* Optional extended info string. */
	size_t				current_index;	/* Index of the currently selected value. */
	size_t				previous_index;	/* Index of the previously selected value. */
	size_t				default_index;	/* Index of the default value. */
	struct string_list*	values;			/* List of valid value strings. */
	struct string_list*	labels;			/* List of display label strings. */
	bool				visible;		/* Whether this option is currently visible. */
}
CoreOption;

/**************************************************************************************************
 * OptionsManager Structure
 *************************************************************************************************/

typedef struct OptionsManager
{
	char		path[PATH_MAX_LENGTH];	/* File path to the options configuration file. */
	CoreOption*	entries;				/* Array of core option entries. */
	size_t		size;					/* Number of option entries. */
	bool		content_specific;		/* Whether options are content-specific. */
	bool		initialized;			/* Whether the options manager is initialized. */
	bool		modified;				/* Whether any option values have been modified. */
	bool		new;					/* Whether this is a newly created options file. */
}
OptionsManager;

/**************************************************************************************************
 * OptionsManager Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

OptionsManager* GetOptionsManagerContext(void);

void InitializeOptions(const char* base_path, const char* library_name,
	const struct retro_core_option_definition* definitions);
void InitializeOptionsVariables(const char* base_path, const char* library_name,
	const struct retro_variable* variables);
void DeinitializeOptions(void);
const char* GetOptionValue(const char* key);
void SetOptionVisibility(const char* key, bool visible);

RETRO_END_DECLS

#endif
