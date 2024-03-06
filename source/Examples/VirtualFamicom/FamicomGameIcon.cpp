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
#include "FamicomGameIcon.h"

//-------------------------------------------------------------------------------------------------
// Constructor(s)\Destructor
//-------------------------------------------------------------------------------------------------

FamicomGameIcon::FamicomGameIcon(int tileIndex1, int tileIndex2, int tileIndex3, int tileIndex4, int tileIndex5)
{
	m_iconTiles[0] = tileIndex1;
	m_iconTiles[1] = tileIndex2;
	m_iconTiles[2] = tileIndex3;
	m_iconTiles[3] = tileIndex4;
	m_iconTiles[4] = tileIndex5;
}

FamicomGameIcon::~FamicomGameIcon(void)
{

}

//-------------------------------------------------------------------------------------------------
// Methods
//-------------------------------------------------------------------------------------------------

int FamicomGameIcon::GetTileIndex(int index)
{
	return m_iconTiles[index];
}
