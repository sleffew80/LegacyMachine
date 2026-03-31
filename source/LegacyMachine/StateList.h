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

#ifndef _STATE_LIST_H
#define _STATE_LIST_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <retro_common_api.h>

/**************************************************************************************************
 * StateList Structure
 *************************************************************************************************/

typedef struct StateList
{
	void**	contexts;				/* Array of pointers to individual context items. */
	void*	(*cb_init)(void);		/* Optional callback to initialize a new context item. */
	void	(*cb_deinit)(void*);	/* Optional callback to deinitialize and free a context item. */
	int		capacity;				/* Total allocated capacity of the context array. */
	int		size;					/* Current number of active items in the list. */
}
StateList;

/**************************************************************************************************
 * StateList Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

void CreateStateList(StateList** state_list, int initial_capacity,
	void* (*cb_init)(void), void (*cb_deinit)(void*));
void* AddStateListItem(StateList* state_list);
void ResizeStateList(StateList* state_list, int new_size, bool do_init);
void FreeStateList(StateList** state_list);

RETRO_END_DECLS

#endif