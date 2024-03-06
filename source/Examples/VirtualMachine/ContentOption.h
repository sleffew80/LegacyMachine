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

#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "MenuOption.h"

//-------------------------------------------------------------------------------------------------
// Using-Declarations
//-------------------------------------------------------------------------------------------------
using std::string;

//-------------------------------------------------------------------------------------------------
// ContentOption Class
//-------------------------------------------------------------------------------------------------

/// <summary>
/// Simple class for storing basic libretro core and content information in a simple list menu.
/// </summary>
class ContentOption : public MenuOption
{
private:
	// Member Variables

	string m_corePath;
	string m_contentPath;
	int m_numberPlayers;

public:
	// Constructor(s) / Destructor

	ContentOption(string displayText, string corePath, string contentPath, int numberPlayers, int rowPosition, int columnPosition);
	~ContentOption(void);

	// Accessor Methods

	string GetCorePath(void);
	string GetContentPath(void);
	int GetNumberOfPlayers(void);
};