// ------------------------------------------------------------------------------------------------ -
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
#include "BasicScreen.h"

//-------------------------------------------------------------------------------------------------
// Constructor(s)\Destructor
//-------------------------------------------------------------------------------------------------

BasicScreen::BasicScreen(void)
{
	for (int i = 0; i < CONSOLE_MAX_LAYERS; i++)
		m_tilemaps[i] = NULL;
}

BasicScreen::~BasicScreen(void)
{
	for (int i = 0; i < CONSOLE_MAX_LAYERS; i++)
		if (m_tilemaps[i])
			TLN_DeleteTilemap(m_tilemaps[i]);
}

//-------------------------------------------------------------------------------------------------
// Methods
//-------------------------------------------------------------------------------------------------

// Virtual update method.
void BasicScreen::Update(void)
{

}

// Sets the background color in Tilengine.
void BasicScreen::SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b)
{
	TLN_SetBGColor(r, g, b);
}

// Gets the TLN_Tilemap of the provided layer.
TLN_Tilemap BasicScreen::GetLayerTilemap(BasicLayer tileLayer)
{
	return m_tilemaps[tileLayer];
}

// Loads a tmx tilemap file and stores it in an array of tilemaps where each index of the array
// serves as a separate layer.
void BasicScreen::LoadTilemap(const char* filename, BasicLayer tileLayer)
{
	m_tilemaps[tileLayer] = TLN_LoadTilemap(filename, NULL);
}

// Prints text using a tiles ASCII position on the provided layer's tilesheet. Text is printed 
// starting at the provided row and column of the provided layer's tilemap.
void BasicScreen::PrintText(BasicLayer tileLayer, int row, int column, char* text)
{
	Tile tile = { 0 };

	while (*text)
	{
		tile.index = *text + 1;
		TLN_SetTilemapTile(m_tilemaps[tileLayer], row, column, &tile);
		text++;
		column++;
	}
}

// Prints text using a tiles ASCII position on the provided layer's tilesheet. The offset is added
// to the ASCII position and is useful for when there is more than one set of fonts in a single 
// tilesheet. Text is printed starting at the provided row and column of the provided layer's tilemap.
void BasicScreen::PrintText(BasicLayer tileLayer, int row, int column, char* text, int offset)
{
	Tile tile = { 0 };

	while (*text)
	{
		tile.index = *text + offset + 1;
		TLN_SetTilemapTile(m_tilemaps[tileLayer], row, column, &tile);
		text++;
		column++;
	}
}

// Draws a tile from the index of a tilesheet to the provided location on the tilemap of the 
// provided layer.
void BasicScreen::DrawTile(BasicLayer tileLayer, int row, int column, int tileSetIndex)
{
	Tile tile = { 0 };

	tile.index = tileSetIndex + 1;
	TLN_SetTilemapTile(m_tilemaps[tileLayer], row, column, &tile);
}

