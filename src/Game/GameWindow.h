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

/////////////////////////////////////////////
// TEMP

/////////////////////////////////////////////


namespace OpenXcom
{

class Options;
class GameSurface;
class WindowSurface;
class GraphicsCommand;

/////////////////////////////////////////////
// TEMP
class Pipeline;
class PipelineBinding;
class Shader;
class DeviceBuffer;
/////////////////////////////////////////////


class GameWindow
{
protected:
	std::unique_ptr<GameSurface> _gameSurface;
	std::unique_ptr<WindowSurface> _windowSurface;

	/////////////////////////////////////////////
	// TEMP
	std::unique_ptr<Pipeline> _pipeline;
	std::unique_ptr<PipelineBinding> _pipelineBinding;

	std::unique_ptr<Shader> _vertexShader;
	std::unique_ptr<Shader> _fragmentShader;

	std::unique_ptr<DeviceBuffer> _vertexBuffer;
	std::unique_ptr<DeviceBuffer> _indexBuffer;

	/////////////////////////////////////////////

	void onWindowRender(GraphicsCommand& command);
	void onGameRender(GraphicsCommand& command);

public:
	GameWindow(const std::string& title, Options& options);
	~GameWindow();

	void update();

	bool isRunning() const;
};

} // namespace OpenXcom
