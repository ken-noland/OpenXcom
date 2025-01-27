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
#include <string>
#include <glm/mat4x4.hpp>

namespace OpenXcom
{

class PlatformWindow;
class GraphicsSurface;
class Pipeline;
class PipelineBinding;
class DeviceBuffer;
class RenderTarget;
class Shader;

class GameWindow
{
private:
	std::unique_ptr<PlatformWindow> _window;
	std::unique_ptr<GraphicsSurface> _windowSurface;

	std::unique_ptr<Pipeline> _windowPipeline;
	std::unique_ptr<PipelineBinding> _windowPipelineBinding;
	glm::mat4 _windowProjection;

	std::unique_ptr<DeviceBuffer> _vertexBuffer;
	std::unique_ptr<DeviceBuffer> _indexBuffer;

	std::unique_ptr<RenderTarget> _gameSurface;

	std::unique_ptr<Shader> _vertexShader;
	std::unique_ptr<Shader> _fragmentShader;

	bool _isRunning;

	void updateProjection();

	void onClose();
	void onResize();

public:
	GameWindow(const std::string& title);
	~GameWindow();

	void update();

	bool isRunning() const { return _isRunning; }
};

} // namespace OpenXcom
