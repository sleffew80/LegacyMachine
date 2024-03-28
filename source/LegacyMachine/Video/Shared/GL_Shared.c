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

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "GL_Shared.h"
#include "../../Logging.h"

/**************************************************************************************************
 * OpenGL Shared Video Functions
 *************************************************************************************************/

GLuint GL_CompileShader(unsigned type, unsigned count, const char** strings)
{
    /* Create shader type (vertex/fragment), set source, and compile. */
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, count, strings, NULL);
    glCompileShader(shader);

    /* Check compiled shader for errors. */
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        char buffer[4096];
        glGetShaderInfoLog(shader, sizeof(buffer), NULL, buffer);
        lmc_trace(LMC_LOG_ERRORS, "[OpenGL]: Failed to compile %s shader: %s", type == GL_VERTEX_SHADER ? "vertex" : "fragment", buffer);
    }

    return shader;
}

bool GL_InitializeShaders(const char* vertex_shader, const char* fragment_shader)
{
    GL_VideoInfo* gl_video = GL_GetVideoInfoContext();
    SystemManager* system = GetSystemManagerContext();

    /* Compile basic shader and generate program. */
    GLuint vshader = GL_CompileShader(GL_VERTEX_SHADER, 1, &vertex_shader);
    GLuint fshader = GL_CompileShader(GL_FRAGMENT_SHADER, 1, &fragment_shader);
    GLuint program = glCreateProgram();

    retro_assert(program);

    /* Attach shaders to program. */
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);

    glDeleteShader(vshader);
    glDeleteShader(fshader);

    glValidateProgram(program);

    /* Check shader program for errors. */
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        char buffer[4096];
        glGetProgramInfoLog(program, sizeof(buffer), NULL, buffer);
        lmc_trace(LMC_LOG_ERRORS, "[OpenGL]: Failed to link shader program: %s", buffer);
        return false;
    }

    /* Initialize OpenGL shader info structure. */
    gl_video->shader->program = program;
    gl_video->shader->i_position = glGetAttribLocation(program, "i_position");
    gl_video->shader->i_coordinate = glGetAttribLocation(program, "i_coordinate");
    gl_video->shader->u_texture = glGetUniformLocation(program, "u_texture");
    gl_video->shader->u_mvp = glGetUniformLocation(program, "u_mvp");

    /* Create VBO. */
    glGenVertexArrays(1, &gl_video->shader->vertex_array);
    glGenBuffers(1, &gl_video->shader->vertex_buffer);

    /* Bind program. */
    glUseProgram(gl_video->shader->program);

    glUniform1i(gl_video->shader->u_texture, 0);

    float m[4][4];
    if (system->cb_hw_render.bottom_left_origin)
        Orthographic2D(m, -1, 1, 1, -1);
    else
        Orthographic2D(m, -1, 1, -1, 1);

    /* Apply orthographic model view projection matrix to shader. */
    glUniformMatrix4fv(gl_video->shader->u_mvp, 1, GL_FALSE, (float*)m);

    /* Unbind program. */
    glUseProgram(0);

    return true;
}

void GL_RefreshVertexData(void)
{
    GL_VideoInfo* gl_video = GL_GetVideoInfoContext();
    VideoInfo* video = GetVideoInfo();

    /* Screen geometry should always be set by this point. */
    retro_assert(video->max_width);
    retro_assert(video->max_height);
    retro_assert(video->frame->width);
    retro_assert(video->frame->height);

    float bottom = (float)video->frame->height / video->max_height;
    float right = (float)video->frame->width / video->max_width;

    float vertex_data[] = {
        // pos, coord
        -1.0f, -1.0f, 0.0f,  bottom, // left-bottom
        -1.0f,  1.0f, 0.0f,  0.0f,   // left-top
         1.0f, -1.0f, right,  bottom,// right-bottom
         1.0f,  1.0f, right,  0.0f,  // right-top
    };

    /* Bind vertex array, creating it if necessary. */
    glBindVertexArray(gl_video->shader->vertex_array);

    glBindBuffer(GL_ARRAY_BUFFER, gl_video->shader->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STREAM_DRAW);

    glEnableVertexAttribArray(gl_video->shader->i_position);
    glEnableVertexAttribArray(gl_video->shader->i_coordinate);
    glVertexAttribPointer(gl_video->shader->i_position, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
    glVertexAttribPointer(gl_video->shader->i_coordinate, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(2 * sizeof(float)));

    /* Unbind vertex array. */
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}