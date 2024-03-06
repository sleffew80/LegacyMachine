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
// Includes
//-------------------------------------------------------------------------------------------------
#include <VirtualMachine.h>

#include "Common.h"

//-------------------------------------------------------------------------------------------------
// BasicScreen Class
//-------------------------------------------------------------------------------------------------

// A generic class for managing Tilengine based screens.
class BasicScreen
{
protected:
	TLN_Tilemap m_tilemaps[CONSOLE_MAX_LAYERS];
public:
	BasicScreen(void);
	~BasicScreen(void);

	virtual void Update();

	void SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b);
	TLN_Tilemap GetLayerTilemap(BasicLayer tileLayer);
	void LoadTilemap(const char* filename, BasicLayer tileLayer);
	void PrintText(BasicLayer tileLayer, int row, int column, char* text);
	void PrintText(BasicLayer tileLayer, int row, int column, char* text, int offset);
	void DrawTile(BasicLayer tileLayer, int row, int column, int tileIndex);
};