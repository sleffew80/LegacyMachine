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

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Utilities.h"
#include "JoypadDefaultMappings.h"
#include "JoypadManager.h"

//-------------------------------------------------------------------------------------------------
// Definitions
//-------------------------------------------------------------------------------------------------

#define REQUIRED_COLUMNS 28

//-------------------------------------------------------------------------------------------------
// Using Declarations
//-------------------------------------------------------------------------------------------------

using std::ofstream;
using std::ifstream;
using std::istringstream;
using std::getline;
using std::to_string;

//-------------------------------------------------------------------------------------------------
// JoypadKey Hash Specialization
//-------------------------------------------------------------------------------------------------

size_t JoypadHash::operator()(const JoypadKey& key) const
{
	size_t h1 = std::hash<uint32_t>()(key.vendor);
	size_t h2 = std::hash<uint32_t>()(key.product);
	size_t h3 = std::hash<string>()(key.name);
	// simple combine
	return h1 ^ (h2 << 1) ^ (h3 << 2);
}

//-------------------------------------------------------------------------------------------------
// Static Variable Initialization
//-------------------------------------------------------------------------------------------------
JoypadManager* JoypadManager::m_pJoypadManager = NULL;

//-------------------------------------------------------------------------------------------------
// Local Functions
//-------------------------------------------------------------------------------------------------

