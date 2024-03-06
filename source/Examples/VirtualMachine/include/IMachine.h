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
// IMachine Interface Class
//-------------------------------------------------------------------------------------------------

/// <summary>
/// IMachine interface class.
/// </summary>
class IMachine
{
public:
	virtual ~IMachine(void) = 0;
	virtual bool Initialize(int cwfFlag) = 0;
	virtual void Start(void) = 0;
	virtual void End(void) = 0;
	virtual void Resume(void) = 0;
	virtual void Pause(void) = 0;
	virtual void Update(void) = 0;
};

inline IMachine::~IMachine(void) { }