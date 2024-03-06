/*
* LegacyMachine - A libRetro implementation for creating simple lo-fi
* frontends intended to simulate the look and feel of the classic
* video gaming consoles, computers, and arcade machines being emulated.
*
* Copyright (C) 2022-2024 Steven Leffew
* All rights reserved
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
* */

#ifndef _LOGGING_H
#define _LOGGING_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"

/**************************************************************************************************
 * Logging Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

	void lmc_core_log(enum retro_log_level level, const char* format, ...);
	void lmc_trace(LMC_LogLevel log_level, const char* format, ...);

RETRO_END_DECLS

#endif