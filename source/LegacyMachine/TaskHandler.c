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

#include <file/file_path.h>
#include <string/stdstring.h>

#include <retro_types.h>

#include "TaskHandler.h"
#include "MainEngine.h"

/**************************************************************************************************
 * Macro Definitions
 **************************************************************************************************/

/* Size in bytes of each chunk used when reading or writing save state data. */
#define SAVE_STATE_DATA_CHUNK 4096

/**************************************************************************************************
 * TaskHandler Static Functions
 *************************************************************************************************/

/* Finalizes a save state task, closing the file and moving state data to the task for callback 
   delivery. */
static void FinalizeSaveState(retro_task_t* task, retro_save_state_task_t* state)
{
	retro_save_state_task_t* task_state_data = NULL;

	task_set_finished(task, true);

	intfstream_close(state->file);
	free(state->file);

	if (!task_get_error(task) && task_get_cancelled(task))
		task_set_error(task, strdup("Task canceled"));

	/* Allocate and copy state data for delivery to the task callback. */
	task_state_data = (retro_save_state_task_t*)calloc(1, sizeof(retro_save_state_task_t));

	memcpy(task_state_data, state, sizeof(retro_save_state_task_t));

	task_set_data(task, task_state_data);

	if (state->data)
		free(state->data);

	state->data = NULL;

	free(state);
}

/* Finalizes a load state task, closing the file if still open and moving state data to the task
   for callback delivery. */
static void FinalizeLoadState(retro_task_t* task, retro_save_state_task_t* state)
{
	retro_save_state_task_t* task_state_data = NULL;

	task_set_finished(task, true);

	if (state->file)
	{
		intfstream_close(state->file);
		free(state->file);
	}

	if (!task_get_error(task) && task_get_cancelled(task))
		task_set_error(task, strdup("Task canceled"));

	task_state_data = (retro_save_state_task_t*)calloc(1, sizeof(*task_state_data));

	if (!task_state_data)
		return;

	memcpy(task_state_data, state, sizeof(*task_state_data));

	task_set_data(task, task_state_data);

	free(state);
}

/* Incrementally writes serialized core state data to a file, processing one chunk per call. */
static void HandleSaveState(retro_task_t* task)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	retro_save_state_task_t* state = (retro_save_state_task_t*)task->state;
	int data_written;
	ssize_t data_remaining;

	/* On the first call, open the output file using compressed or uncompressed format. */
	if (!state->file)
	{
		if (state->compress)
			state->file = intfstream_open_rzip_file(state->path, RETRO_VFS_FILE_ACCESS_WRITE);
		else
			state->file = intfstream_open_file(state->path,
				RETRO_VFS_FILE_ACCESS_WRITE,
				RETRO_VFS_FILE_ACCESS_HINT_NONE);

		if (!state->file)
			return;
	}

	/* On the first call, serialize and store the full core state for chunked writing. */
	if (!state->data)
	{
		state->size = legacy_machine->system->core->library->retro_serialize_size();
		state->data = calloc(1, state->size);
		legacy_machine->system->core->library->retro_serialize(state->data, state->size);
	}

	data_remaining = MIN(state->size - state->written, SAVE_STATE_DATA_CHUNK);

	if (state->data)
		data_written = (int)intfstream_write(state->file,
			(uint8_t*)state->data + state->written,
			data_remaining);
	else
		data_written = 0;

	state->written += data_written;

	task_set_progress(task, (state->written / (float)state->size) * 100);

	if (task_get_cancelled(task) || data_written != data_remaining)
	{
		/* Write failed or task was cancelled � report error and finalize. */
		char* message = (char*)malloc(sizeof(char*) * (PATH_MAX_LENGTH * 2));
		message[0] = '\0';

		snprintf(message, sizeof(message), "Failed to save state to: %s", state->path);

		task_set_error(task, strdup(message));

		free(message);

		FinalizeSaveState(task, state);

		return;
	}

	if (state->written == state->size)
	{
		/* All data written successfully - set completion title and finalize. */
		if (!task_get_mute(task))
		{
			char* message = (char*)malloc(sizeof(char*) * 128);
			message[0] = '\0';

			task_free_title(task);

			snprintf(message, sizeof(message), "Saved state to slot #%d.", state->slot);

			if (message)
			{
				task_set_title(task, strdup(message));
			}

			free(message);
		}

		FinalizeSaveState(task, state);

		return;
	}
}

