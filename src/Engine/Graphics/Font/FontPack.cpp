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
#include "FontPack.h"

#include "FontManager.h"

#include "../Image/Image.h"
#include "../Image/ImageManager.h"

#include "../../EngineContext.h"
#include "../../Logger.h"
#include "../../Filesystem/VirtualFileSystem.h"
#include "../../Resource/ResourceSystem.h"
#include "../../Resource/FileProcessor/ImageFile.h"
#include "../../Resource/FileProcessor/ImageFileProcessor.h"

#include <ryml.hpp>

#include "../../Utility/RTTR.h"
#include "../../Yaml.h"

namespace OpenXcom
{

inline bool operator==(const FileFontDefinition& lhs, const FileFontDefinition& rhs)
{
	return lhs.id == rhs.id; // compare all relevant members
}

inline bool operator==(const FileFontImageDefinition& lhs, const FileFontImageDefinition& rhs)
{
	return lhs.file == rhs.file; // compare all relevant members
}

SIMPLERTTR
{
	
	SimpleRTTR::registration().type<FileFontImageDefinition>()
		.meta("Serialize", ObjectSerialize::ALWAYS) // always serialize this
		.property(REGISTER_PROPERTY(FileFontImageDefinition, file))
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(REGISTER_PROPERTY(FileFontImageDefinition, chars))
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(REGISTER_PROPERTY(FileFontImageDefinition, width))
			.meta("Serialize", PropertySerialize::OPTIONAL)
		.property(REGISTER_PROPERTY(FileFontImageDefinition, spacing))
			.meta("Serialize", PropertySerialize::OPTIONAL);

	SimpleRTTR::registration().type<FileFontDefinition>()
		.meta("Serialize", ObjectSerialize::ALWAYS) // always serialize this
		.property(REGISTER_PROPERTY(FileFontDefinition, id))
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(REGISTER_PROPERTY(FileFontDefinition, width))
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(REGISTER_PROPERTY(FileFontDefinition, height))
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(REGISTER_PROPERTY(FileFontDefinition, spacing))
			.meta("Serialize", PropertySerialize::OPTIONAL)
		.property(REGISTER_PROPERTY(FileFontDefinition, images))
			.meta("Serialize", PropertySerialize::ALWAYS);

	SimpleRTTR::registration().type<FileFont>()
		.meta("Serialize", ObjectSerialize::ALWAYS) // always serialize this
		.property(REGISTER_PROPERTY(FileFont, fonts))
			.meta("Serialize", PropertySerialize::ALWAYS);

}

FontPack::FontPack(EngineContext& context, const std::filesystem::path& fontPackPath)
	: _context(context)
{
	load(fontPackPath);
}

FontPack::~FontPack()
{
}

// Load the font pack
void FontPack::load(const std::filesystem::path& path)
{
	ResourceSystem& resourceSystem = _context.getResourceSystem();
	FontManager& fontManager = resourceSystem.getFontManager();
	ImageManager& imageManager = resourceSystem.getImageManager();
	ImageFileProcessor& imageFileProcessor = resourceSystem.getImageFileProcessor();

	// use the virtual file system to find the font file
	FileSystem& vfs = _context.getVirtualFileSystem().getDataFileSystem();
	std::unique_ptr<FileEntry> file = vfs.getFile(path);

	if (!file)
	{
		Log(LOG_ERROR) << "FontPack: Could not find font file: " << path.string();
		return;
	}

	std::unique_ptr<std::istream> stream = file->openRead();
	std::string contents(std::istreambuf_iterator<char>(*stream), {});

	// Read the file into memory
	ryml::substr substr(contents.data(), contents.size());
	ryml::Tree tree = ryml::parse_in_place(substr);

	// Parse the YAML document
	FileFont fileFonts;
	fromYaml(tree.rootref(), fileFonts);

	// Load the fonts
	for (const FileFontDefinition& fontDef : fileFonts.fonts)
	{
		std::vector<OwningHandle<DeviceImage>> images;

		// Load the font images
		std::array<Glyph, 128> asciiGlyphs;
		std::unordered_map<char32_t, Glyph> extendedGlyphs;

		for (const FileFontImageDefinition& imageDef : fontDef.images)
		{
			// Load the image
			std::filesystem::path imagePath = path.parent_path() / imageDef.file;
			std::string name = fontDef.id + "_FontImage";

			ImageFile imageFile;
			imageFileProcessor.load(imageFile, name, imagePath);

			// We still need to keep the host image around for this function because we parse out the width of
			// each char from the actual image data itself.
			OwningHandle<HostImage> hostImage = imageFile.takeImage();

			if (hostImage->getFormat() != ImageFormat::R8)
			{
				Log(LOG_ERROR) << "FontPack: Font image must be R8 format: " << imagePath.string();
				continue;
			}

			// Move HostImage to device, which is necessary in order to get the resource handle for the glyph rendering
			OwningHandle<DeviceImage> deviceImage = imageManager.createDeviceImage(*hostImage);

			const uint8_t* pixels = static_cast<const uint8_t*>(hostImage->map());
			uint32_t imageWidth = hostImage->getExtent().x; // total width of the image
			uint32_t cellWidth = fontDef.width;            // defined in your YAML file
			uint32_t cellHeight = fontDef.height;          // defined in your YAML file

			// Calculate how many glyphs fit in one row:
			uint32_t columns = imageWidth / cellWidth;

			size_t glyphIndex = 0;
			for(char32_t character : imageDef.chars)
			{
				// Parse out the actual width and spacing
				//	KN NOTE: The old system for loading fonts used this as a means to define the width of individual
				// characters. This is no longer necessary, as the width of each character is defined in the font file
				// itself, but we need to retain backwards compatibility.

				 
				// Determine cell starting position
				int startX = static_cast<int>((glyphIndex % columns) * cellWidth);
				int startY = static_cast<int>((glyphIndex / columns) * cellHeight);

				// For non-monospace, scan for the actual glyph boundaries.
				int left = -1, right = -1;

				// Find the left boundary
				for(int x = startX; x < startX + static_cast<int>(cellWidth) && left == -1; ++x)
				{
					for (int y = startY; y < startY + static_cast<int>(cellHeight); ++y)
					{
						size_t index = ((y * imageWidth) + x);
						if (pixels[index] != 0)
						{ // non-zero pixel found
							left = x;
							break;
						}
					}
				}

				// Find the right boundary
				for (int x = startX + cellWidth - 1; x >= startX && right == -1; --x)
				{
					for (int y = startY; y < startY + static_cast<int>(cellHeight); ++y)
					{
						size_t index = ((y * imageWidth) + x);
						if (pixels[index] != 0)
						{
							right = x;
							break;
						}
					}
				}

				// Set the glyph rectangle
				Glyph glyph;
				glyph.image = deviceImage.getHandle();

				glyph.x = left;
				glyph.y = startY;
				glyph.width = (right - left + 1);
				glyph.height = cellHeight;
				glyph.xAdvance = glyph.width + fontDef.spacing;
				glyph.xOffset = 0;
				glyph.yOffset = 0;

				// Add the glyph to the appropriate map
				if (character < 128)
				{
					asciiGlyphs[character] = glyph;
				}
				else
				{
					extendedGlyphs[character] = glyph;
				}

				++glyphIndex;
			}
			hostImage->unmap();

			images.push_back(std::move(deviceImage));
		}
		// special case for space
		asciiGlyphs[' '] = Glyph{0, 0, 0, 0, 0, 0, static_cast<char>(fontDef.width/2 + fontDef.spacing), images[0].getHandle()};

		// Load the font
		FontSettings settings;
		settings.width = fontDef.width;
		settings.height = fontDef.height;
		settings.spacing = fontDef.spacing;
		settings.defaultPaletteIndex = 1;
		settings.numPaletteEntries = 5;

		OwningHandle<Font> font = fontManager.load(fontDef.id, settings, std::move(images), asciiGlyphs, extendedGlyphs);
		_fontPackHandles.push_back(std::move(font));
	}
}

const std::vector<OwningHandle<Font>>& FontPack::getFonts() const
{
	return _fontPackHandles;
}

} // namespace OpenXcom
