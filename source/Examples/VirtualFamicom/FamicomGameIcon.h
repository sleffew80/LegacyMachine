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
#include "Common.h"

//-------------------------------------------------------------------------------------------------
// FamicomGameIcon Class
//-------------------------------------------------------------------------------------------------

// A class for managing Famicom/NES themed Tilengine menu icons. Icons get appended to game names
// in the on screen game list.
class FamicomGameIcon
{
private:
	int m_iconTiles[5];
public:
	FamicomGameIcon(int tileIndex1, int tileIndex2, int tileIndex3, int tileIndex4, int tileIndex5);
	~FamicomGameIcon();

	int GetTileIndex(int index);
};