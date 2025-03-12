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
#include "ImageFileProcessor.h"

#include "ImageFile.h"
#include "ImagePNGFileProcessor.h"
#include "ImageBMPFileProcessor.h"

#include "../ResourceSystem.h"

#include "../../EngineContext.h"
#include "../../Logger.h"

namespace OpenXcom
{

ImageFileProcessor::ImageFileProcessor(EngineContext& context)
	: _context(context)
{

}

ImageFileProcessor::~ImageFileProcessor()
{
}

ImageFile ImageFileProcessor::load(const std::string& name, std::filesystem::path file, ImageLoadParams& params)
{
	// Extract the extension, remove the leading dot, and convert to lowercase
	std::string ext = file.extension().string();
	if (!ext.empty() && ext.front() == '.')
		ext.erase(0, 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

	ResourceSystem& resourceSystem = _context.getResourceSystem();

	// Route to the correct function based on the extension
	if (ext == "png")
	{
		ImagePNGFileProcessor& pngProcessor = resourceSystem.getImagePNGFileProcessor();
		return pngProcessor.load(name, file, params);
	}
	else if (ext == "bmp")
	{
		ImageBMPFileProcessor& bmpProcessor = resourceSystem.getImageBMPFileProcessor();
		return bmpProcessor.load(name, file, params);
	}
	else
	{
		Log(LOG_ERROR) << "Unsupported extension: " << ext << "\n";
		return ImageFile();
	}
}

ImageFile ImageFileProcessor::load(const std::string& name, std::filesystem::path file)
{
	ImageLoadParams params;
	return load(name, file, params);
}


} // namespace OpenXcom
