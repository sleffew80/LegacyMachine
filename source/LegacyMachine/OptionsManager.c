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
#include <string/stdstring.h>

#include "OptionsManager.h"
#include "Logging.h"

/**************************************************************************************************
 * OptionsManager Context
 *************************************************************************************************/

static OptionsManager options_manager = { 0 };

/**************************************************************************************************
 * OptionsManager Static Functions
 *************************************************************************************************/

/* Builds and stores a path to the options configuration file. */
static bool InitializeOptionsPath(const char* base_path, const char* library_name)
{
	OptionsManager* options = GetOptionsManagerContext();
	const struct retro_variable* variable = NULL;
	char* options_filename = (char*)malloc(NAME_MAX_LENGTH);
	char* options_ext = ".cfg";

	if (!options_filename)
	{
		lmc_core_log(RETRO_LOG_ERROR,
			"[Options]: Failed to allocate memory for storing options configuration file name");
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		return false;
	}

	strlcpy(options_filename, library_name, NAME_MAX_LENGTH);
	strlcat(options_filename, options_ext, NAME_MAX_LENGTH);
	strlcpy(options->path, base_path, sizeof(options->path));
	fill_pathname_slash(options->path, sizeof(options->path));
	strlcat(options->path, options_filename, sizeof(options->path));

	free(options_filename);

	return true;
}

/* Load an existing options configuration file or allocate space for a new one. */
static config_file_t* LoadOptionsConfigFile(void)
{
	OptionsManager* options = GetOptionsManagerContext();
	config_file_t* options_config = config_file_new_from_path_to_string(options->path);

	if (!options_config)
	{
		options_config = config_file_new_alloc();
		if (!options_config)
		{
			lmc_core_log(RETRO_LOG_ERROR,
				"[Options]: Failed to allocate memory for new options configuration");
			return NULL;
		}
		options_config->modified = true;
		options->new = true;
	}

	return options_config;
}

/* Save option manager entry values to a configuration file. */
static bool SaveOptionsConfigFile(config_file_t* options_config)
{
	OptionsManager* options = GetOptionsManagerContext();

	if ((options_config) && (options->size > 0))
	{
		for (size_t i = 0; i < options->size; i++)
		{
				CoreOption* option = (CoreOption*)&options->entries[i];

					if (option)
						config_set_string(options_config, option->key,
							option->values->elems[option->current_index].data);
				}

				return config_file_write(options_config, options->path, true);
			}

			lmc_core_log(RETRO_LOG_ERROR,
				"[Options]: Failed to save options configuration file: \"%s\".\n", options->path);

	return false;
}

/* Get the option entry of the provided key. */
static CoreOption* GetOptionEntry(const char* key)
{
	OptionsManager* options = GetOptionsManagerContext();

	for (size_t i = 0; i < options->size; i++)
	{
		const char* option_key = options->entries[i].key;

		if (string_is_equal(option_key, key))
		{
			return &options->entries[i];
		}
	}
	return NULL;
}

/**************************************************************************************************
 * OptionsManager Functions
 *************************************************************************************************/

/* Returns the current core options manager context. */
OptionsManager* GetOptionsManagerContext(void)
{
	return &options_manager;
}

