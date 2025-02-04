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
#include "BoxPrimitiveFactory.h"
#include "LinePrimitiveFactory.h"
#include "PointPrimitiveFactory.h"
#include <memory>

namespace OpenXcom
{

class Shader;

// TODO: This class is mostly temporary because I want an object to hold the default
// shaders without having to implement a huge shader manager
class ShaderCollection
{
protected:
	EngineContext& _context;

	// vertex shaders
	std::unique_ptr<Shader> _defaultVec2Shader;
	std::unique_ptr<Shader> _defaultVec2ColorShader;
	std::unique_ptr<Shader> _defaultVec2UvShader;

	std::unique_ptr<Shader> _defaultFragmentShader;
	std::unique_ptr<Shader> _defaultUVFragmentShader;

public:
	ShaderCollection(EngineContext& context);
	~ShaderCollection();

	Shader& getDefaultVec2VertexShader();
	Shader& getDefaultVec2ColorVertexShader();
	Shader& getDefaultVec2UVVertexShader();

	Shader& getDefaultFragmentShader();
	Shader& getDefaultUVFragmentShader();
};

} // namespace OpenXcom
