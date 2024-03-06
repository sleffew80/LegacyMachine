// ------------------------------------------------------------------------------------------------ -
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
#include "FamicomScreen.h"

//-------------------------------------------------------------------------------------------------
// Constructor(s)\Destructor
//-------------------------------------------------------------------------------------------------

FamicomScreen::FamicomScreen(void)
{
	for (int i = 0; i < FC_MAX_LAYERS; i++)
		m_tilemaps[i] = NULL;
}

FamicomScreen::~FamicomScreen(void)
{
	for (int i = 0; i < FC_MAX_LAYERS; i++)
		if (m_tilemaps[i])
			TLN_DeleteTilemap(m_tilemaps[i]);
}

//-------------------------------------------------------------------------------------------------
// Methods
//-------------------------------------------------------------------------------------------------

// Virtual update method.
void FamicomScreen::Update(void)
{

}

// Sets the background color in Tilengine.
void FamicomScreen::SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b)
{
	TLN_SetBGColor(r, g, b);
}

// Sets the current screen color palette in Tilengine from the Famicom/NES themed system palette.
// Mimics the Famicom/NES's 4 palettes of 4 colors per layer where each palette color is an index
// from the system palette created from an nes.pal file.
void FamicomScreen::SetPalette(FamicomLayer tileLayer, FamicomPalette* systemPalette, int palette1Color1, int palette1Color2, int palette1Color3,
	int palette2Color1, int palette2Color2, int palette2Color3,
	int palette3Color1, int palette3Color2, int palette3Color3,
	int palette4Color1, int palette4Color2, int palette4Color3)
{
	int palette[12] = { palette1Color1, palette1Color2, palette1Color3,
		palette2Color1, palette2Color2, palette2Color3,
		palette3Color1, palette3Color2, palette3Color3,
		palette4Color1, palette4Color2, palette4Color3 };

	int paletteSize = sizeof(palette) / sizeof(palette[0]);

	for (int i = 0; i < paletteSize; i++)
	{
		SetPaletteColor(tileLayer, systemPalette, i + 1, palette[i]);
	}

}

// Sets an individual color within a Tilengine color palette from the Famicom/NES themed system palette.
void FamicomScreen::SetPaletteColor(FamicomLayer tileLayer, FamicomPalette* systemPalette, int color, int paletteIndex)
{
	TLN_SetPaletteColor(TLN_GetTilesetPalette(TLN_GetTilemapTileset(m_tilemaps[tileLayer])), color,
		systemPalette->GetPaletteColor(paletteIndex).r,
		systemPalette->GetPaletteColor(paletteIndex).g,
		systemPalette->GetPaletteColor(paletteIndex).b);
}

// Gets the TLN_Tilemap of the provided layer.
TLN_Tilemap FamicomScreen::GetLayerTilemap(FamicomLayer tileLayer)
{
	return m_tilemaps[tileLayer];
}

// Loads a tmx tilemap file and stores it in an array of tilemaps where each index of the array
// serves as a separate layer.
void FamicomScreen::LoadTilemap(const char* filename, FamicomLayer tileLayer)
{
	m_tilemaps[tileLayer] = TLN_LoadTilemap(filename, NULL);
}

// Prints text using a tiles ASCII position on the provided layer's tilesheet. Text is printed 
// starting at the provided row and column of the provided layer's tilemap.
void FamicomScreen::PrintText(FamicomLayer tileLayer, int row, int column, char* text)
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
void FamicomScreen::PrintText(FamicomLayer tileLayer, int row, int column, char* text, int offset)
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
void FamicomScreen::DrawTile(FamicomLayer tileLayer, int row, int column, int tileSetIndex)
{
	Tile tile = { 0 };

	tile.index = tileSetIndex + 1;
	TLN_SetTilemapTile(m_tilemaps[tileLayer], row, column, &tile);
}

