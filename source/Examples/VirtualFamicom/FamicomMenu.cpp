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
#include <cstring>

#include "FamicomMenu.h"

//-------------------------------------------------------------------------------------------------
// Constructor(s)\Destructor
//-------------------------------------------------------------------------------------------------

FamicomMenu::FamicomMenu(void)
{
	m_activeMenuOption = 0;
	m_totalMenuOptions = 0;
}

FamicomMenu::~FamicomMenu(void)
{

}

//-------------------------------------------------------------------------------------------------
// Methods
//-------------------------------------------------------------------------------------------------

// Iterate through menu options list and print them to screen while also updating the currently 
// selected option.
void FamicomMenu::Update()
{
	for (unsigned int i = 0; i < m_menuOptions.size(); i++)
	{
		char* displayText = new char[m_menuOptions[i]->GetDisplayText().length() + 1];
		strcpy(displayText, m_menuOptions[i]->GetDisplayText().c_str());

		PrintText(Background, m_menuOptions[i]->GetRowPosition(), m_menuOptions[i]->GetColumnPosition(), displayText);

		delete[] displayText;
	}
}

// Update the currently selected option base on user input.
void FamicomMenu::UpdateActiveOption()
{
	if (LMC_GetInput(LMC_INPUT_UP))
	{
		if (m_activeMenuOption - 1 < 0)
			m_activeMenuOption = m_totalMenuOptions - 1;
		else
			--m_activeMenuOption;
	}
	else if (LMC_GetInput(LMC_INPUT_DOWN))
	{
		if (m_activeMenuOption + 1 >= m_totalMenuOptions)
			m_activeMenuOption = 0;
		else
			++m_activeMenuOption;
	}
	else if (LMC_GetInput(LMC_INPUT_SELECT))
	{
		if (m_activeMenuOption + 1 >= m_totalMenuOptions)
			m_activeMenuOption = 0;
		else
			++m_activeMenuOption;
	}
}