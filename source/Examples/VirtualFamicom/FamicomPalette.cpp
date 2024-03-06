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

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <LegacyMachine.h>

#include "FamicomPalette.h"

//-------------------------------------------------------------------------------------------------
// Using-Declarations
//-------------------------------------------------------------------------------------------------
using std::string;
using std::ifstream;

//-------------------------------------------------------------------------------------------------
// Constructor(s)\Destructor
//-------------------------------------------------------------------------------------------------

FamicomPalette::FamicomPalette(void)
{

}

FamicomPalette::~FamicomPalette(void)
{

}

//-------------------------------------------------------------------------------------------------
// Methods
//-------------------------------------------------------------------------------------------------

// Open a .pal file from the system folder and store the indexed colors in the system palette 
// array one color value at a time.
bool FamicomPalette::InitializePalette(const char* fileName)
{
	string filePath = string(LMC_GetPath(LMC_SYSTEM_PATH)) + "/" + fileName;

	ifstream paletteFile{ filePath, std::ios::binary };

	if (!paletteFile)
	{
		return false;
	}

	char byteValue = 0;
	int paletteIndex = 0;
	int colorIndex = 0;
	bool newColor = false;

	while (paletteFile)
	{
		if (newColor == false)
			byteValue = paletteFile.get();
		else
			newColor = false;

		if ((paletteIndex < FC_MAX_SYSTEM_COLORS) && (byteValue != -1))
		{
			if (colorIndex == 0)
			{
				m_systemPalette[paletteIndex].r = (uint8_t)byteValue;
				colorIndex++;
			}
			else if (colorIndex == 1)
			{
				m_systemPalette[paletteIndex].g = (uint8_t)byteValue;
				colorIndex++;
			}
			else if (colorIndex == 2)
			{
				m_systemPalette[paletteIndex].b = (uint8_t)byteValue;
				colorIndex++;
			}
			else
			{
				m_systemPalette[paletteIndex].a = 255;
				colorIndex = 0;
				newColor = true;
				paletteIndex++;
			}
		}
	}

	paletteFile.close();

	return true;
}

// Get a palette color from the provided palette index.
Color FamicomPalette::GetPaletteColor(int paletteIndex)
{
	return m_systemPalette[paletteIndex];
}