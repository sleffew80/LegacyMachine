/* Copyright  (C) 2026 Steven Leffew
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (retro_types.h).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _RETRO_TYPES_H
#define _RETRO_TYPES_H

#include <retro_common_api.h>
#include <retro_miscellaneous.h>
#include <streams/interface_stream.h>

/**************************************************************************************************
 * Additional LibRetro Specific Types/Structures
 *************************************************************************************************/

typedef struct retro_sram_block
{
	void* data;
	size_t size;
	unsigned type;
} retro_sram_block_t;

typedef struct retro_serialize_info
{
	const void* data_const;
	void* data;
	size_t size;
} retro_serialize_info_t;

typedef struct retro_save_state_task
{
	intfstream_t* file;
	void* data;
	ssize_t size;
	ssize_t written;
	ssize_t read;
	int slot;
	char path[PATH_MAX_LENGTH];
	bool compress;
	bool mute;
}
retro_save_state_task_t;

#endif