/* Initialize core options. */
void InitializeOptions(const char* base_path, const char* library_name,
	const struct retro_core_option_definition* definitions)
{
	OptionsManager* options = GetOptionsManagerContext();
	const struct retro_core_option_definition* definition = NULL;
	config_file_t* options_config = NULL;
	size_t option_index;
	size_t value_index;
	size_t value_size;

	if (!InitializeOptionsPath(base_path, library_name))
		return;

	options_config = LoadOptionsConfigFile();

	if (!options_config)
	{
		lmc_core_log(RETRO_LOG_ERROR, 
			"[Options]: Failed to allocate memory for core options config");
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		DeinitializeOptions();
		return;
	}

	for (option_index = 0; definitions[option_index].key; option_index++)
		options->size++;

	if (options->size < 1)
		return;

	options->entries = (CoreOption*)calloc(options->size, sizeof(*options->entries));

	if (!options->entries)
	{
		lmc_core_log(RETRO_LOG_ERROR, 
			"[Options]: Failed to allocate memory for core option entries");
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		DeinitializeOptions();
		return;
	}

	option_index = 0;

	for (definition = definitions;
		definition->key && definition->desc && definition->values[0].value;
		definition++, option_index++)
	{
		CoreOption* option = (CoreOption*)&options->entries[option_index];
		const struct retro_core_option_value* option_values = definition->values;
		union string_list_elem_attr elem_attr;

		option->visible = true;

		if (!string_is_empty(definition->key))
			option->key = strdup(definition->key);

		if (!string_is_empty(definition->info))
			option->info = strdup(definition->info);

		if (!string_is_empty(definition->desc))
			option->description = strdup(definition->desc);

		value_size = 0;

		for (value_index = 0; option_values[value_index].value; value_index++)
			value_size++;

		if (value_size < 1)
			return;

		elem_attr.i = 0;
		option->values = string_list_new();
		option->labels = string_list_new();

		if (!option->values || !option->labels)
			return;

		for (value_index = 0; value_index < value_size; value_index++)
		{
			const char* value = option_values[value_index].value;
			const char* label = option_values[value_index].label;

			if (string_is_empty(label))
				label = value;

			string_list_append(option->values, value, elem_attr);

			string_list_append(option->labels, label, elem_attr);

			if (!string_is_empty(definition->default_value))
			{
				if (string_is_equal(definition->default_value, value))
				{
					option->default_index = value_index;
					option->previous_index = value_index;
					option->current_index = value_index;
				}
			}
		}

		if (!options->new)
		{
			struct config_entry_list* config_entry = NULL;
			size_t entry_index;

			config_entry = config_get_entry(options_config, option->key);

			if (config_entry && !string_is_empty(config_entry->value))
			{
				for (entry_index = 0; entry_index < option->values->size; entry_index++)
				{
					const char* option_value = option->values->elems[entry_index].data;

					if (string_is_equal(option_value, config_entry->value))
					{
						option->current_index = entry_index;
						break;
					}
				}
			}
		}
	}

	if (options->new)
	{
		if (!SaveOptionsConfigFile(options_config))
		{
			lmc_core_log(RETRO_LOG_ERROR,
				"[Options]: Failed to create new core options configuration file: \"%s\".\n", 
				options->path);
		}

		options->new = false;

		lmc_core_log(RETRO_LOG_INFO,
			"[Options]: Created new core options configuration file: \"%s\".\n", 
			options->path);
	}

	lmc_core_log(RETRO_LOG_INFO, "[Options] Core options initialized successfully");

	if (options_config)
		config_file_free(options_config);

	options->initialized = true;
}

