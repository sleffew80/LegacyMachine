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
// Includes
//-------------------------------------------------------------------------------------------------
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>

#include "Common.h"

//-------------------------------------------------------------------------------------------------
// Color Type
//-------------------------------------------------------------------------------------------------
typedef union
{
	uint32_t value;
	struct
	{
		uint8_t b, g, r, a;
	};
}
Color;

//-------------------------------------------------------------------------------------------------
// FamicomPalette Class
//-------------------------------------------------------------------------------------------------

// A class for managing Famicom/NES themed Tilengine palette. Mimics a real Famicom/NES and uses
// an actual Famicom/NES pal file stored in the "System" directory.
class FamicomPalette
{
private:
	// Member Variables

	Color m_systemPalette[FC_MAX_SYSTEM_COLORS];

public:
	// Constructor(s)/Destructor
	FamicomPalette(void);
	~FamicomPalette(void);

	// General Methods

	bool InitializePalette(const char* fileName);
	Color GetPaletteColor(int paletteIndex);
};