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
#include <VirtualMachine.h>

#include "FamicomPalette.h"

//-------------------------------------------------------------------------------------------------
// FamicomScreen Class
//-------------------------------------------------------------------------------------------------

// A class for managing Famicom/NES themed Tilengine based screens, including palette management.
class FamicomScreen
{
protected:
	TLN_Tilemap m_tilemaps[FC_MAX_LAYERS];
public:
	FamicomScreen(void);
	~FamicomScreen(void);

	virtual void Update();

	void SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b);
	void SetPalette(FamicomLayer tileLayer, FamicomPalette* systemPalette,
		int palette1Color1, int palette1Color2, int palette1Color3,
		int palette2Color1, int palette2Color2, int palette2Color3,
		int palette3Color1, int palette3Color2, int palette3Color3,
		int palette4Color1, int palette4Color2, int palette4Color3);
	void SetPaletteColor(FamicomLayer tileLayer, FamicomPalette* systemPalette, int index, int paletteColor);
	TLN_Tilemap GetLayerTilemap(FamicomLayer tileLayer);
	void LoadTilemap(const char* filename, FamicomLayer tileLayer);
	void PrintText(FamicomLayer tileLayer, int row, int column, char* text);
	void PrintText(FamicomLayer tileLayer, int row, int column, char* text, int offset);
	void DrawTile(FamicomLayer tileLayer, int row, int column, int tileIndex);
};