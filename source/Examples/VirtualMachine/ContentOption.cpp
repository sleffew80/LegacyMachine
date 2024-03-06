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
#include "ContentOption.h"

//-------------------------------------------------------------------------------------------------
// ContentOption Constructor(s) / Destructor
//-------------------------------------------------------------------------------------------------

/// <summary>
/// ContentOption constructor.
/// </summary>
/// <param name="displayText">Option's display text to be drawn to screen.</param>
/// <param name="corePath">Option's path on the filesystem to load a core from.</param>
/// <param name="contentPath">Option's path on the filesystem to load content from.</param>
/// <param name="rowPosition">Option's tile row position to be displayed at.</param>
/// <param name="columnPosition">Option's tile column position to be displayed at.</param>
ContentOption::ContentOption(string displayText, string corePath, string contentPath, int numberPlayers, int rowPosition, int columnPosition)
{
	m_displayText = displayText;
	m_corePath = corePath;
	m_contentPath = contentPath;
	m_numberPlayers = numberPlayers;

	m_rowPosition = rowPosition;
	m_columnPosition = columnPosition;
}

/// <summary>
/// ContentOption destructor.
/// </summary>
ContentOption::~ContentOption(void)
{
}

//-------------------------------------------------------------------------------------------------
// ContentOption Accessor Methods
//-------------------------------------------------------------------------------------------------

/// <summary>
/// Gets options's path to the associated libretro core.
/// </summary>
/// <returns>Path to a libretro core.</returns>
string ContentOption::GetCorePath(void)
{
	return m_corePath;
}

/// <summary>
/// Gets options's path to the associated libretro content file (i.e. *.ROM, *.ISO, etc...).
/// </summary>
/// <returns>Path to a libretro content file.</returns>
string ContentOption::GetContentPath(void)
{
	return m_contentPath;
}

/// <summary>
/// Gets number of players supported by the libretro core's content.
/// </summary>
/// <returns>Content's supported number of players.</returns>
int ContentOption::GetNumberOfPlayers(void)
{
	return m_numberPlayers;
}