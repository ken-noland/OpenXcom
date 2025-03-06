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
#include "../../Resource/Handle.h"

#include <memory>
#include <string>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace OpenXcom
{

class EngineContext;
class GameSurface;
class PlatformWindow;
class RenderTarget;
class GraphicsSurface;
class Shader;
class DeviceBuffer;
class Pipeline;
class PipelineBinding;
class GraphicsCommand;

class WindowSurface
{
protected:
	EngineContext& _engine;	//TODO: GameContext would be more appropriate here

	GameSurface& _gameSurface;

	OwningHandle<PlatformWindow> _window;
	std::unique_ptr<GraphicsSurface> _windowSurface;

	MulticastDelegate<void()>::Handle _onClose;
	MulticastDelegate<void(glm::ivec2)>::Handle _onResize;

	std::unique_ptr<Pipeline> _pipeline;
	std::unique_ptr<PipelineBinding> _pipelineBinding;
	glm::mat4 _projection;

	std::unique_ptr<Shader> _vertexShader;
	std::unique_ptr<Shader> _fragmentShader;

	std::unique_ptr<DeviceBuffer> _vertexBuffer;
	std::unique_ptr<DeviceBuffer> _indexBuffer;

	bool _isRunning;
	bool _isHeadless;

	// events
	MulticastDelegate<void(GraphicsCommand&)> _onRender;

	void createWindowed(GameSurface& gameSurface);
	void createHeadless();

	void updateProjection();
	
	void onResize(glm::ivec2 size);
	void onClose();

public:
	WindowSurface(EngineContext& engine, GameSurface& gameSurface);
	~WindowSurface();

	void update();

	bool isRunning() const { return _isRunning; }
		
	RenderTarget& getRenderTarget();
	PlatformWindow& getWindow();

	//(re)binds the game surface to the pipeline
	void bindGameSurface(GameSurface& gameSurface);

	MulticastDelegate<void(GraphicsCommand&)>& onRender() { return _onRender; }
};

} // namespace OpenXcom
