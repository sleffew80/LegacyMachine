//-------------------------------------------------------------------------------------------------
// VirtualMachine - A LegacyMachine implementation using Tilengine as the frontend.
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include <unordered_map>

#include <LegacyMachine.h>

//-------------------------------------------------------------------------------------------------
// Using-Declarations
//-------------------------------------------------------------------------------------------------
using std::string;
using std::vector;
using std::size_t;
using std::unordered_map;

//-------------------------------------------------------------------------------------------------
// Prototypes
//-------------------------------------------------------------------------------------------------
extern "C" {
	// Auto-configuration callback used by LegacyMachine for automated joypad configuration.
	void JoypadAutoConfig(LMC_Player player, char* name, uint32_t vendor, uint32_t product);
}

//-------------------------------------------------------------------------------------------------
// Structures
//-------------------------------------------------------------------------------------------------

enum InputType
{
	INPUT_TYPE_NONE = 0,
	INPUT_TYPE_BUTTON,
	INPUT_TYPE_HAT,
	INPUT_TYPE_AXIS
};

struct InputDescription
{	
	InputType	type;
	int			input;
	int			index;		// Index of hat/axis if applicable.
};

struct JoypadKey
{
	uint32_t vendor;
	uint32_t product;
	string   name;

	bool operator==(const JoypadKey& other) const
	{
		return (vendor == other.vendor && product == other.product && name == other.name);
	}
};

struct JoypadHash
{
	size_t operator()(const JoypadKey& key) const;
};

struct JoypadConfiguration
{
	InputDescription	input_up;
	InputDescription	input_down;
	InputDescription	input_left;
	InputDescription	input_right;
	InputDescription	input_select;
	InputDescription	input_start;
	InputDescription	input_a;
	InputDescription	input_b;
	InputDescription	input_x;
	InputDescription	input_y;
	InputDescription	input_l;
	InputDescription	input_r;
	InputDescription	input_l2;
	InputDescription	input_r2;
	InputDescription	input_l3;
	InputDescription	input_r3;
	InputDescription	input_home;
	InputDescription	input_l_analog_up;
	InputDescription	input_l_analog_down;
	InputDescription	input_l_analog_left;
	InputDescription	input_l_analog_right;
	InputDescription	input_r_analog_up;
	InputDescription	input_r_analog_down;
	InputDescription	input_r_analog_left;
	InputDescription	input_r_analog_right;
};

//-------------------------------------------------------------------------------------------------
// JoypadManager Class
//-------------------------------------------------------------------------------------------------

class JoypadManager
{
private:
	static JoypadManager* m_pJoypadManager;
	unordered_map<JoypadKey, JoypadConfiguration, JoypadHash> m_joypadDB;
	unsigned m_totalJoypads;
	bool m_initialized;

public:
	// Constructor(s) / Destructor

	JoypadManager(void);
	~JoypadManager(void);

	// General Methods

	static JoypadManager* GetJoypadManager(void) {
		if (m_pJoypadManager == NULL)
			m_pJoypadManager = new JoypadManager();
		return m_pJoypadManager;
	};

	bool IntializeJoypadDatabase(const char* fileName);

	// Accessor Methods

	JoypadConfiguration* GetJoypadConfiguration(uint32_t vendor, uint32_t product, string name);
	unsigned GetTotalJoypads(void) { return m_totalJoypads; }
	bool IsInitialized(void) { return m_initialized; }
};
