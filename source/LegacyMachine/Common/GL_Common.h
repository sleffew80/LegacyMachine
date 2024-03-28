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

#ifndef _GL_COMMON_H_
#define _GL_COMMON_H_

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <retro_common_api.h>
#include <glad/glad.h>

/**************************************************************************************************
 * OpenGL Common Types/Structures
 *************************************************************************************************/

/* Structure for storing OpenGL shader info. */
typedef struct GL_ShaderInfo 
{
    GLuint vertex_array;
    GLuint vertex_buffer;
    GLuint program;

    GLint i_position;
    GLint i_coordinate;
    GLint u_texture;
    GLint u_mvp;
}
GL_ShaderInfo;

/* Structure for storing additional video info related specifically to OpenGL video rendering. */
typedef struct GL_VideoInfo
{
    GL_ShaderInfo* shader;
    GLuint texture;
    GLuint framebuffer_id;
    GLuint renderbuffer_id;
}
GL_VideoInfo;

/**************************************************************************************************
 * OpenGL Common Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

GL_VideoInfo* GL_GetVideoInfoContext(void);

RETRO_END_DECLS

#endif