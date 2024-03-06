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
#include <IMachine.h>
#include <VirtualMachine.h>

#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>

#include "FamicomPalette.h"
#include "FamicomScreen.h"
#include "FamicomGameSelect.h"

//-------------------------------------------------------------------------------------------------
// FamicomMachine Class
//-------------------------------------------------------------------------------------------------

// Famicom Virtual Machine interface class.
class FamicomMachine : public IMachine
{
private:
	// Member Variables

	FamicomSystemState m_systemState;
	FamicomPalette* m_pSystemPalette;
	FamicomScreen* m_pStartupScreen;
	FamicomGameSelect* m_pGameSelect;
	int m_currentFrame;

public:
	// Destructor

	~FamicomMachine();

	// Interface Methods

	bool Initialize(int cwfFlag);
	void Start();
	void End();
	void Resume();
	void Pause();
	void Update();

	// General Methods

	bool SetLayerTilemap(FamicomLayer tileLayer, TLN_Tilemap tilemap);
	void ErrorQuit(const char* errorMessage);
};