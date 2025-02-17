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

// Recursive function to walk through a YAML document
void traverse_node(ryml::ConstNodeRef node, int indent = 0)
{
	// Create an indentation string based on the depth
	std::string indentation(indent * 2, ' ');

	// Check if the current node is a map
	if (node.is_map())
	{
		// Iterate over each key-value pair in the map
		for (ryml::ConstNodeRef child : node.children())
		{
			if (child.has_val())
			{
				Log(LOG_DEBUG) << indentation << child.key() << ": " << child.val();
			}
			else
			{
				Log(LOG_DEBUG) << indentation << child.key() << ": ";
				traverse_node(child, indent + 1);
			}
		}
	}
	// Check if the current node is a sequence (list)
	else if (node.is_seq())
	{
		// Iterate over each element in the sequence
		for (ryml::ConstNodeRef child : node.children())
		{
			// For nested sequences or maps, print a newline and recurse
			traverse_node(child, indent + 1);
		}
	}
	// If the node is a scalar (a leaf), simply print it
	else if (node.is_val())
	{
		Log(LOG_DEBUG) << std::string(indentation, '\t') << node << "\n";
	}
}


void FontPack::load(const std::filesystem::path& path)
{
	// Load the font pack

	// use the virtual file system to find the font file
	FileSystem& vfs = _context.getVirtualFileSystem().getDataFileSystem();
	std::unique_ptr<FileEntry> file = vfs.getFile(path);

	if (!file)
	{
		Log(LOG_ERROR) << "FontPack: Could not find font file: " << path.string();
		return;
	}

	FileEntry::IStreamPtr stream = file->openRead();
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
		// Load the font images
		std::array<Glyph, 128> asciiGlyphs;
		std::unordered_map<char32_t, Glyph> extendedGlyphs;
		for (const FileFontImageDefinition& imageDef : fontDef.images)
		{
			// Load the image
			std::filesystem::path imagePath = path.parent_path() / imageDef.file;
			ImageFile imageFile;
			_context.getResourceSystem().getImageFileProcessor().load(imageFile, "", imagePath);

		//	OwningHandle<DeviceImage> image = _context.getResourceSystem().getImageManager().load(imagePath);

		//	// Load the glyphs
		//	for (char c : imageDef.chars)
		//	{
		//		if (c < 128)
		//		{
		//			asciiGlyphs[c] = Glyph{image, c};
		//		}
		//		else
		//		{
		//			extendedGlyphs[c] = Glyph{image, c};
		//		}
		//	}
		}

		//// Load the font
		//OwningHandle<Font> font = _context.getResourceManager().getFontManager().load(fontDef.id, asciiGlyphs, extendedGlyphs);
		//_fontPackHandles.push_back(font);
	}

}

} // namespace OpenXcom
