//-------------------------------------------------------------------------------------------------
// VirtualFamicom.
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

#define FC_SCREEN_WIDTH 256			// Famicom's internal horizontal resolution.
#define FC_SCREEN_HEIGHT 224		// Famicom's internal vertical resolution.
#define FC_SCREEN_ASPECT 1.3333f	// Famicom's display aspect ratio
#define FC_REFRESH_RATE 60			// Famicom's refresh rate (fps).
#define FC_MAX_SPRITES 64			// Famicom's maximum number of sprites.
#define FC_MAX_SYSTEM_COLORS 64		// Famicom's total palatte colors.
#define FC_MAX_TILE_COLORS 4		// Famicom's maximum number of colors per tile.
#define FC_MAX_NAME_LENGTH 22		// Famicom's maximum length of a game's name in selection list.
#define FC_BOOT_FRAME_TIME 256		// Famicom's splash screen display time in frames.

//-------------------------------------------------------------------------------------------------
// Enumerations
//-------------------------------------------------------------------------------------------------

// Famicom system state.
enum FamicomSystemState
{
	Startup,
	GameSelect,
	GameRunning
};

// Famicom layer.
enum FamicomLayer
{
	Foreground,
	Background,
	FC_MAX_LAYERS
};