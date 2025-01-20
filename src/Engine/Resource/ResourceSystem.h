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

class ShaderManager;
class PipelineManager;
class FontManager;
class PaletteManager;
class ImageManager;

class VirtualFileSystem;
class GraphicsSystem;
class Options;

class ResourceSystem
{
	std::unique_ptr<ShaderManager> _shaderManager;
	std::unique_ptr<PipelineManager> _pipelineManager;
	std::unique_ptr<PaletteManager> _paletteManager;

	std::unique_ptr<FontManager> _fontManager;
	std::unique_ptr<ImageManager> _imageManager;

public:
	ResourceSystem(VirtualFileSystem& virtualFileSystem, GraphicsSystem& graphicsSystem, Options& options);
	virtual ~ResourceSystem();

	ShaderManager& getShaderManager() { return *_shaderManager; };
	PipelineManager& getPipelineManager() { return *_pipelineManager; };
	PaletteManager& getPaletteManager() { return *_paletteManager; };

	FontManager& getFontManager() { return *_fontManager; };
	ImageManager& getImageManager() { return *_imageManager; };
};

} // namespace OpenXcom
