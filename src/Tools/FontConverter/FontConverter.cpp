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
#include <iostream>
#include <fstream>
#include <filesystem>
#include <functional>
#include <vector>

#include "../../Engine/Engine.h"
#include "../../Engine/Logger.h"
#include "../../Engine/Json.h"
#include "../../Engine/Graphics/Font/Font.h"
#include "../../Engine/Graphics/Font/FontPack.h"
#include "../../Engine/Graphics/Font/FontManager.h"

#include <utf8.h>

using namespace OpenXcom;

/*
 * The font converter is a simple tool that converts the old yaml font pack files to singular json files.
 */


namespace OpenXcom
{
// Specializations for the Font class to allow it to be converted to and from json.
// Note: I'm not using the run time type refection here because it requires special
// handling of the images
template <>
bool fromJson<Font>(const nlohmann::json& json, Font& type)
{
	return false;
}

template <>
bool toJson<Font>(const Font& font, nlohmann::json& json)
{
	json["name"] = font.getName();
	toJson(font.getSettings(), json["settings"]);

	// images
	nlohmann::json images = nlohmann::json::array();
	int index = 0;
	for (const OwningHandle<DeviceImage>& image : font._fontTextures)
	{

		std::string name = font.getName() + "_" + std::to_string(index) + ".png";
		images.push_back(name);
		index++;
	}

	json["images"] = images;

	// ascii glyphs
	nlohmann::json glyphs = nlohmann::json();
	for (int i = 0; i < 128; i++)
	{
		const Glyph& glyph = font._asciiGlyphs[i];

		// throw out any glyphs that have negative values
		if (glyph.x < 0 || glyph.y < 0 || glyph.width < 0 || glyph.height < 0)
		{
			continue;
		}

		// throw out any glyphs that don't have an image
		if (!glyph.image.isValid())
		{
			continue;
		}
		
		std::string strChar(1, static_cast<char>(i));
		// get the index of the glyph image
		int index = 0;
		bool found = false;
		for (int i = 0; i < font._fontTextures.size(); i++)
		{
			if (font._fontTextures[i].getHandle() == glyph.image)
			{
				index = i;
				found = true;
				break;
			}
		}

		if(!found)
		{
			Log(LOG_ERROR) << "Failed to find image for glyph '" + strChar + "'" << std::endl;
			continue;
		}

		nlohmann::json glyphJson;
		glyphJson["image_index"] = index;
		glyphJson["x"] = glyph.x;
		glyphJson["y"] = glyph.y;
		glyphJson["width"] = glyph.width;
		glyphJson["height"] = glyph.height;
		glyphJson["x_advance"] = glyph.xAdvance;
		glyphJson["x_offset"] = glyph.xOffset;
		glyphJson["y_offset"] = glyph.yOffset;

		glyphs[strChar] = glyphJson;
	}

	// extended glyphs
	for (const auto& [codepoint, glyph] : font._extendedGlyphs)
	{
		// throw out any glyphs that have negative values
		if (glyph.x < 0 || glyph.y < 0 || glyph.width < 0 || glyph.height < 0)
		{
			continue;
		}

		// throw out any glyphs that don't have an image
		if (!glyph.image.isValid())
		{
			continue;
		}

		std::u32string u32Char(1, codepoint);
		std::string strChar;
		utf8::utf32to8(u32Char.begin(), u32Char.end(), std::back_inserter(strChar));

		// get the index of the glyph image
		int index = 0;
		bool found = false;
		for (int i = 0; i < font._fontTextures.size(); i++)
		{
			if (font._fontTextures[i].getHandle() == glyph.image)
			{
				index = i;
				found = true;
				break;
			}
		}
		if (!found)
		{
			Log(LOG_ERROR) << "Failed to find image for glyph '" + strChar + "'" << std::endl;
			continue;
		}
		nlohmann::json glyphJson;
		glyphJson["image_index"] = index;
		glyphJson["x"] = glyph.x;
		glyphJson["y"] = glyph.y;
		glyphJson["width"] = glyph.width;
		glyphJson["height"] = glyph.height;
		glyphJson["x_advance"] = glyph.xAdvance;
		glyphJson["x_offset"] = glyph.xOffset;
		glyphJson["y_offset"] = glyph.yOffset;
		glyphs[strChar] = glyphJson;
	}

	json["glyphs"] = glyphs;

	return true;
}

} // namespace OpenXcom



