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
#include <string>

//-------------------------------------------------------------------------------------------------
// Using-Declarations
//-------------------------------------------------------------------------------------------------
using std::string;

//-------------------------------------------------------------------------------------------------
// MenuOption Class
//-------------------------------------------------------------------------------------------------

/// <summary>
/// Simple class for storing basic option information in a simple list menu.
/// </summary>
class MenuOption
{
protected:
	// Member Variables

	string m_displayText;
	int m_rowPosition;
	int m_columnPosition;
	unsigned int m_action;

public:
	// Constructor(s) / Destructor

	MenuOption(void);
	MenuOption(string displayText, int xPosition, int yPosition);
	~MenuOption(void);

	// Accessor Methods

	string GetDisplayText(void);
	void SetDisplayText(string displayText);
	int GetRowPosition(void);
	int GetColumnPosition(void);
	unsigned int GetAction(void);
	void SetAction(int action);
};