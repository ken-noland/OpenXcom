/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "StartState.h"
#include "../../Game/GameContext.h"
#include "../../Game/GameWindow.h"
#include "../../Engine/EngineContext.h"
#include "../../Engine/Graphics/GraphicsSystem.h"
#include "../../Engine/Graphics/GraphicsSurface.h"
#include "../../Engine/Graphics/Common/GameSurface.h"
#include "../../Engine/Graphics/Primitive/PrimitiveFactory.h"
#include "../../Engine/Graphics/Primitive/TextPrimitive.h"
#include "../../Engine/Graphics/Primitive/TextPrimitiveFactory.h"
#include "../../Engine/Graphics/Palette/Palette.h"
#include "../../Engine/Graphics/Palette/PaletteManager.h"
#include "../../Engine/Graphics/Font/Font.h"
#include "../../Engine/Graphics/Font/FontManager.h"
#include "../../Engine/Graphics/Image/Image.h"
#include "../../Engine/Graphics/Image/ImageManager.h"
#include "../../Engine/Resource/ResourceSystem.h"
#include "../../Engine/Resource/FileProcessor/ImageBMPFileProcessor.h"
#include "../../Engine/Resource/FileProcessor/ImageFile.h"

#include <glm/vec4.hpp>
#include <cstring>

// wrap in an anonymous namespace to avoid name conflicts
namespace
{
#include "../../Engine/Graphics/Font/DosFont.h"
}

namespace OpenXcom
{

StartState::StartState(GameContext& game)
	: _game(game)
{
	EngineContext& engine = _game.getEngine();
	GraphicsSystem& graphics = engine.getGraphicsSystem();

	// set the size to 720x400 terminal surface
	_game.getGameWindow().setGameSurfaceSize(glm::ivec2(720, 400));

	// get the game surface
	GameSurface& gameSurface = _game.getGameWindow().getGameSurface();
	// get the primitive factory
	PrimitiveFactory& primitiveFactory = gameSurface.getRenderTarget().getPrimitiveFactory();
		
	createDosFont();

	std::string text = "Hello World!\n\n" 
					   "Lorem ipsum dolor sit amet, consectetur adipiscing elit. \n"
					   "Sed do eiusmod tempor incididunt ut labore et dolore magna \n"
					   "aliqua.";

	// create the text

	TextSettings textSettings;
	textSettings.alignment = TextAlignment::Left;
	textSettings.offset = glm::vec2(0, 0);
	textSettings.extents = glm::vec2(720, 400);
	textSettings.fontHandle = _dosFont.getHandle();
	textSettings.paletteHandle = _dosFontPalette.getHandle();
	textSettings.defaultStyle.backgroundColorIndex = 0;
	textSettings.defaultStyle.colorIndex = 7;
	textSettings.defaultStyle.underline = false;

	_text = primitiveFactory.createTextPrimitive(text, textSettings);
}

/**
 * Kill the thread in case the game is quit early.
 */
StartState::~StartState()
{
	_text.reset();
	_cursor.reset();

	_dosFont.reset();
	_dosFontPalette.reset();
}

// helper function to get the glyph from the font
std::array<OpenXcom::Glyph, 128> getAsciiGlyphs(OpenXcom::ResourceHandle<OpenXcom::DeviceImage> image)
{
	std::array<OpenXcom::Glyph, 128> asciiGlyphs;
	memset(asciiGlyphs.data(), 0, asciiGlyphs.size() * sizeof(OpenXcom::Glyph));

	int charWidth = 9;
	int charHeight = 16;
	int textureWidth = 288;
	int textureHeight = 48;
	int charsPerRow = textureWidth / charWidth;

	std::string characters =
		" !\"#$%&'()*+,-./0123456789:;<=>?"
		"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^"
		"_`abcdefghijklmnopqrstuvwxyz{|}~";

	for (size_t i = 0; i < characters.size(); i++)
	{
		int x = static_cast<int>((i % charsPerRow) * charWidth);
		int y = static_cast<int>((i / charsPerRow) * charHeight);

		assert(characters[i] < 127);

		asciiGlyphs[characters[i]] = {
			static_cast<uint16_t>(x),
			static_cast<uint16_t>(y),
			static_cast<uint16_t>(charWidth),
			static_cast<uint16_t>(charHeight),
			0, 0,
			static_cast<int8_t>(charWidth), // Fixed width spacing
			image};
	}

	return asciiGlyphs;
}

void StartState::createDosFont()
{
	EngineContext& engine = _game.getEngine();

	ResourceSystem& resourceSystem = engine.getResourceSystem();
	ImageManager& imageManager = resourceSystem.getImageManager();
	PaletteManager& paletteManager = resourceSystem.getPaletteManager();
	FontManager& fontManager = resourceSystem.getFontManager();

	// load the dos font palette
	OpenXcom::PackedColor paletteData[] = {
		0x000000FF, // 0: Black       (0,0,0)
		0xAA0000FF, // 1: Red         (170,0,0)
		0x00AA00FF, // 2: Green       (0,170,0)
		0xAA5500FF, // 3: Yellow      (170,85,0)
		0x0000AAFF, // 4: Blue        (0,0,170)
		0xAA00AAFF, // 5: Magenta     (170,0,170)
		0x00AAAAFF, // 6: Cyan        (0,170,170)
		0xAAAAAAFF, // 7: White       (170,170,170)
		0x555555FF, // 8: Bright Black (Gray) (85,85,85)
		0xFF5555FF, // 9: Bright Red  (255,85,85)
		0x55FF55FF, // 10: Bright Green (85,255,85)
		0xFFFF55FF, // 11: Bright Yellow (255,255,85)
		0x5555FFFF, // 12: Bright Blue (85,85,255)
		0xFF55FFFF, // 13: Bright Magenta (255,85,255)
		0x55FFFFFF, // 14: Bright Cyan (85,255,255)
		0xFFFFFFFF  // 15: Bright White (255,255,255)
	};

	_dosFontPalette = paletteManager.createPalette("Ansi16Colors", paletteData, 16);

	// Load the DOS font image
	ImageFile fontTextureFile;
	ImageLoadParams params;

	resourceSystem.getImageBMPFileProcessor().load(fontTextureFile, "DosFont", dosFont, DOSFONT_SIZE, params);
	OpenXcom::OwningHandle<OpenXcom::HostImage> hostFontTexture = fontTextureFile.takeImage();

	// Move to device
	OpenXcom::OwningHandle<OpenXcom::DeviceImage> deviceFontTexture = imageManager.createDeviceImage(*hostFontTexture);
	std::vector<OpenXcom::OwningHandle<OpenXcom::DeviceImage>> textures;
	textures.push_back(std::move(deviceFontTexture));

	// Create the font object
	OpenXcom::FontSettings settings;
	settings.width = 9;
	settings.height = 16;
	settings.spacing = 0;
	settings.defaultPaletteIndex = 1;
	settings.numPaletteEntries = 1;

	_dosFont = fontManager.load("dosFont", settings, std::move(textures), getAsciiGlyphs(textures[0].getHandle()));
}

void StartState::onRender(GraphicsCommand& command)
{
	_text->draw(command);
}

} // namespace OpenXcom
