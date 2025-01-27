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
#include "../../Utility/Delegate.h"

#include <memory>
#include <string>
#include <glm/mat4x4.hpp>

namespace OpenXcom
{

class Options;
class GameSurface;
class PlatformWindow;
class GraphicsSurface;
class Shader;
class DeviceBuffer;
class Pipeline;
class PipelineBinding;
class GraphicsCommand;

class WindowSurface
{
protected:
	GameSurface& _gameSurface;

	std::unique_ptr<PlatformWindow> _window;
	std::unique_ptr<GraphicsSurface> _windowSurface;

	std::unique_ptr<Pipeline> _pipeline;
	std::unique_ptr<PipelineBinding> _pipelineBinding;
	glm::mat4 _projection;

	std::unique_ptr<Shader> _vertexShader;
	std::unique_ptr<Shader> _fragmentShader;

	std::unique_ptr<DeviceBuffer> _vertexBuffer;
	std::unique_ptr<DeviceBuffer> _indexBuffer;

	bool _isRunning;

	// events
	MulticastDelegate<void(GraphicsCommand&)> _onRender;

	void updateProjection();
	
	void onResize();
	void onClose();

public:
	WindowSurface(const std::string& title, Options& options, GameSurface& gameSurface);
	~WindowSurface();

	void update();

	bool isRunning() const { return _isRunning; }

	MulticastDelegate<void(GraphicsCommand&)>& onRender() { return _onRender; }
};

} // namespace OpenXcom
