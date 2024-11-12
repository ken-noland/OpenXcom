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
#include "VulkanShaderManager.h"

#include <shaderc/shaderc.hpp>
#include "../../../Logger.h"

namespace OpenXcom
{

shaderc_shader_kind getShadercKind(ShaderType type)
{
	switch (type)
	{
	case ShaderType::Vertex:
		return shaderc_glsl_vertex_shader;
	case ShaderType::Fragment:
		return shaderc_glsl_fragment_shader;
	case ShaderType::Geometry:
		return shaderc_glsl_geometry_shader;
	case ShaderType::Compute:
		return shaderc_glsl_compute_shader;
	case ShaderType::TessControl:
		return shaderc_glsl_tess_control_shader;
	case ShaderType::TessEvaluation:
		return shaderc_glsl_tess_evaluation_shader;
	case ShaderType::InferFromSource:
	default:
		return shaderc_glsl_infer_from_source;
	}
	return shaderc_shader_kind();
}

VulkanShaderManager::VulkanShaderManager(vk::Device& device)
	: _device(device)
{

#if defined(_DEBUG) && defined(_WIN32)
	// annoyingly, the shader compiler leaks a single std::mutex, so to avoid that being reported in Crt, I have
	// to disable memory checking for just this one part
	int oldFlags = _CrtSetDbgFlag(0);
#endif

	_compiler = std::make_unique<shaderc::Compiler>();

#if defined(_DEBUG) && defined(_WIN32)
	// and re-enable memory checking
	_CrtSetDbgFlag(oldFlags);
#endif

}

VulkanShaderManager::~VulkanShaderManager()
{
}

std::vector<uint32_t> VulkanShaderManager::compileGLSL(const std::string& source, ShaderType type)
{
	shaderc_shader_kind kind = getShadercKind(type);

	shaderc::CompileOptions options;
	options.SetOptimizationLevel(shaderc_optimization_level_size);

	shaderc::SpvCompilationResult result = _compiler->CompileGlslToSpv(source, kind, "shader", options);

	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		Log(LOG_ERROR) << "Failed to compile GLSL shader: " << result.GetErrorMessage();
		return {};
	}
	return {result.cbegin(), result.cend()};
}


VulkanShaderManager::Handle VulkanShaderManager::loadShaderFromMemory(const std::string& name, const std::string shader, ShaderType type)
{
	std::vector<uint32_t> spirv = compileGLSL(shader, type);
	std::unique_ptr<VulkanShader> vulkanShader = std::make_unique<VulkanShader>(name, _device, spirv);
	return add(std::move(vulkanShader));
}

VulkanShaderManager::Handle VulkanShaderManager::loadShaderFromFile(const std::string& name, const std::filesystem::path& path, ShaderType type)
{
	return Handle();
}

void VulkanShaderManager::clear()
{
	//just remove it all
	_resources.clear();
}


} // namespace OpenXcom
