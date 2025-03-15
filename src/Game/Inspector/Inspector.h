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
#include <glm/mat4x4.hpp>
#include "../../Engine/Resource/Handle.h"

namespace OpenXcom
{

class GameContext;

class PlatformWindow;
class GraphicsSurface;

class HostBuffer;
class DeviceBuffer;

class DeviceImage;

class Shader;
class Pipeline;
class PipelineBinding;

class Inspector
{
protected:
	GameContext& _game;

	OwningHandle<PlatformWindow> _window;
	std::unique_ptr<GraphicsSurface> _windowSurface;

	OwningHandle<DeviceImage> _font;

	std::unique_ptr<Shader> _vertexShader;
	std::unique_ptr<Shader> _fragmentShader;

	std::unique_ptr<Pipeline> _pipeline;
	std::unique_ptr<PipelineBinding> _pipelineBinding;
	glm::mat4 _projection;

	std::unique_ptr<HostBuffer> _hostVertexBuffer;
	std::unique_ptr<HostBuffer> _hostIndexBuffer;

	std::unique_ptr<DeviceBuffer> _deviceVertexBuffer;
	std::unique_ptr<DeviceBuffer> _deviceIndexBuffer;


public:
	Inspector(GameContext& game);
	~Inspector();

	void show();
	void hide();

	void update();
};

} // namespace OpenXcom