/* Incrementally reads saved core state data from a file, processing one chunk per call. */
static void HandleLoadState(retro_task_t* task)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	retro_save_state_task_t* state = (retro_save_state_task_t*)task->state;
	ssize_t data_read;
	ssize_t data_remaining;

	/* On the first call, open the state file and allocate a buffer for the full state data. */
	if (!state->file)
	{
#if defined(HAVE_COMPRESSION) && defined(HAVE_ZLIB)
		/* Will automatically handle uncompressed data too. */
		state->file = intfstream_open_rzip_file(state->path, RETRO_VFS_FILE_ACCESS_READ);
#else
		state->file = intfstream_open_file(state->path,
			RETRO_VFS_FILE_ACCESS_READ,
			RETRO_VFS_FILE_ACCESS_HINT_NONE);
#endif

		if (!state->file)
			FinalizeLoadState(task, state);


		state->size = intfstream_get_size(state->file);

		if (state->size < 0)
			FinalizeLoadState(task, state);

		state->data = malloc(state->size + 1);

		if (!state->data)
			FinalizeLoadState(task, state);
	}

	data_remaining = MIN(state->size - state->read, SAVE_STATE_DATA_CHUNK);
	data_read = intfstream_read(state->file, (uint8_t*)state->data + state->read, data_remaining);

	state->read += data_read;

	if (state->size > 0)
		task_set_progress(task, (state->read / (float)state->size) * 100);

	if (task_get_cancelled(task) || data_read != data_remaining)
	{
		/* Read failed or task was cancelled � report error, free data, and finalize. */
		char* message = (char*)malloc(sizeof(char*) * (PATH_MAX_LENGTH * 2));
		message[0] = '\0';

		snprintf(message, sizeof(message), "Failed to load state from: %s", state->path);

		task_set_error(task, strdup(message));

		free(message);
		free(state->data);
		state->data = NULL;

		FinalizeLoadState(task, state);

		return;
	}

	if (state->read == state->size)
	{
		/* All data read successfully � set completion title and finalize. */
		if (!task_get_mute(task))
		{
			char* message = (char*)malloc(sizeof(char*) * 128);
			message[0] = '\0';

			task_free_title(task);

			snprintf(message, sizeof(message), "Loaded state from slot #%d.", state->slot);

			if (message)
			{
				task_set_title(task, strdup(message));
			}

			free(message);
		}

		FinalizeLoadState(task, state);

		return;
	}
}

/* Frees the task state data after a save state task has completed. */
static void SaveStateCallback(retro_task_t* task, void* task_data, void* user_data, const char* error)
{
	retro_save_state_task_t* state = (retro_save_state_task_t*)task_data;

	free(state);
}

/* Applies loaded state data to the running core after a load state task completes. */
static void LoadStateCallback(retro_task_t* task, void* task_data, void* user_data, const char* error)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	retro_save_state_task_t* state = (retro_save_state_task_t*)task_data;

	if (state->data)
		if (!legacy_machine->system->core->library->retro_unserialize(state->data, state->size))
			lmc_core_log(RETRO_LOG_WARN, "[State]: Failed to load state from slot #%d", state->slot);

	free(state);
}

/**************************************************************************************************
 * TaskHandler Functions
 *************************************************************************************************/

/* Pushes a message to the system message queue with the given priority and duration. */
void PushMessageQueueTask(retro_task_t* task, const char* message, unsigned priority,
	unsigned duration, bool flush)
{
	LMC_Engine legacy_machine = LMC_GetContext();

#if defined HAVE_THREADS
	slock_lock(legacy_machine->system->message_queue_lock);
#endif
	if (flush)
		msg_queue_clear(&legacy_machine->system->message_queue);

	msg_queue_push(&legacy_machine->system->message_queue, message, priority, duration,
		NULL, MESSAGE_QUEUE_ICON_DEFAULT, MESSAGE_QUEUE_CATEGORY_INFO);

	legacy_machine->system->message_queue_size =
		msg_queue_size(&legacy_machine->system->message_queue);

#if defined HAVE_THREADS
	slock_unlock(legacy_machine->system->message_queue_lock);
#endif
}

/* Creates and pushes/enqueues a task to serialize the core's current state to a file. */
void PushSaveStateTask(const char* path, void* data, size_t size, int slot, bool compress)
{
	retro_task_t* task = task_init();
	retro_save_state_task_t* state =
		(retro_save_state_task_t*)calloc(1, sizeof(retro_save_state_task_t));

	strlcpy(state->path, path, PATH_MAX_LENGTH);
	state->data = data;
	state->size = size;
	state->slot = slot;
	state->compress = compress;
	state->mute = false;

	task->type = TASK_TYPE_BLOCKING;
	task->state = state;
	task->handler = HandleSaveState;
	task->callback = SaveStateCallback;
	task->title = strdup("Saving state...");
	task->mute = state->mute;

	if (!task_queue_push(task))
	{
		/* Clean up resources if the task could not be enqueued. */
		if (data)
			free(data);
		if (task->title)
			task_free_title(task);
		free(task);
		free(state);
	}
}

/* Creates and pushes/enqueues a task to load and deserialize a saved state from a file. */
void PushLoadStateTask(const char* path, int slot)
{
	retro_task_t* task = task_init();
	retro_save_state_task_t* state =
		(retro_save_state_task_t*)calloc(1, sizeof(retro_save_state_task_t));

	strlcpy(state->path, path, PATH_MAX_LENGTH);

	state->slot = slot;
	state->compress = false;

	task->type = TASK_TYPE_BLOCKING;
	task->state = state;
	task->handler = HandleLoadState;
	task->callback = LoadStateCallback;
	task->title = strdup("Loading state...");

	if (!task_queue_push(task))
	{
		/* Clean up resources if the task could not be enqueued. */
		if (task->title)
			task_free_title(task);
		free(task);
		free(state);
	}
}
