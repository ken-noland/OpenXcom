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
#include "EngineContext.h"
#include <memory>
#include <vector>
#include <string>

namespace OpenXcom
{

class EngineContext;
class Options;
class VirtualFileSystem;
class PlatformProcessSystem;
class GraphicsSystem;
class ResourceSystem;

// Engine is used for process wide initialization and cleanup. In theory, it
// provides a way to separate and isolate the game from the rest of the
// systems, allowing for easier testing and debugging.
class Engine
{
protected:
	// Options
	std::unique_ptr<Options> _options;

	// Virtual File System
	std::unique_ptr<VirtualFileSystem> _virtualFileSystem;

	// Platform Process System
	std::unique_ptr<PlatformProcessSystem> _platformProcessSystem;

	// Platform Window System
	std::unique_ptr<PlatformWindowSystem> _platformWindowSystem;

	// Graphics system
	std::unique_ptr<GraphicsSystem> _graphicsSystem;

	// Resource system
	std::unique_ptr<ResourceSystem> _resourceSystem;

	// Time system
	std::unique_ptr<TimeSystem> _timeSystem;

	// Engine context
	std::unique_ptr<EngineContext> _engineContext;

	// Empty constructor for unit tests
	Engine();

public:
	Engine(const std::vector<std::string>& args);
	~Engine();

	void update();

	// call this to exit the application
	void exit();

	EngineContext& getEngineContext() { return *_engineContext; }

	Options& getOptions() { return *_options; }
	VirtualFileSystem& getVirtualFileSystem() { return *_virtualFileSystem; }
	PlatformProcessSystem& getPlatformProcessSystem() { return *_platformProcessSystem; }
	GraphicsSystem& getGraphicsSystem() { return *_graphicsSystem; }
	ResourceSystem& getResourceSystem() { return *_resourceSystem; }
	TimeSystem& getTimeSystem() { return *_timeSystem; }
};

} // namespace OpenXcom
