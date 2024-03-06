//-------------------------------------------------------------------------------------------------
// VirtualConsole.
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Definitions
//-------------------------------------------------------------------------------------------------

#define CONSOLE_SCREEN_WIDTH 256		// Console's internal horizontal resolution.
#define CONSOLE_SCREEN_HEIGHT 224		// Console's internal vertical resolution.
#define CONSOLE_SCREEN_ASPECT 1.3333f	// Console's display aspect ratio
#define CONSOLE_REFRESH_RATE 60			// Console's refresh rate (fps).
#define CONSOLE_MAX_SPRITES 64			// Console's maximum number of sprites.
#define CONSOLE_MAX_NAME_LENGTH 26		// Console's maximum length of a game's name in selection list.
#define CONSOLE_BOOT_FRAME_TIME 256		// Console's splash screen display time in frames.

//-------------------------------------------------------------------------------------------------
// Enumerations
//-------------------------------------------------------------------------------------------------

// Console system state.
enum BasicSystemState
{
	Startup,
	GameSelect,
	GameRunning
};

// Console layer.
enum BasicLayer
{
	Background,
	CONSOLE_MAX_LAYERS
};