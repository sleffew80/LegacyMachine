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
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>

#include "FamicomScreen.h"
#include "FamicomGameIcon.h"

//-------------------------------------------------------------------------------------------------
// Using-Declarations
//-------------------------------------------------------------------------------------------------
using std::vector;
using std::string;

//-------------------------------------------------------------------------------------------------
// FamicomGameSelect Class
//-------------------------------------------------------------------------------------------------

// A class for managing Famicom/NES themed Tilengine based game selection menus.
class FamicomGameSelect : public FamicomScreen
{
private:
	// Member Variables

	vector<ContentOption*> m_games;
	string m_clearLine;
	int m_activeMenuOption;
	int m_totalMenuOptions;
	int m_pageNumber;
	int m_optionsPerPage;
	int m_rowPosition;
	int m_columnPosition;
	int m_lastInput;
	bool m_gameRunning;

	FamicomGameIcon* m_zapperIcon;
	FamicomGameIcon* m_zapperIconSelected;
	FamicomGameIcon* m_onePlayerIcon;
	FamicomGameIcon* m_twoPlayerIcon;
	FamicomGameIcon* m_clearIcon;

public:
	// Constructor(s) / Destructor

	FamicomGameSelect(int optionsPerPage, int rowPosition, int columnPosition);
	~FamicomGameSelect(void);

	// General Methods

	bool IntializeGameList(const char* fileName);

	void ClearText(void);
	void ClearIcon(void);

	void DrawIcon(FamicomLayer tileLayer, FamicomGameIcon* gameIcon, int row, int column);

	void LoadGame(void);

	void Update(void);

	void UpdateActiveOption(void);

	bool IsGameRunning(void);
};