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

#ifndef _GL_SHARED_H_
#define _GL_SHARED_H_

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "../../Common/GL_Common.h"
#include "../../SystemManager.h"
#include "../VideoDriver.h"

/**************************************************************************************************
 * OpenGL Shared Video Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

GLuint GL_CompileShader(unsigned type, unsigned count, const char** strings);
bool GL_InitializeShaders(const char* vertex_shader, const char* fragment_shader);
void GL_RefreshVertexData(void);

RETRO_END_DECLS

#endif