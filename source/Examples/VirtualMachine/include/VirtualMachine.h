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

#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <stdio.h>

#include <LegacyMachine.h>
#include <Tilengine.h>

#include "IMachine.h"
#include "../MenuOption.h"
#include "../ContentOption.h"

//-------------------------------------------------------------------------------------------------
// VirtualMachine Class
//-------------------------------------------------------------------------------------------------

/// <summary>
/// A class for managing and running a user interface rendered with Tilengine for libretro cores 
/// and content using LegacyMachine.
/// </summary>
class VirtualMachine
{
private:
	// Constructor

	VirtualMachine(void);

	// Member Variables

	static VirtualMachine* m_pVirtualMachine;
	LMC_Engine m_legacyMachine;
	int m_currentFrame;

public:
	// Machine Interface

	IMachine* m_machine;

	// Destructor

	virtual ~VirtualMachine(void);

	// General Methods

	static VirtualMachine* GetMachine(void) {
		if (m_pVirtualMachine == NULL)
			m_pVirtualMachine = new VirtualMachine();
		return m_pVirtualMachine;
	};
	bool Initialize(char* appName,
		char* appTitle,
		int baseWidth,
		int baseHeight,
		int maxWidth,
		int maxHeight,
		float aspect,
		double fps,
		int maxLayers,
		int maxSprites,
		int maxAnims,
		int cwfFlag);
	void Run(void);

	// Helper Methods

	void ErrorQuit(const char* errorMessage, ...);
	void LoadSoftware(const char* coreFilename, const char* softwareFilename);
	void UnloadSoftware(void);
};