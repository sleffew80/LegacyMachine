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
#include "BasicMachine.h"

//-------------------------------------------------------------------------------------------------
// BasicMachine Destructor
//-------------------------------------------------------------------------------------------------

BasicMachine::~BasicMachine()
{
	End();
}

//-------------------------------------------------------------------------------------------------
// BasicMachine Interface Methods
//-------------------------------------------------------------------------------------------------

// Initialize the virtual machine.
bool BasicMachine::Initialize(int cwfFlag)
{
	VirtualMachine* pVirtualMachine = VirtualMachine::GetMachine();
	if (!pVirtualMachine->Initialize(PROGRAM_NAME,
		"Video Gaming System",
		CONSOLE_SCREEN_WIDTH,
		CONSOLE_SCREEN_HEIGHT,
		CONSOLE_SCREEN_WIDTH,
		CONSOLE_SCREEN_HEIGHT,
		CONSOLE_SCREEN_ASPECT,
		CONSOLE_REFRESH_RATE,
		CONSOLE_MAX_LAYERS,
		0, 0, cwfFlag))
	{
		return false;
	}

	m_systemState = Startup;

	m_currentFrame = 0;

	m_pStartupScreen = new BasicScreen();
	m_pGameSelect = new BasicGameSelect(10, 8, 3);

	return true;
}

// Post initialization setup.
void BasicMachine::Start()
{
	m_pStartupScreen->LoadTilemap("Startup.tmx", Background);
	m_pGameSelect->IntializeGameList("Games.csv");

	m_pGameSelect->LoadTilemap("Menu.tmx", Background);

	if (!SetLayerTilemap(Background, m_pStartupScreen->GetLayerTilemap(Background)))
		ErrorQuit("Tilengine error.");
}

// Deinitialization and shutdown.
void BasicMachine::End()
{
	if (m_pStartupScreen)
		delete m_pStartupScreen;
	if (m_pGameSelect)
		delete m_pGameSelect;

	LMC_Deinit();

	// Cleanup the system engine
	VirtualMachine* pVirtualMachine = VirtualMachine::GetMachine();
	delete pVirtualMachine;
}

void BasicMachine::Resume()
{

}

void BasicMachine::Pause()
{

}

// Update menu and handle input.
void BasicMachine::Update()
{
	switch (m_systemState)
	{
	case Startup:
	{
		if (m_currentFrame > CONSOLE_BOOT_FRAME_TIME)
		{
			if (!SetLayerTilemap(Background, m_pGameSelect->GetLayerTilemap(Background)))
				ErrorQuit("Tilengine error.");
			m_systemState = GameSelect;
		}
		break;
	}
	case GameSelect:
	{
		m_pGameSelect->UpdateActiveOption();
		m_pGameSelect->Update();
		if (m_pGameSelect->IsGameRunning())
			m_systemState = GameRunning;
		break;
	}
	case GameRunning:
	{
		break;
	}

	}
	m_currentFrame++;
}

//-------------------------------------------------------------------------------------------------
// BasicMachine General Methods
//-------------------------------------------------------------------------------------------------

// Sets the current background tilemap.
bool BasicMachine::SetLayerTilemap(BasicLayer tileLayer, TLN_Tilemap tilemap)
{
	return TLN_SetLayerTilemap(tileLayer, tilemap);
}

// Displays an error and exits the program.
void BasicMachine::ErrorQuit(const char* errorMessage)
{
	VirtualMachine* pVirtualMachine = VirtualMachine::GetMachine();
	pVirtualMachine->ErrorQuit(errorMessage);
}
