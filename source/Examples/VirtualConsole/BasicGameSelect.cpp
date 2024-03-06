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

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstring>
#include <sstream>

#include "BasicGameSelect.h"

//-------------------------------------------------------------------------------------------------
// Using-Declarations
//-------------------------------------------------------------------------------------------------
using std::ifstream;
using std::istringstream;
using std::to_string;

//-------------------------------------------------------------------------------------------------
// Constructor(s)\Destructor
//-------------------------------------------------------------------------------------------------

BasicGameSelect::BasicGameSelect(int optionsPerPage, int rowPosition, int columnPosition)
{
	m_gameRunning = false;
	m_pageNumber = 0;
	m_optionsPerPage = optionsPerPage;
	m_activeMenuOption = 0;
	m_totalMenuOptions = 0;
	m_rowPosition = rowPosition;
	m_columnPosition = columnPosition;
	m_lastInput = LMC_INPUT_NONE;
}

BasicGameSelect::~BasicGameSelect(void)
{

}

//-------------------------------------------------------------------------------------------------
// Methods
//-------------------------------------------------------------------------------------------------

// Initialize game list from lines in provided csv file located in the 
// settings folder. Format should be: Display Name, LibRetro Core File, 
// LibRetro Content File, Number of Players supported by content.
bool BasicGameSelect::IntializeGameList(const char* fileName)
{
	string filePath = string(LMC_GetPath(LMC_SETTING_PATH)) + "/" + fileName;

	ifstream gameListFile(filePath.c_str());

	if (!gameListFile)
	{
		return false;
	}

	string line;
	int displayNumber = 1;
	int tileRow = m_rowPosition;
	int tileColumn = m_columnPosition;
	int maxRow = m_rowPosition + m_optionsPerPage - 1;

	while (getline(gameListFile, line))
	{
		istringstream stringStream(line);

		string displayText;
		string displayTemp;
		string subString;
		vector<string> subStrings;

		while (getline(stringStream, subString, ','))
		{
			subStrings.push_back(subString);
		}

		if (tileRow > maxRow)
			tileRow = m_rowPosition;

		if (displayNumber < 10)
			displayText = " " + to_string(displayNumber) + ":" + subStrings[0];
		else
			displayText = to_string(displayNumber) + ":" + subStrings[0];

		if (displayText.length() > CONSOLE_MAX_NAME_LENGTH)
		{
			displayTemp = displayText;
			displayText = displayTemp.substr(0, CONSOLE_MAX_NAME_LENGTH);
		}

		m_games.push_back(new ContentOption(displayText, subStrings[1], subStrings[2], stoi(subStrings[3]), tileRow, tileColumn));

		displayNumber++;
		tileRow++;
	}

	while (m_clearLine.length() < CONSOLE_MAX_NAME_LENGTH + 1)
	{
		m_clearLine += " ";
	}

	m_totalMenuOptions = m_games.size();
}

// Clears out a line of text during refreshes for updating selected options and page changes.
void BasicGameSelect::ClearText(void)
{
	char* emptyText = new char[m_clearLine.length() + 1];
	strcpy(emptyText, m_clearLine.c_str());
	int row = m_rowPosition;

	for (unsigned int i = 0; i < m_optionsPerPage; i++)
	{
		PrintText(Background, row, m_columnPosition, emptyText);
		row++;
	}

	delete[] emptyText;
}

// Loads and plays libretro content associated with the actively selected menu option.
void BasicGameSelect::LoadGame(void)
{
	VirtualMachine* pVirtualMachine = VirtualMachine::GetMachine();

	char* coreLocation = new char[m_games[m_activeMenuOption]->GetCorePath().length() + 1];
	char* contentLocation = new char[m_games[m_activeMenuOption]->GetContentPath().length() + 1];

	strcpy(coreLocation, m_games[m_activeMenuOption]->GetCorePath().c_str());
	strcpy(contentLocation, m_games[m_activeMenuOption]->GetContentPath().c_str());

	pVirtualMachine->LoadSoftware(coreLocation, contentLocation);
}

