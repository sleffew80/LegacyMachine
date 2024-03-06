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
#include "FamicomMachine.h"

//-------------------------------------------------------------------------------------------------
// FamicomMachine Destructor
//-------------------------------------------------------------------------------------------------

FamicomMachine::~FamicomMachine()
{
	End();
}

//-------------------------------------------------------------------------------------------------
// FamicomMachine Interface Methods
//-------------------------------------------------------------------------------------------------

// Initialize the famicom virtual machine.
bool FamicomMachine::Initialize(int cwfFlag)
{
	VirtualMachine* pVirtualMachine = VirtualMachine::GetMachine();
	if (!pVirtualMachine->Initialize(PROGRAM_NAME,
		"Nintendo Entertainment System",
		FC_SCREEN_WIDTH,
		FC_SCREEN_HEIGHT,
		FC_SCREEN_WIDTH,
		FC_SCREEN_HEIGHT,
		FC_SCREEN_ASPECT,
		FC_REFRESH_RATE,
		FC_MAX_LAYERS,
		0, 0, cwfFlag))
	{
		return false;
	}

	m_systemState = Startup;

	m_currentFrame = 0;

	m_pSystemPalette = new FamicomPalette();
	m_pSystemPalette->InitializePalette("nes.pal");

	m_pStartupScreen = new FamicomScreen();
	m_pGameSelect = new FamicomGameSelect(10, 8, 3);

	return true;
}

// Post initialization setup.
void FamicomMachine::Start()
{
	m_pStartupScreen->LoadTilemap("Startup_0_U.tmx", Background);
	m_pGameSelect->IntializeGameList("Games.csv");

	m_pGameSelect->LoadTilemap("Menu_0.tmx", Background);
	m_pGameSelect->LoadTilemap("Menu_1.tmx", Foreground);

	m_pStartupScreen->SetPalette(Background, m_pSystemPalette, 54, 22, 48, 44, 22, 48, 0, 0, 0, 0, 0, 0);
	m_pGameSelect->SetPalette(Background, m_pSystemPalette, 55, 32, 41, 6, 22, 41, 22, 54, 41, 57, 9, 41);
	m_pGameSelect->SetPalette(Foreground, m_pSystemPalette, 56, 1, 13, 22, 32, 41, 55, 1, 7, 55, 1, 22);

	if (!SetLayerTilemap(Background, m_pStartupScreen->GetLayerTilemap(Background)))
		ErrorQuit("Tilengine error.");
}

// Deinitialization and shutdown.
void FamicomMachine::End()
{
	if (m_pStartupScreen)
		delete m_pStartupScreen;
	if (m_pGameSelect)
		delete m_pGameSelect;
	if (m_pSystemPalette)
		delete m_pSystemPalette;

	LMC_Deinit();

	// Cleanup the system engine
	VirtualMachine* pVirtualMachine = VirtualMachine::GetMachine();
	delete pVirtualMachine;
}

void FamicomMachine::Resume()
{

}

void FamicomMachine::Pause()
{

}

// Update menu and handle input.
void FamicomMachine::Update()
{
	switch (m_systemState)
	{
	case Startup:
	{
		if (m_currentFrame > FC_BOOT_FRAME_TIME)
		{
			if (!SetLayerTilemap(Background, m_pGameSelect->GetLayerTilemap(Background)))
				ErrorQuit("Tilengine error.");
			if (!SetLayerTilemap(Foreground, m_pGameSelect->GetLayerTilemap(Foreground)))
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
// FamicomMachine General Methods
//-------------------------------------------------------------------------------------------------

// Sets the current background tilemap.
bool FamicomMachine::SetLayerTilemap(FamicomLayer tileLayer, TLN_Tilemap tilemap)
{
	return TLN_SetLayerTilemap(tileLayer, tilemap);
}

// Displays an error and exits the program.
void FamicomMachine::ErrorQuit(const char* errorMessage)
{
	VirtualMachine* pVirtualMachine = VirtualMachine::GetMachine();
	pVirtualMachine->ErrorQuit(errorMessage);
}
