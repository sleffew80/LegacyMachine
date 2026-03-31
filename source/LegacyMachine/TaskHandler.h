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

#ifndef _TASK_HANDLER_H
#define _TASK_HANDLER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <retro_common_api.h>
#include <queues/task_queue.h>

/**************************************************************************************************
 * TaskHandler Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

void PushMessageQueueTask(retro_task_t* task, const char* message,
	unsigned priority, unsigned duration, bool flush);

void PushSaveStateTask(const char* path, void* data, size_t size, int slot, bool compress);
void PushLoadStateTask(const char* path, int slot);

RETRO_END_DECLS

#endif