// Parse an input description entry from the joypad database.
static InputDescription ParseInputDescription(string description, string joypadName, int VID, int PID)
{
	InputDescription inputDesc;

	// Default to none (invalid/no mapping).
	inputDesc.type = INPUT_TYPE_NONE;
	inputDesc.input = 0;
	inputDesc.index = 0;

	// Normalize the string.
	description = ToLower(TrimString(description));
	if (description.empty())
	{
		LMC_Log(LMC_LOG_VERBOSE, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Empty description", joypadName.c_str(), VID, PID);
		return inputDesc;
	}

	// NULL/NONE
	if (description == "null" || description == "none")
	{
		// Input explicitly has no mapping.
		return inputDesc;
	}

	// BUTTON: bN  (b0..b255)
	if (description[0] == 'b')
	{
		string numberString = description.substr(1);
		if (numberString.empty())
		{
			LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Empty button index '%s'", joypadName.c_str(), VID, PID, description.c_str());
			return inputDesc;
		}
		for (char c : numberString)
		{
			if (!isdigit((unsigned char)c))
			{
				LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Non-digit in button index '%s'", joypadName.c_str(), VID, PID, description.c_str());
				return inputDesc;
			}
		}
		try
		{
			int buttonValue = stoi(numberString);
			if (buttonValue < 0 || buttonValue > 255)
			{
				LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Button out of range '%s'", joypadName.c_str(), VID, PID, description.c_str());
				return inputDesc;
			}
			inputDesc.type = INPUT_TYPE_BUTTON;
			inputDesc.input = buttonValue;
			inputDesc.index = 0;
			return inputDesc;
		}
		catch (...)
		{
			LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Failed to parse button '%s'", joypadName.c_str(), VID, PID, description.c_str());
			return inputDesc;
		}
	}

	// HAT: h<index><dir> where dir is up,dn,lt,rt
	if (description[0] == 'h')
	{
		size_t i = 1;
		while (i < description.size() && isdigit((unsigned char)description[i])) ++i;
		if (i == 1 || i >= description.size())
		{
			LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Invalid hat format '%s'", joypadName.c_str(), VID, PID, description.c_str());
			return inputDesc;
		}

		string indexString = description.substr(1, i - 1);
		string directionString = description.substr(i);

		for (char c : indexString)
			if (!isdigit((unsigned char)c))
			{
				LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Non-digit in hat index '%s'", joypadName.c_str(), VID, PID, description.c_str());
				return inputDesc;
			}

		int hatIndex = 0;
		try { hatIndex = stoi(indexString); }
		catch (...) { LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Hat stoi failed '%s'", joypadName.c_str(), VID, PID, description.c_str()); return inputDesc; }

		if (directionString == "up")
			inputDesc.input = (int)LMC_HAT_UP;
		else if (directionString == "dn")
			inputDesc.input = (int)LMC_HAT_DOWN;
		else if (directionString == "lt")
			inputDesc.input = (int)LMC_HAT_LEFT;
		else if (directionString == "rt")
			inputDesc.input = (int)LMC_HAT_RIGHT;
		else
		{
			LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Unknown hat direction '%s'", joypadName.c_str(), VID, PID, description.c_str());
			return inputDesc;
		}

		inputDesc.type = INPUT_TYPE_HAT;
		inputDesc.index = hatIndex;
		return inputDesc;
	}

	// AXIS: a<index><+|-> e.g. a0+ or a1-
	if (description[0] == 'a')
	{
		size_t i = 1;
		while (i < description.size() && isdigit((unsigned char)description[i])) ++i;
		if (i == 1 || i >= description.size())
		{
			LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Invalid axis format '%s'", joypadName.c_str(), VID, PID, description.c_str());
			return inputDesc;
		}

		string indexString = description.substr(1, i - 1);
		string signString = description.substr(i);

		for (char c : indexString)
			if (!isdigit((unsigned char)c))
			{
				LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Non-digit in axis index '%s'", joypadName.c_str(), VID, PID, description.c_str());
				return inputDesc;
			}

		int analogIndex = 0;
		try { analogIndex = stoi(indexString); }
		catch (...) { LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Axis stoi failed '%s'", joypadName.c_str(), VID, PID, description.c_str()); return inputDesc; }

		if (signString == "+")
			inputDesc.input = (int)LMC_AXIS_POS;
		else if (signString == "-")
			inputDesc.input = (int)LMC_AXIS_NEG;
		else
		{
			LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Unknown axis sign '%s'", joypadName.c_str(), VID, PID, description.c_str());
			return inputDesc;
		}

		inputDesc.type = INPUT_TYPE_AXIS;
		inputDesc.index = analogIndex;
		return inputDesc;
	}

	// Unknown descriptor.
	LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration] [%s (VID: %i, PID: %i)]: Unknown descriptor '%s'", joypadName.c_str(), VID, PID, description.c_str());
	return inputDesc;
}

// Create a new joypad database file with default button mappings.
static bool WriteJoypadDatabaseFile(const char* fileName)
{
	if (!fileName)
		return false;

	// Build full path inside settings directory.
	string filePath = string(LMC_GetPath(LMC_PATH_SETTINGS)) + "/" + fileName;

	// Create/Open file for writing.
	ofstream out(filePath.c_str(), std::ios::out | std::ios::trunc);
	if (!out)
	{
		return false;
	}

	// Determine number of default mapping lines from the static array in the header.
	size_t count = sizeof(s_JoypadDefaultMappings) / sizeof(s_JoypadDefaultMappings[0]);

	for (size_t i = 0; i < count; ++i)
	{
		// Output line with return.
		out << s_JoypadDefaultMappings[i];
		out << "\n";
	}

	out.close();

	if (out.fail())
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
// VirtualMachine Constructor(s) / Destructor
//-------------------------------------------------------------------------------------------------

/// <summary>
/// VirtualMachine constructor.
/// </summary>
JoypadManager::JoypadManager(void)
{
	m_totalJoypads = 0;
	m_initialized = false;
	m_joypadDB.clear();
}

/// <summary>
/// VirtualMachine destructor.
/// </summary>
JoypadManager::~JoypadManager(void)
{
	m_joypadDB.clear();
}

//-------------------------------------------------------------------------------------------------
// General Methods
//-------------------------------------------------------------------------------------------------

/// <summary>
/// Initializes the Joypad Manager's internal joypad database from file.
/// </summary>
/// <param name="fileName">Joypad database file name.</param>
/// <returns>True if successful or false if an error occured.</returns>
bool JoypadManager::IntializeJoypadDatabase(const char* fileName)
{
	string filePath = string(LMC_GetPath(LMC_PATH_SETTINGS)) + "/" + fileName;

	ifstream joypadListFile(filePath.c_str());

	if (!joypadListFile)
	{
		// If the file doesn't exist, create a new one.
		LMC_Log(LMC_LOG_VERBOSE, "VirtualMachine: [Input Configuration]: Joypad database file not found. Creating new file: '%s'", filePath.c_str());
		if (!WriteJoypadDatabaseFile(fileName))
		{
			LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration]: Failed to create joypad database file: '%s'", filePath.c_str());
			return false;
		}

		// Attempt to open the newly created file.
		joypadListFile.clear();
		joypadListFile.open(filePath.c_str());
		if (!joypadListFile)
		{
			LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration]: Failed to open joypad database file: '%s'", filePath.c_str());
			return false;
		}

	}

	string line;
	int lineNumber = 0;
	bool loaded = false;

	while (getline(joypadListFile, line))
	{
		lineNumber++;

		// Trim and skip blank lines and comments.
		string trimmedLine = TrimString(line);
		if (trimmedLine.empty())
			continue;
		if (trimmedLine.size() >= 1 && trimmedLine[0] == '#')
			continue;
		if (trimmedLine.size() >= 2 && trimmedLine[0] == '/' && trimmedLine[1] == '/')
			continue;

		// Split CSV line.
		istringstream stringStream(line);
		string subString;
		vector<string> subStrings;

		while (getline(stringStream, subString, ','))
		{
			subStrings.push_back(TrimString(subString));
		}

		// Basic column validation.
		if (subStrings.size() < REQUIRED_COLUMNS)
		{
			LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration]: Not enough columns (%zu), expected at least %u. file: %s, line: %d",
				subStrings.size(), (unsigned)REQUIRED_COLUMNS, filePath.c_str(), lineNumber);
			continue;
		}

		// Skip header row if present.
		if (!subStrings[0].empty() && ToLower(TrimString(subStrings[0])) == "vendor id")
			continue;

		// Parse vendor/product/device safely.
		int32_t joypadVendor = 0;
		int32_t joypadProduct = 0;
		try
		{
			joypadVendor = stoi(subStrings[0]);
			joypadProduct = stoi(subStrings[1]);
		}
		catch (...)
		{
			LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration]: Invalid vendor/product '%s'/'%s' in file: %s, line: %d",
				subStrings[0].c_str(), subStrings[1].c_str(), filePath.c_str(), lineNumber);
			continue;
		}

		string joypadName = subStrings[2];

		// Parse all input fields.
		InputDescription inputUp = ParseInputDescription(subStrings[3], joypadName, joypadVendor, joypadProduct);
		InputDescription inputDown = ParseInputDescription(subStrings[4], joypadName, joypadVendor, joypadProduct);
		InputDescription inputLeft = ParseInputDescription(subStrings[5], joypadName, joypadVendor, joypadProduct);
		InputDescription inputRight = ParseInputDescription(subStrings[6], joypadName, joypadVendor, joypadProduct);
		InputDescription inputSelect = ParseInputDescription(subStrings[7], joypadName, joypadVendor, joypadProduct);
		InputDescription inputStart = ParseInputDescription(subStrings[8], joypadName, joypadVendor, joypadProduct);
		InputDescription inputA = ParseInputDescription(subStrings[9], joypadName, joypadVendor, joypadProduct);
		InputDescription inputB = ParseInputDescription(subStrings[10], joypadName, joypadVendor, joypadProduct);
		InputDescription inputX = ParseInputDescription(subStrings[11], joypadName, joypadVendor, joypadProduct);
		InputDescription inputY = ParseInputDescription(subStrings[12], joypadName, joypadVendor, joypadProduct);
		InputDescription inputL = ParseInputDescription(subStrings[13], joypadName, joypadVendor, joypadProduct);
		InputDescription inputR = ParseInputDescription(subStrings[14], joypadName, joypadVendor, joypadProduct);
		InputDescription inputL2 = ParseInputDescription(subStrings[15], joypadName, joypadVendor, joypadProduct);
		InputDescription inputR2 = ParseInputDescription(subStrings[16], joypadName, joypadVendor, joypadProduct);
		InputDescription inputL3 = ParseInputDescription(subStrings[17], joypadName, joypadVendor, joypadProduct);
		InputDescription inputR3 = ParseInputDescription(subStrings[18], joypadName, joypadVendor, joypadProduct);
		InputDescription inputHome = ParseInputDescription(subStrings[19], joypadName, joypadVendor, joypadProduct);
		InputDescription inputLAnalogUp = ParseInputDescription(subStrings[20], joypadName, joypadVendor, joypadProduct);
		InputDescription inputLAnalogDown = ParseInputDescription(subStrings[21], joypadName, joypadVendor, joypadProduct);
		InputDescription inputLAnalogLeft = ParseInputDescription(subStrings[22], joypadName, joypadVendor, joypadProduct);
		InputDescription inputLAnalogRight = ParseInputDescription(subStrings[23], joypadName, joypadVendor, joypadProduct);
		InputDescription inputRAnalogUp = ParseInputDescription(subStrings[24], joypadName, joypadVendor, joypadProduct);
		InputDescription inputRAnalogDown = ParseInputDescription(subStrings[25], joypadName, joypadVendor, joypadProduct);
		InputDescription inputRAnalogLeft = ParseInputDescription(subStrings[26], joypadName, joypadVendor, joypadProduct);
		InputDescription inputRAnalogRight = ParseInputDescription(subStrings[27], joypadName, joypadVendor, joypadProduct);
	
		// Add joypad configuration to database.
		JoypadConfiguration joypadConfig {  inputUp, inputDown, inputLeft, inputRight,
											inputSelect, inputStart, inputA, inputB,
											inputX, inputY, inputL, inputR,
											inputL2, inputR2, inputL3, inputR3,
											inputHome, inputLAnalogUp, inputLAnalogDown,
											inputLAnalogLeft, inputLAnalogRight,
											inputRAnalogUp, inputRAnalogDown,
											inputRAnalogLeft, inputRAnalogRight };


		m_joypadDB[JoypadKey{ (uint32_t)joypadVendor, (uint32_t)joypadProduct, joypadName }] = joypadConfig;

		loaded = true;
	}

	m_totalJoypads = (unsigned)m_joypadDB.size();

	if (!loaded)
	{
		LMC_Log(LMC_LOG_ERRORS, "VirtualMachine: [Input Configuration]: No valid joypad configurations loaded from file: '%s'", filePath.c_str());
		return false;
	}

	m_initialized = true;

	return m_initialized;
}

//-------------------------------------------------------------------------------------------------
// Accessor Methods
//-------------------------------------------------------------------------------------------------

/// <summary>
/// Get an entry from the joypad database based on the supplied parameters.
/// </summary>
/// <param name="vendor">Joypad's vendor id number.</param>
/// <param name="product">Joypad's product id number.</param>
/// <param name="product">Joypad's internal name.</param>
/// <returns>Returns a Joypad Configuration if an entry exists within the joypad database.</returns>
JoypadConfiguration* JoypadManager::GetJoypadConfiguration(uint32_t vendor, uint32_t product, string name)
{
	JoypadKey key{vendor, product, name};
	auto joypadSearch = m_joypadDB.find(key);
	if (joypadSearch != m_joypadDB.end())
		return &joypadSearch->second;

	return nullptr;
}