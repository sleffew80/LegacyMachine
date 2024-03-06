//-------------------------------------------------------------------------------------------------
// VirtualMachine - A LegacyMachine implementation using Tilengine as the frontend.
//
// Copyright(C) 2024 Steven Leffew
// All rights reserved
//-------------------------------------------------------------------------------------------------
// This program is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <https://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstring>
#include <cstdarg>

#include "VirtualMachine.h"

//-------------------------------------------------------------------------------------------------
// Static Variable Initialization
//-------------------------------------------------------------------------------------------------
VirtualMachine* VirtualMachine::m_pVirtualMachine = NULL;

//-------------------------------------------------------------------------------------------------
// VirtualMachine Constructor(s) / Destructor
//-------------------------------------------------------------------------------------------------

/// <summary>
/// VirtualMachine constructor.
/// </summary>
VirtualMachine::VirtualMachine(void)
{
    m_machine = NULL;
    m_legacyMachine = NULL;
    m_tileEngine = NULL;
    m_currentFrame = 0;
}

/// <summary>
/// VirtualMachine destructor.
/// </summary>
VirtualMachine::~VirtualMachine(void)
{
    exit(EXIT_SUCCESS);
}

//-------------------------------------------------------------------------------------------------
// VirtualMachine General Methods
//-------------------------------------------------------------------------------------------------

/// <summary>
/// Initializes the virtual machine engine.
/// </summary>
/// <param name="appName">Application's internal name.</param>
/// <param name="appTitle">Application's window title.</param>
/// <param name="appIcon">Application's icon.</param>
/// <param name="baseWidth">Frontend's horizontal resolution in pixels.</param>
/// <param name="baseHeight">Frontend's vertical resolution in pixels.</param>
/// <param name="maxWidth">Framebuffer's maximum horizontal resolution in pixels.</param>
/// <param name="maxHeight">Framebuffer's maximum vertical resolution in pixels.</param>
/// <param name="aspect">Frontend's output aspect ratio.</param>
/// <param name="fps">Frontend's refresh rate in frames per second.</param>
/// <param name="maxLayers">Frontend's number of supported layers.</param>
/// <param name="maxSprites">Frontend's number of supported sprites.</param>
/// <param name="maxAnims">Frontend's number of supported palette animation slots.</param>
/// <param name="cwfFlag">Create window flags.</param>
/// <returns>True if successful or false if an error occured.</returns>
bool VirtualMachine::Initialize(char* appName, char* appTitle,
    int baseWidth, int baseHeight, int maxWidth, int maxHeight,
    float aspect, double fps, int maxLayers,
    int maxSprites, int maxAnims, int cwfFlag)
{
    // Set the member variables for the virtual machine.
    m_pVirtualMachine = this;
    m_legacyMachine = LMC_Init(appName, baseWidth, baseHeight, maxWidth, maxHeight, aspect, fps, maxLayers, maxSprites, maxAnims);
    if (!m_legacyMachine)
        return false;
    m_tileEngine = LMC_GetMenuTileEngineContext();

#ifdef _DEBUG
    LMC_SetLogLevel(LMC_LOG_VERBOSE);
    TLN_SetLogLevel(TLN_LOG_VERBOSE);
#endif

    LMC_SetWindowTitle(appTitle);

    if (!LMC_CreateWindow(cwfFlag))
        return false;

    return true;
}

/// <summary>
/// Starts virtual machine engine and updates and draws while the associated window remains active.
/// Ends and deinitalizes the virtual machine engine when window is closed or program is exited.
/// </summary>
void VirtualMachine::Run(void)
{
    m_machine->Start();

    while (LMC_ProcessWindow())
    {
        m_machine->Update();
        LMC_UpdateFrame(m_currentFrame);

        m_currentFrame++;
    }

    m_machine->End();
}

//-------------------------------------------------------------------------------------------------
// VirtualMachine Helper Methods
//-------------------------------------------------------------------------------------------------

/// <summary>
/// Displays an error and exits the program.
/// </summary>
/// <param name="errorMessage">Error message to print to console.</param>
void VirtualMachine::ErrorQuit(const char* errorMessage, ...)
{
    char line[4096];

    va_list ap;
    va_start(ap, errorMessage);
    vsnprintf(line, sizeof(line), errorMessage, ap);
    va_end(ap);

    fputs(line, stderr);
    fputc('\n', stderr);
    fflush(stderr);

    if (m_legacyMachine)
        LMC_Deinit();
    if (m_machine)
        m_machine->End();

    exit(EXIT_FAILURE);
}

/// <summary>
/// Loads and initializes a libretro core to run with a specified content file.
/// </summary>
/// <param name="coreFilename">Path on the filesystem to load a core from.</param>
/// <param name="softwareFilename">Path on the filesystem to load content from.</param>
void VirtualMachine::LoadSoftware(const char* coreFilename, const char* softwareFilename)
{
    if (!LMC_LoadCore(coreFilename))
        ErrorQuit("Failed to load core. ", LMC_GetErrorString(LMC_GetLastError()));
    if (!LMC_LoadContent(softwareFilename))
        ErrorQuit("Failed to load core software content. ", LMC_GetErrorString(LMC_GetLastError()));
}

/// <summary>
/// Unloads content and closes current libretro core.
/// </summary>
/// <param name=""></param>
void VirtualMachine::UnloadSoftware(void)
{
    LMC_CloseCore();
}