struct CommandLineArguments
{
	std::filesystem::path fontPackFile;
	std::filesystem::path outputPath;
};

bool parseCommandLine(CommandLineArguments& args, int argc, char* argv[])
{
	// convert to vector for easier handling
	std::vector<std::string> cmdArgs(argv, argv + argc);

	std::vector<std::pair<std::string, std::function<void(const std::string&)>>> argHandlers = {
		{"-font", [&args](const std::string& arg) { args.fontPackFile = arg; }},
		{"-output", [&args](const std::string& arg) { args.outputPath = arg; }}};

	std::vector<std::string>::iterator it = ++cmdArgs.begin(); // skip the first argument which is the executable name
	while (it != cmdArgs.end())
	{
		bool handlerFound = false;
		for (const auto& handler : argHandlers)
		{
			if (*it == handler.first)
			{
				handlerFound = true;
				++it;
				if (it != cmdArgs.end())
				{
					handler.second(*it);
				}
				else
				{
					Log(LOG_ERROR) << "No argument provided for \"" << handler.first << "\"." << std::endl;
					return false;
				}
			}
		}
		if (!handlerFound)
		{
			Log(LOG_ERROR) << "Unknown argument \"" << *it << "\"." << std::endl;
			return false;
		}
		++it;
	}

	// check that the template was specified in the command line
	if (args.fontPackFile.empty())
	{
		Log(LOG_ERROR) << "No font pack file specified" << std::endl;
		return false;
	}

	// check the template actually exists
	if (!std::filesystem::exists(args.fontPackFile))
	{
		Log(LOG_ERROR) << "Font pack file could not be found" << std::endl;
		return false;
	}

	// check that the output was specified in the command line
	if (args.outputPath.empty())
	{
		Log(LOG_ERROR) << "No output directory specified" << std::endl;
		return false;
	}



	return true;
}

void printUsage()
{
	std::cout << "Usage: fontconv -font <font definition file> -output <output directory>" << std::endl;
}



int main(int argc, char* argv[])
{
	CommandLineArguments args;
	if (!parseCommandLine(args, argc, argv))
	{
		printUsage();
		return 1;
	}

	// we need to initialize the engine to load the resources
	std::vector<std::string> engineArgs = {"-headless"};
	std::unique_ptr<Engine> _engine = std::make_unique<Engine>(engineArgs);

	// load the font pack
	FontPack fontPack(_engine->getEngineContext(), args.fontPackFile);

	const std::vector<OwningHandle<Font>>& fonts = fontPack.getFonts();
	if(fonts.size() == 0)
	{
		Log(LOG_ERROR) << "No fonts found in font pack" << std::endl;
		return -1;
	}

	// check the path to the output actually exists, and if not, create it
	if (!std::filesystem::exists(args.outputPath))
	{
		if (!std::filesystem::create_directories(args.outputPath))
		{
			Log(LOG_ERROR) << "Failed to create output directory" << std::endl;
			return -1;
		}
	}

	for (const OwningHandle<Font>& fontHandle : fonts)
	{
		Font& font = *fontHandle;

		std::filesystem::path outputPath = args.outputPath / (font.getName() + ".json");
		std::ofstream output(outputPath);
		if (!output.is_open())
		{
			Log(LOG_ERROR) << "Failed to open output file: " << outputPath << std::endl;
			return -1;
		}

		nlohmann::json json;
		if(!toJson(font, json))
		{
			Log(LOG_ERROR) << "Failed to convert font to json" << std::endl;
			return -1;
		}

		output << json.dump(1, '\t', true);

		output.close();
	}

	return 0;
}