// Iterate through games list and print them to screen while also updating the currently 
// selected game.
void BasicGameSelect::Update(void)
{
	unsigned int listStart = m_pageNumber * m_optionsPerPage;
	unsigned int listEnd = listStart + m_optionsPerPage;

	if (listEnd > m_totalMenuOptions)
		listEnd = m_totalMenuOptions;

	for (unsigned int i = listStart; i < listEnd; i++)
	{
		char* displayText = new char[m_games[i]->GetDisplayText().length() + 1];
		strcpy(displayText, m_games[i]->GetDisplayText().c_str());

		if (i == m_activeMenuOption)
		{
			PrintText(Background, m_games[i]->GetRowPosition(), m_games[i]->GetColumnPosition(), displayText, 256);
		}
		else
		{
			PrintText(Background, m_games[i]->GetRowPosition(), m_games[i]->GetColumnPosition(), displayText);
		}

		delete[] displayText;
	}
}

// Update the currently selected game based on user input.
void BasicGameSelect::UpdateActiveOption(void)
{
	unsigned int listStart = m_pageNumber * m_optionsPerPage;
	unsigned int listEnd = listStart + m_optionsPerPage;
	unsigned int lastOption = m_totalMenuOptions - 1;
	bool lastPage = false;

	if (listEnd > m_totalMenuOptions)
		listEnd = m_totalMenuOptions;

	if (listEnd == m_totalMenuOptions)
		lastPage = true;

	if (LMC_GetInput(LMC_INPUT_SELECT) && (m_lastInput != LMC_INPUT_SELECT))
	{
		if (m_activeMenuOption >= lastOption)
			m_activeMenuOption = lastOption;
		else
			++m_activeMenuOption;

		if (m_activeMenuOption >= listEnd)
		{
			ClearText();
			++m_pageNumber;
		}

		m_lastInput = LMC_INPUT_SELECT;
	}
	else if (LMC_GetInput(LMC_INPUT_UP) && (m_lastInput != LMC_INPUT_UP))
	{
		if (m_activeMenuOption - 1 < 0)
			m_activeMenuOption = 0;
		else
			--m_activeMenuOption;

		if (m_activeMenuOption < listStart)
		{
			ClearText();
			--m_pageNumber;
		}

		m_lastInput = LMC_INPUT_UP;
	}
	else if (LMC_GetInput(LMC_INPUT_DOWN) && (m_lastInput != LMC_INPUT_DOWN))
	{
		if (m_activeMenuOption >= lastOption)
			m_activeMenuOption = lastOption;
		else
			++m_activeMenuOption;

		if (m_activeMenuOption >= listEnd)
		{
			ClearText();
			++m_pageNumber;
		}

		m_lastInput = LMC_INPUT_DOWN;
	}
	else if (LMC_GetInput(LMC_INPUT_LEFT) && (m_lastInput != LMC_INPUT_LEFT))
	{
		if (listStart != 0)
		{
			ClearText();
			--m_pageNumber;
			m_activeMenuOption = m_pageNumber * m_optionsPerPage;
		}

		m_lastInput = LMC_INPUT_LEFT;
	}
	else if (LMC_GetInput(LMC_INPUT_RIGHT) && (m_lastInput != LMC_INPUT_RIGHT))
	{
		if (!lastPage)
		{
			ClearText();
			++m_pageNumber;
			m_activeMenuOption = m_pageNumber * m_optionsPerPage;
		}

		m_lastInput = LMC_INPUT_RIGHT;
	}
	else if (!LMC_GetInput(LMC_INPUT_SELECT) &&
		!LMC_GetInput(LMC_INPUT_UP) &&
		!LMC_GetInput(LMC_INPUT_DOWN) &&
		!LMC_GetInput(LMC_INPUT_LEFT) &&
		!LMC_GetInput(LMC_INPUT_RIGHT))
		m_lastInput = LMC_INPUT_NONE;

	if (LMC_GetInput(LMC_INPUT_START))
	{
		m_gameRunning = true;
		LoadGame();
	}
}

// Is game running. Used to determine whether the tilengine menus should be displayed or
// libretro content.
bool BasicGameSelect::IsGameRunning(void)
{
	return m_gameRunning;
}