/* Initialize core options from variables. */
void InitializeOptionsVariables(const char* base_path, const char* library_name,
	const struct retro_variable* variables)
{
	OptionsManager* options = GetOptionsManagerContext();
	const struct retro_variable* variable = NULL;
	config_file_t* options_config = NULL;
	size_t option_index = 0;
	size_t value_index = 0;

	if (!InitializeOptionsPath(base_path, library_name))
		return;

	options_config = LoadOptionsConfigFile();

	if (!options_config)
	{
		lmc_core_log(RETRO_LOG_ERROR, 
			"[Options]: Failed to allocate memory for core options config");
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		DeinitializeOptions();
		return;
	}

	for (variable = variables; variable->key && variable->value; variable++)
		options->size++;

	options->entries = (CoreOption*)calloc(options->size, sizeof(*options->entries));

	if (!options->entries)
	{
		lmc_core_log(RETRO_LOG_ERROR, 
			"[Options]: Failed to allocate memory for core option entries");
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		DeinitializeOptions();
		return;
	}

	for (variable = variables; variable->key && variable->value; variable++, option_index++)
	{
		CoreOption* option = (CoreOption*)&options->entries[option_index];
		union string_list_elem_attr elem_attr;
		char* variable_value = NULL;
		char* value_start = NULL;
		char* description_end = NULL;

		if (!string_is_empty(variable->key))
		{
			option->key = strdup(variable->key);
		}

		if (!string_is_empty(variable->value))
			variable_value = strdup(variable->value);

		if (variable_value)
			if (!string_is_empty(variable_value))
				description_end = strstr(variable_value, "; ");

		if (!description_end)
		{
			lmc_core_log(RETRO_LOG_ERROR,
				"[Options]: Failed to allocate memory for parsing core option variable values");
			LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
			free(variable_value);
			DeinitializeOptions();
			return;
		}

		*description_end = '\0';

		if (!string_is_empty(variable_value))
			option->description = strdup(variable_value);

		value_start = description_end + 2;

		option->values = string_split(value_start, "|");
		option->labels = string_list_new();
		elem_attr.i = 0;

		if (!option->values)
		{
			lmc_core_log(RETRO_LOG_ERROR, 
				"[Options]: Failed to allocate memory for core option entry values");
			LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
			free(variable_value);
			DeinitializeOptions();
			return;
		}

		for (value_index = 0; value_index < option->values->size; value_index++)
		{
			const char* value = option->values->elems[value_index].data;
			const char* label = value;

			string_list_append(option->labels, label, elem_attr);
		}

		option->default_index = 0;
		option->previous_index = 0;
		option->current_index = 0;
		option->visible = true;

		if (!options->new)
		{
			struct config_entry_list* config_entry = NULL;
			size_t entry_index = 0;

			config_entry = config_get_entry(options_config, option->key);

			if (config_entry && !string_is_empty(config_entry->value))
			{
				for (entry_index = 0; entry_index < option->values->size; entry_index++)
				{
					const char* option_value = option->values->elems[entry_index].data;

					if (string_is_equal(option_value, config_entry->value))
					{
						option->current_index = entry_index;
						break;
					}
				}
			}
		}

		if (variable_value)
			free(variable_value);
	}

	if (options->new)
	{
		if (!SaveOptionsConfigFile(options_config))
		{
			lmc_core_log(RETRO_LOG_ERROR,
				"[Options]: Failed to create new core options configuration file: \"%s\".\n",
				options->path);
		}

		options->new = false;

		lmc_core_log(RETRO_LOG_INFO,
			"[Options]: Created new core options configuration file: \"%s\".\n",
			options->path);
	}

	lmc_core_log(RETRO_LOG_INFO, "[Options] Core options initialized successfully");

	if (options_config)
		config_file_free(options_config);

	options->initialized = true;
}

/* Free all option entries and deinitialize core options. */
void DeinitializeOptions(void)
{
	OptionsManager* options = GetOptionsManagerContext();
	size_t i;

	for (i = 0; i < options->size; i++)
	{
		if (options->entries[i].labels)
			string_list_free(options->entries[i].labels);
		if (options->entries[i].values)
			string_list_free(options->entries[i].values);
		if (options->entries[i].info)
			free(options->entries[i].info);
		if (options->entries[i].description)
			free(options->entries[i].description);
		if (options->entries[i].key)
			free(options->entries[i].key);

		options->entries[i].key = NULL;
		options->entries[i].description = NULL;
		options->entries[i].info = NULL;
		options->entries[i].values = NULL;
		options->entries[i].labels = NULL;
	}

	free(options->entries);

	memset(options, 0, sizeof(OptionsManager));
}

/* Returns the value of an option identified by the specified key. */
const char* GetOptionValue(const char* key)
{
	CoreOption* option = GetOptionEntry(key);

	if (option)
		return option->values->elems[option->current_index].data;

	return NULL;
}

/* Sets the visibility of an option identified by the specified key. */
void SetOptionVisibility(const char* key, bool visible)
{
	CoreOption* option = GetOptionEntry(key);

	if (option)
		option->visible = visible;
}
