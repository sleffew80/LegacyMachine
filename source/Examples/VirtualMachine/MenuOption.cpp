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

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "MenuOption.h"

//-------------------------------------------------------------------------------------------------
// MenuOption Constructor(s) / Destructor
//-------------------------------------------------------------------------------------------------

/// <summary>
/// MenuOption constructor.
/// </summary>
MenuOption::MenuOption(void)
{
	m_displayText = "";

	m_rowPosition = 0;
	m_columnPosition = 0;
	m_action = 0;
}

/// <summary>
/// MenuOption constructor.
/// </summary>
/// <param name="displayText">Option's display text to be drawn to screen.</param>
/// <param name="rowPosition">Option's tile row position to be displayed at.</param>
/// <param name="columnPosition">Option's tile column position to be displayed at.</param>
MenuOption::MenuOption(string displayText, int rowPosition, int columnPosition)
{
	m_displayText = displayText;

	m_rowPosition = rowPosition;
	m_columnPosition = columnPosition;
	m_action = 0;
}

/// <summary>
/// MenuOption destructor.
/// </summary>
MenuOption::~MenuOption(void)
{
}

//-------------------------------------------------------------------------------------------------
// MenuOption Accessor Methods
//-------------------------------------------------------------------------------------------------

/// <summary>
/// Gets the option's display text.
/// </summary>
/// <returns>Option's text to be drawn to screen.</returns>
string MenuOption::GetDisplayText(void)
{
	return m_displayText;
}

/// <summary>
/// Sets the option's display text.
/// </summary>
/// <param name="displayText">Option's text to be drawn to screen.</param>
void MenuOption::SetDisplayText(string displayText)
{
	m_displayText = displayText;
}

/// <summary>
/// Gets the option's row position in tile coordinates.
/// </summary>
/// <returns>Option's tile row position to be displayed at.</returns>
int MenuOption::GetRowPosition(void)
{
	return m_rowPosition;
}

/// <summary>
/// Gets the option's column position in tile coordinates.
/// </summary>
/// <returns>Option's tile column position to be displayed at.</returns>
int MenuOption::GetColumnPosition(void)
{
	return m_columnPosition;
}

/// <summary>
/// Gets the option's action id.
/// </summary>
/// <returns>Action ID associated with the option.</returns>
unsigned int MenuOption::GetAction(void)
{
	return m_action;
}

/// <summary>
/// Sets the option's current action id.
/// </summary>
void MenuOption::SetAction(int action)
{
	m_action = action;
}