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
// Default Joypad Mappings
//-------------------------------------------------------------------------------------------------

static const char* s_JoypadDefaultMappings[] = {
	"// Joypad Button Mappings",
	"// Version 1.0",
	"Vendor ID,Product ID,Device Name,Input Up,Input Down,Input Left,Input Right,Input Select,Input Start,Input A,Input B,Input X,Input Y,Input L,Input R,Input L2,Input R2,Input L3,Input R3,Input Home,Input L Analog Up,Input L Analog Down,Input L Analog Left,Input L Analog Right,Input R Analog Up,Input R Analog Down,Input R Analog Left,Input R Analog Right",
	#if defined(_WIN32)
	"1133,49686,Logitech Dual Action,h0up,h0dn,h0lt,h0rt,b8,b9,b2,b1,b3,b0,b4,b5,b6,b7,b10,b11,null,a1-,a1+,a0-,a0+,a3-,a3+,a2-,a2+",
	"1133,49694,Logitech F510 Gamepad,h0up,h0dn,h0lt,h0rt,b8,b9,b2,b1,b3,b0,b4,b5,b6,b7,b10,b11,null,a1-,a1+,a0-,a0+,a3-,a3+,a2-,a2+",
	"1133,49695,Logitech F710 Gamepad,h0up,h0dn,h0lt,h0rt,b8,b9,b2,b1,b3,b0,b4,b5,b6,b7,b10,b11,null,a1-,a1+,a0-,a0+,a3-,a3+,a2-,a2+",
	"9414,21258,Controller (Xbox 360 Pro Ex),h0up,h0dn,h0lt,h0rt,b6,b7,b1,b0,b3,b2,b4,b5,a4+,a5+,b8,b9,b10,a1-,a1+,a0-,a0+,a3-,a3+,a2-,a2+",
	#endif
	#if defined(__APPLE__) && defined(__MACH__)
	"1133,49686,Logitech Dual Action,h0up,h0dn,h0lt,h0rt,b8,b9,b2,b1,b3,b0,b4,b5,b6,b7,b10,b11,null,a1-,a1+,a0-,a0+,a3-,a3+,a2-,a2+",
	#endif
	#if defined(__linux__) || (defined(BSD) && !defined(__MACH__))
	"1133,49686,Logitech Dual Action,h0up,h0dn,h0lt,h0rt,b8,b9,b2,b1,b3,b0,b4,b5,b6,b7,b10,b11,null,a1-,a1+,a0-,a0+,a3-,a3+,a2-,a2+",
	#endif
};

