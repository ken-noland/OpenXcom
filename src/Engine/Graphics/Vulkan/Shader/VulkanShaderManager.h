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
#include "../../ShaderManager.h"
#include "VulkanShader.h"

namespace OpenXcom
{

class VulkanContext;

class VulkanShaderManager : public ShaderManager
{
protected:
	VulkanContext& _context;

	std::unique_ptr<shaderc::Compiler> _compiler;

	std::vector<uint32_t> compileGLSL(const std::string& name, const std::string& source, ShaderType type);

public:
	VulkanShaderManager(VulkanContext& context);
	virtual ~VulkanShaderManager();

	// load shader from memory
	virtual std::unique_ptr<Shader> loadShaderFromMemory(const std::string& name, const std::string shader, ShaderType type = ShaderType::InferFromSource) override;

	// load shader from file
	virtual std::unique_ptr<Shader> loadShaderFromFile(const std::string& name, const std::filesystem::path& path, ShaderType type = ShaderType::InferFromSource) override;
};

} // namespace OpenXcom
