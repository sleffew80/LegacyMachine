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
#include <regex>

#include "Utilities.h"

//-------------------------------------------------------------------------------------------------
// Using-Declarations
//-------------------------------------------------------------------------------------------------
using std::regex;
using std::regex_replace;
using std::transform;
using std::tolower;
using std::toupper;

//-------------------------------------------------------------------------------------------------
// Common Utility Helper Functions
//-------------------------------------------------------------------------------------------------
string TrimString(string str)
{
	regex whiteSpace("^\\s+|\\s+$");

	return regex_replace(str, whiteSpace, "");
}

string ToUpper(string str)
{
    transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c) { return toupper(c); }
    );

    return str;
}

string ToLower(string str)
{
    transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c) { return tolower(c); }
    );

    return str;
}
