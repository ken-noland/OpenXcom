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
#include "ModLoader.h"
#include "../../Mod.h"
#include "../../../Logger.h"
#include "../../../YamlFile.h"
#include "../../../Utility/RTTR.h"
#include "../../../Filesystem/CompositeFileSystem.h"

#include "../../../EngineContext.h"

#include "../../../Graphics/Image/Image.h"
#include "../../../Graphics/Image/ImageManager.h"

#include "../../../Resource/ResourceSystem.h"
#include "../../../Resource/FileProcessor/ImageFile.h"
#include "../../../Resource/FileProcessor/ImageBDYFileProcessor.h"
#include "../../../Resource/FileProcessor/ImagePCKFileProcessor.h"
#include "../../../Resource/FileProcessor/ImagePNGFileProcessor.h"
#include "../../../Resource/FileProcessor/ImageSCRFileProcessor.h"
#include "../../../Resource/FileProcessor/ImageSPKFileProcessor.h"
#include "../../../Resource/FileProcessor/PaletteDATFileProcessor.h"


namespace OpenXcom
{

namespace ModLoader_8_1_2
{

// register the structs for serialization
SIMPLERTTR
{
	SimpleRTTR::registration().type<SoundDefinition>()
		.meta("Serialize", ObjectSerialize::ALWAYS)
		.property(&SoundDefinition::type, "type")
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(&SoundDefinition::file, "file")
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(&SoundDefinition::sounds, "sounds")
			.meta("Serialize", PropertySerialize::OPTIONAL)
		.property(&SoundDefinition::soundRanges, "soundRanges")
			.meta("Serialize", PropertySerialize::OPTIONAL)
		;

	SimpleRTTR::registration().type<TransparencyLUTDefinition>()
		.meta("Serialize", ObjectSerialize::ALWAYS)
		.property(&TransparencyLUTDefinition::colors, "colors")
			.meta("Serialize", PropertySerialize::ALWAYS)
		;

	SimpleRTTR::registration().type<ResourceConfigFile>()
		.meta("Serialize", ObjectSerialize::ALWAYS)
		.property(&ResourceConfigFile::soundDefs, "soundDefs")
			.meta("Serialize", PropertySerialize::ALWAYS)
		.property(&ResourceConfigFile::transparencyLUTs, "transparencyLUTs")
			.meta("Serialize", PropertySerialize::ALWAYS)
		;

}

bool loadVanillaResources(EngineContext& context, Mod* mod, ResourceConfigFile& resourceConfig);

bool loadPaletteData(EngineContext& context, Mod* mod, ResourceConfigFile& resourceConfig);
bool loadBackpalsData(EngineContext& context, Mod* mod, ResourceConfigFile& resourceConfig);

bool loadSCRImage(EngineContext& context, Mod* mod, const std::string& name, std::filesystem::path path, const glm::ivec2& extents);
bool loadBDYImage(EngineContext& context, Mod* mod, const std::string& name, std::filesystem::path path, const glm::ivec2& extents);
bool loadSPKImage(EngineContext& context, Mod* mod, const std::string& name, std::filesystem::path path, const glm::ivec2& extents);

bool loadImages(EngineContext& context, Mod* mod, ResourceConfigFile& resourceConfig);

// global function to load 8.1.2 version mods
bool load(EngineContext& context, Mod* mod)
{
	assert(mod);
	assert(&mod->getFileSystem());

	// if the mod has a resourceConfigFile, we need to extract the sounds and transparency LUTs
	ResourceConfigFile resourceConfig;
	if(!mod->getInfo().resourceConfigFile.empty())
	{
		Log(LOG_INFO) << "Pre-loading rulesets...";

		CompositeFileSystem& fs = mod->getFileSystem();

		std::unique_ptr<FileEntry> resourceConfigFile = fs.getFile(mod->getInfo().resourceConfigFile);
		if (!resourceConfigFile)
		{
			Log(LOG_ERROR) << "Failed to load resource config file '" << mod->getInfo().resourceConfigFile << "' for mod '" << mod->getInfo().id << "'";
			return false;
		}

		// load the resource config file
		YamlFile yamlFile;
		resourceConfig = yamlFile.load<ResourceConfigFile>(resourceConfigFile);
	}

	Log(LOG_INFO) << "Loading vanilla resources...";

	// now we can load the vanilla resources
	if(!loadVanillaResources(context, mod, resourceConfig))
	{
		return false;
	}

	return true;
}

bool loadVanillaResources(EngineContext& context, Mod* mod, ResourceConfigFile& resourceConfig)
{
	// ---
	// Load in the palettes

	// load in "PAL_GEOSCAPE", "PAL_BASESCAPE", "PAL_GRAPHS", and (optionally) "PAL_UFOPAEDIA", "PAL_BATTLEPEDIA"
	if (!loadPaletteData(context, mod, resourceConfig)) { return false;	}

	// load in "BACKPALS"
	if (!loadBackpalsData(context, mod, resourceConfig)) { return false; }

	// TODO: there's a chunk in the original source which loaded and "corrected" the "PAL_BATTLESCAPE" palette, but I'm not sure it's necessary anymore. I'll leave it out for now.

	// ---
	// Load images
	if (!loadImages(context, mod, resourceConfig)) { return false; }

	return true;
}

bool loadPaletteData(EngineContext& context, Mod* mod, ResourceConfigFile& resourceConfig)
{
	ResourceSystem& resourceSystem = context.getResourceSystem();
	PaletteDATFileProcessor& paletteDATProcessor = resourceSystem.getPaletteDATFileProcessor();

	// Load palettes
	std::unique_ptr<FileEntry> paletteFile = mod->getFileSystem().getFile("GEODATA/PALETTES.DAT");
	if (!paletteFile)
	{
		Log(LOG_ERROR) << "Failed to open palettes file 'GEODATA/PALETTES.DAT' for mod '" << mod->getInfo().id << "'";
		CompositeFileSystem& fs = mod->getFileSystem();
		for (const std::unique_ptr<FileSystem>& filesystem : fs.getFileSystems())
		{
			Log(LOG_ERROR) << "    Searched in : " << filesystem->getPath();
		}
		return false;
	}

	PaletteLoadParams params;
	params.paletteEntries = 256;
	params.filePaletteEntries = 258; // two extra entries(no idea what they are for!)

	std::vector<std::string> paletteNames;
	size_t numPalettes = paletteDATProcessor.getPaletteCount(paletteFile, params);

	if(numPalettes == 5)
	{
		// xcom 1
		paletteNames = { "PAL_GEOSCAPE", "PAL_BASESCAPE", "PAL_GRAPHS", "PAL_UFOPAEDIA", "PAL_BATTLEPEDIA" };
	}
	else if(numPalettes == 3)
	{
		// xcom 2
		paletteNames = { "PAL_GEOSCAPE", "PAL_BASESCAPE", "PAL_GRAPHS" };
	}
	else
	{
		Log(LOG_ERROR) << "Unknown number of palettes in GEODATA/PALETTES.DAT";
		return false;
	}
	
	PaletteFile paletteFileData = paletteDATProcessor.load(paletteNames, paletteFile, params);
	if(paletteFileData.getPalettes().size() != paletteNames.size())
	{
		Log(LOG_ERROR) << "Failed to load GEODATA/PALETTES.DAT palettes";
		return false;
	}

	// register the palettes with the mod
	while (!paletteFileData.getPalettes().empty())
	{
		mod->registerPalette(paletteFileData.takePalette(0));
	}

	return true;
}

bool loadBackpalsData(EngineContext& context, Mod* mod, ResourceConfigFile& resourceConfig)
{
	ResourceSystem& resourceSystem = context.getResourceSystem();
	PaletteDATFileProcessor& paletteDATProcessor = resourceSystem.getPaletteDATFileProcessor();

	// load in backpals
	std::unique_ptr<FileEntry> backpalFile = mod->getFileSystem().getFile("GEODATA/BACKPALS.DAT");
	if (!backpalFile)
	{
		Log(LOG_ERROR) << "Failed to open palettes file GEODATA/BACKPALS.DAT";
		CompositeFileSystem& fs = mod->getFileSystem();
		for (const std::unique_ptr<FileSystem>& filesystem : fs.getFileSystems())
		{
			Log(LOG_ERROR) << "    Searched in : " << filesystem->getPath();
		}
		return false;
	}

	PaletteLoadParams params;
	params.paletteEntries = 128;
	params.filePaletteEntries = 128;

	PaletteFile backpalFileData = paletteDATProcessor.load({"BACKPALS"}, backpalFile, params);
	if(backpalFileData.getPalettes().size() != 1)
	{
		Log(LOG_ERROR) << "Failed to load GEODATA/BACKPALS.DAT backpals";
		return false;
	}

	// register the backpals with the mod
	mod->registerPalette(backpalFileData.takePalette(0));
	
	return true;
}

bool loadImages(EngineContext& context, Mod* mod, ResourceConfigFile& resourceConfig)
{
	if (!loadSCRImage(context, mod, "INTERWIN.DAT", "GEODATA/INTERWIN.DAT", {160, 600})) { return false; }

	std::unique_ptr<FolderEntry> geographFolder = mod->getFileSystem().getFolder("GEOGRAPH");
	if (!geographFolder)
	{
		Log(LOG_ERROR) << "Failed to open folder GEOGRAPH";
		return false;
	}

	// Loop through all the files in the GEOGRAPH folder
	for (std::unique_ptr<VFSEntry> entry : *geographFolder)
	{
		// Only process files
		if (entry->getType() == VFSEntry::EntryType::File)
		{
			std::unique_ptr<FileEntry> file(static_cast<FileEntry*>(entry.release()));
			std::string ext = file->getPath().extension().string();

			// Remove preceding dot and convert to lowercase
			if (!ext.empty() && ext.front() == '.')
				ext.erase(0, 1);
			std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

			std::string name = file->getPath().stem().string();
			std::filesystem::path path = file->getPath();
			if (ext == "scr")
			{
				if (!loadSCRImage(context, mod, name, path, {320, 200})) { return false; }
			}
			else if(ext == "bdy")
			{
				if (!loadBDYImage(context, mod, name, path, {320, 200})) { return false; }
			}
			else if(ext == "spk")
			{
				if (!loadSPKImage(context, mod, name, path, {320, 200})) { return false; }
			}
		}
	}

	// Load the surface sets

	return true;
}


bool loadSCRImage(EngineContext& context, Mod* mod, const std::string& name, std::filesystem::path path, const glm::ivec2& extents)
{
	ResourceSystem& resourceSystem = context.getResourceSystem();
	ImageSCRFileProcessor& imageSCRProcessor = resourceSystem.getImageSCRFileProcessor();

	// Open the file
	std::unique_ptr<FileEntry> surfaceFile = mod->getFileSystem().getFile(path);
	if (!surfaceFile)
	{
		Log(LOG_ERROR) << "Failed to open image file " << path.string();
		return false;
	}

	// Load the image
	ImageSCRLoadParams params{extents};
	ImageFile imageFile = imageSCRProcessor.load(name, surfaceFile, params);
	if (!imageFile.image)
	{
		Log(LOG_ERROR) << "Failed to load SCR image " << path.string();
		return false;
	}

	// Send the image to the device
	OwningHandle<DeviceImage> deviceImage = resourceSystem.getImageManager().createDeviceImage(*imageFile.image);

	mod->registerImage(std::move(deviceImage));

	return true;
}

bool loadBDYImage(EngineContext& context, Mod* mod, const std::string& name, std::filesystem::path path, const glm::ivec2& extents)
{
	ResourceSystem& resourceSystem = context.getResourceSystem();
	ImageBDYFileProcessor& imageBDYProcessor = resourceSystem.getImageBDYFileProcessor();

	// Open the file
	std::unique_ptr<FileEntry> surfaceFile = mod->getFileSystem().getFile(path);
	if (!surfaceFile)
	{
		Log(LOG_ERROR) << "Failed to open image file " << path.string();
		return false;
	}

	// Load the image
	ImageBDYLoadParams params{extents};
	ImageFile imageFile = imageBDYProcessor.load(name, surfaceFile, params);
	if (!imageFile.image)
	{
		Log(LOG_ERROR) << "Failed to load BDY image " << path.string();
		return false;
	}

	// Send the image to the device
	OwningHandle<DeviceImage> deviceImage = resourceSystem.getImageManager().createDeviceImage(*imageFile.image);

	mod->registerImage(std::move(deviceImage));

	return true;
}

bool loadSPKImage(EngineContext& context, Mod* mod, const std::string& name, std::filesystem::path path, const glm::ivec2& extents)
{
	ResourceSystem& resourceSystem = context.getResourceSystem();
	ImageSPKFileProcessor& imageSPKProcessor = resourceSystem.getImageSPKFileProcessor();

	// Open the file
	std::unique_ptr<FileEntry> surfaceFile = mod->getFileSystem().getFile(path);
	if (!surfaceFile)
	{
		Log(LOG_ERROR) << "Failed to open image file " << path.string();
		return false;
	}

	// Load the image
	ImageSPKLoadParams params{extents};
	ImageFile imageFile = imageSPKProcessor.load(name, surfaceFile, params);
	if (!imageFile.image)
	{
		Log(LOG_ERROR) << "Failed to load BDY image " << path.string();
		return false;
	}

	// Send the image to the device
	OwningHandle<DeviceImage> deviceImage = resourceSystem.getImageManager().createDeviceImage(*imageFile.image);

	mod->registerImage(std::move(deviceImage));

	return true;
}

bool loadPCKImages(EngineContext& context, Mod* mod, const std::string& pckName, const std::string& tabName, std::filesystem::path path, const glm::ivec2& extents)
{
	ResourceSystem& resourceSystem = context.getResourceSystem();
	ImagePCKFileProcessor& imagePCKProcessor = resourceSystem.getImagePCKFileProcessor();

	// Open the files
	std::unique_ptr<FileEntry> pckFile = mod->getFileSystem().getFile(path / pckName);
	if (!pckFile)
	{
		Log(LOG_ERROR) << "Failed to open image PCK file " << (path / pckName).string();
		return false;
	}

	std::unique_ptr<FileEntry> tabFile = mod->getFileSystem().getFile(path / tabName);
	if (!tabFile)
	{
		Log(LOG_ERROR) << "Failed to open image TAB file " << (path / tabName).string();
		return false;
	}

	// Load the images
	ImageSetFile imageSetFile = imagePCKProcessor.load(pckName, pckFile, tabFile, {extents});
	if (imageSetFile.images.empty())
	{
		Log(LOG_ERROR) << "Failed to load PCK image " << path.string();
		return false;
	}

	// Send the images to the device
	for (OwningHandle<HostImage>& hostImage : imageSetFile.images)
	{
		OwningHandle<DeviceImage> deviceImage = resourceSystem.getImageManager().createDeviceImage(*hostImage);
		mod->registerImage(std::move(deviceImage));
	}
	return true;
}

} // namespace ModLoader_8_1_2

} // namespace OpenXcom
