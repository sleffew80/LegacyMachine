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

#include <vector>

#include "BasicScreen.h"

//-------------------------------------------------------------------------------------------------
// Using-Declarations
//-------------------------------------------------------------------------------------------------
using std::vector;

//-------------------------------------------------------------------------------------------------
// BasicMenu Class
//-------------------------------------------------------------------------------------------------

// A generic class for managing Tilengine based menus.
class BasicMenu : public BasicScreen
{
private:
	vector<MenuOption*> m_menuOptions;

	int m_activeMenuOption;
	int m_totalMenuOptions;
public:

	BasicMenu(void);
	~BasicMenu(void);

	void Update();

	virtual void UpdateActiveOption();
};