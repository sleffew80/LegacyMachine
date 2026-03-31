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
#include <string/stdstring.h>

#include "LegacyMachine.h"
#include "StateList.h"
#include "Logging.h"

/**************************************************************************************************
 * StateList Functions
 *************************************************************************************************/

/* Create a new state list. */
void CreateStateList(StateList** state_list, int initial_capacity,
	void* (*cb_init)(void), void (*cb_deinit)(void*))
{
	StateList* list = NULL;

	if (!state_list)
		return;

	list = *state_list;

	if (list)
		FreeStateList(state_list);

	list = (StateList*)malloc(sizeof(StateList));

	if (!list)
	{
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		lmc_trace(LMC_LOG_ERRORS, "Failed to allocate memory for state list");
		*state_list = NULL;
		return;
	}

	*state_list = list;
	list->contexts = (void**)calloc(initial_capacity, sizeof(void*));
	if (!list->contexts)
	{
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		lmc_trace(LMC_LOG_ERRORS, "Failed to allocate memory for contexts array for state list");
		free(list);
		*state_list = NULL;
		return;
	}
	list->cb_init = cb_init;
	list->cb_deinit = cb_deinit;
	list->size = 0;
	list->capacity = initial_capacity;
}

/* Add a new item to the state list and return its context pointer. */
void* AddStateListItem(StateList* state_list)
{
	int old_size;

	if (!state_list)
		return NULL;

	old_size = state_list->size;

	ResizeStateList(state_list, old_size + 1, true);

	return state_list->contexts[old_size];
}

/* Resize the state list to the new size. */
void ResizeStateList(StateList* state_list, int new_size, bool do_init)
{
	void* element = NULL;
	void** new_contexts = NULL;
	int new_capacity;
	int old_size;
	int index;

	if (new_size < 0)
		new_size = 0;
	new_capacity = new_size;
	old_size = state_list->size;

	if (new_size == old_size)
		return;

	if (new_size > state_list->capacity)
	{
		if (new_capacity < state_list->capacity * 2)
			new_capacity = state_list->capacity * 2;

		new_contexts = (void**)realloc(state_list->contexts, sizeof(void*) * new_capacity);
		if (!new_contexts)
		{
			lmc_trace(LMC_LOG_ERRORS,
				"Failed to resize state list to capacity %d",
				new_capacity);

			LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);

			return;
		}
		state_list->contexts = new_contexts;

		for (index = state_list->capacity; index < new_capacity; index++)
			state_list->contexts[index] = NULL;

		state_list->capacity = new_capacity;
	}

	/* Safety check (satisfy compiler). */
	retro_assert(state_list->size <= state_list->capacity);

	if (new_size <= state_list->size)
	{
		for (index = new_size; index < state_list->size; index++)
		{
			if (state_list->cb_deinit && state_list->contexts[index])
				state_list->cb_deinit(state_list->contexts[index]);
			state_list->contexts[index] = NULL;
		}
	}
	else
	{
		for (index = state_list->size; index < new_size; index++)
		{
			state_list->contexts[index] = NULL;
			if (state_list->cb_init && do_init)
				state_list->contexts[index] = state_list->cb_init();
		}
	}

	state_list->size = new_size;
}

/* Free the state list and all its items. */
void FreeStateList(StateList** state_list)
{
	if (!state_list)
		return;

	StateList* list = *state_list;

	if (list)
	{
		ResizeStateList(list, 0, false);
		if (list->contexts)
			free(list->contexts);
		free(list);
		*state_list = NULL;
	}
}
