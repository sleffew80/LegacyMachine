//-------------------------------------------------------------------------------------------------
// VirtualConsole.
//
// Copyright(C) 2024-2025 Steven Leffew
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

#include "BasicGameSelect.h"

//-------------------------------------------------------------------------------------------------
// Using-Declarations
//-------------------------------------------------------------------------------------------------
using std::ifstream;
using std::to_string;

//-------------------------------------------------------------------------------------------------
// Helpers
//-------------------------------------------------------------------------------------------------

// Parses a single CSV line into trimmed fields, correctly handling double-quoted fields
// that may contain embedded commas (e.g. "Game (Publisher, Inc.) [Genre].zip").
static vector<string> ParseCSVLine(const string& line)
{
	vector<string> fields;
	string field;
	bool inQuotes = false;

	for (size_t i = 0; i < line.size(); ++i)
	{
		char c = line[i];

		if (c == '"')
		{
			// Two consecutive quotes inside a quoted field represent a literal quote character.
			if (inQuotes && i + 1 < line.size() && line[i + 1] == '"')
			{
				field += '"';
				++i;
			}
			else
			{
				inQuotes = !inQuotes;
			}
		}
		else if (c == ',' && !inQuotes)
		{
			fields.push_back(TrimString(field));
			field.clear();
		}
		else
		{
			field += c;
		}
	}

	fields.push_back(TrimString(field));
	return fields;
}

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
	m_lastInput = LMC_JOYPAD_INPUT_NONE;
}

BasicGameSelect::~BasicGameSelect(void)
{
	for(unsigned int i = 0; i < m_games.size(); i++)
	{
		delete m_games[i];
	}
}

//-------------------------------------------------------------------------------------------------
// Methods
//-------------------------------------------------------------------------------------------------

// Initialize game list from lines in provided csv file located in the 
// settings folder. Format should be: Display Name, LibRetro Core File, 
// LibRetro Content File, Number of Players supported by content.
bool BasicGameSelect::IntializeGameList(const char* fileName)
{
	string filePath = string(LMC_GetPath(LMC_PATH_SETTINGS)) + "/" + fileName;

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
		// Skip blank lines and comment lines (# or //).
		if (line.empty() || line[0] == '#' || line.substr(0, 2) == "//")
			continue;

		string displayText;
		string displayTemp;
		vector<string> subStrings = ParseCSVLine(line);

		// Skip lines that don't have all four required fields, or where the libretro
		// core file field contains no extension (e.g. a column header row).
		if (subStrings.size() < 4 || subStrings[1].find(CORE_EXT) == string::npos)
			continue;

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

	if (LMC_GetJoypadInput(LMC_JOYPAD_INPUT_SELECT) && (m_lastInput != LMC_JOYPAD_INPUT_SELECT))
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

		m_lastInput = LMC_JOYPAD_INPUT_SELECT;
	}
	else if (LMC_GetJoypadInput(LMC_JOYPAD_INPUT_UP) && (m_lastInput != LMC_JOYPAD_INPUT_UP))
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

		m_lastInput = LMC_JOYPAD_INPUT_UP;
	}
	else if (LMC_GetJoypadInput(LMC_JOYPAD_INPUT_DOWN) && (m_lastInput != LMC_JOYPAD_INPUT_DOWN))
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

		m_lastInput = LMC_JOYPAD_INPUT_DOWN;
	}
	else if (LMC_GetJoypadInput(LMC_JOYPAD_INPUT_LEFT) && (m_lastInput != LMC_JOYPAD_INPUT_LEFT))
	{
		if (listStart != 0)
		{
			ClearText();
			--m_pageNumber;
			m_activeMenuOption = m_pageNumber * m_optionsPerPage;
		}

		m_lastInput = LMC_JOYPAD_INPUT_LEFT;
	}
	else if (LMC_GetJoypadInput(LMC_JOYPAD_INPUT_RIGHT) && (m_lastInput != LMC_JOYPAD_INPUT_RIGHT))
	{
		if (!lastPage)
		{
			ClearText();
			++m_pageNumber;
			m_activeMenuOption = m_pageNumber * m_optionsPerPage;
		}

		m_lastInput = LMC_JOYPAD_INPUT_RIGHT;
	}
	else if (!LMC_GetJoypadInput(LMC_JOYPAD_INPUT_SELECT) &&
		!LMC_GetJoypadInput(LMC_JOYPAD_INPUT_UP) &&
		!LMC_GetJoypadInput(LMC_JOYPAD_INPUT_DOWN) &&
		!LMC_GetJoypadInput(LMC_JOYPAD_INPUT_LEFT) &&
		!LMC_GetJoypadInput(LMC_JOYPAD_INPUT_RIGHT))
		m_lastInput = LMC_JOYPAD_INPUT_NONE;

	if (LMC_GetJoypadInput(LMC_JOYPAD_INPUT_START))
	{
		LoadGame();
	}
}

// Is game running. Used to determine whether the tilengine menus should be displayed or
// libretro content.
bool BasicGameSelect::IsGameRunning(void)
{
	m_gameRunning = LMC_IsCoreRunning();

	return m_gameRunning;
}