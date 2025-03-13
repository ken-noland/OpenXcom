#pragma once
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
#include <memory>

namespace OpenXcom
{

class EngineContext;

class ShaderManager;
class PipelineManager;
class BufferManager;

class FontManager;
class PaletteManager;
class ImageManager;

class VirtualFileSystem;
class GraphicsSystem;

// Loaders
class ImageFileProcessor;
class ImageBDYFileProcessor;
class ImageBMPFileProcessor;
class ImagePNGFileProcessor;
class ImageSCRFileProcessor;
class ImageSPKFileProcessor;
class PaletteDATFileProcessor;

class ResourceSystem
{
	std::unique_ptr<ShaderManager> _shaderManager;
	std::unique_ptr<PipelineManager> _pipelineManager;
	std::unique_ptr<BufferManager> _bufferManager;

	std::unique_ptr<PaletteManager> _paletteManager;
	std::unique_ptr<FontManager> _fontManager;
	std::unique_ptr<ImageManager> _imageManager;

	// Loaders
	std::unique_ptr<ImageFileProcessor> _imageFileProcessor;
	std::unique_ptr<ImageBDYFileProcessor> _imageBDYFileProcessor;
	std::unique_ptr<ImageBMPFileProcessor> _imageBMPFileProcessor;
	std::unique_ptr<ImagePNGFileProcessor> _imagePNGFileProcessor;
	std::unique_ptr<ImageSCRFileProcessor> _imageSCRFileProcessor;
	std::unique_ptr<ImageSPKFileProcessor> _imageSPKFileProcessor;
	std::unique_ptr<PaletteDATFileProcessor> _paletteDATFileProcessor;

public:
	ResourceSystem(EngineContext& context);
	virtual ~ResourceSystem();

	ShaderManager& getShaderManager() { return *_shaderManager; };
	PipelineManager& getPipelineManager() { return *_pipelineManager; };
	BufferManager& getBufferManager() { return *_bufferManager; };

	PaletteManager& getPaletteManager() { return *_paletteManager; };
	FontManager& getFontManager() { return *_fontManager; };
	ImageManager& getImageManager() { return *_imageManager; };

	// Loaders
	ImageFileProcessor& getImageFileProcessor() { return *_imageFileProcessor; };
	ImageBDYFileProcessor& getImageBDYFileProcessor() { return *_imageBDYFileProcessor; };
	ImageBMPFileProcessor& getImageBMPFileProcessor() { return *_imageBMPFileProcessor; };
	ImagePNGFileProcessor& getImagePNGFileProcessor() { return *_imagePNGFileProcessor; };
	ImageSCRFileProcessor& getImageSCRFileProcessor() { return *_imageSCRFileProcessor; };
	ImageSPKFileProcessor& getImageSPKFileProcessor() { return *_imageSPKFileProcessor; };
	PaletteDATFileProcessor& getPaletteDATFileProcessor() { return *_paletteDATFileProcessor; };
};

} // namespace OpenXcom
