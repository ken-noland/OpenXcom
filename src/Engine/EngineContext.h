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
#include <string>

namespace OpenXcom
{

class Engine;
class Options;
class VirtualFileSystem;
class PlatformProcessSystem;
class PlatformWindowSystem;
class GraphicsSystem;
class ResourceSystem;
class TimeSystem;

class EngineContext
{
protected:
	Engine& _engine;

	Options* _options;
	VirtualFileSystem* _virtualFileSystem;
	PlatformProcessSystem* _platformProcessSystem;
	PlatformWindowSystem* _platformWindowSystem;
	GraphicsSystem* _graphicsSystem;
	ResourceSystem* _resourceSystem;
	TimeSystem* _timeSystem;

	std::string _title;

public:
	EngineContext(Engine& engine)
		: _engine(engine)
	{
	}

	void setTitle(const std::string& title) { _title = title; }
	const std::string& getTitle() const { return _title; }

	Engine& getEngine() { return _engine; }

	void setOptions(Options* options) { _options = options; }
	Options& getOptions() { return *_options; }

	void setVirtualFileSystem(VirtualFileSystem* virtualFileSystem) { _virtualFileSystem = virtualFileSystem; }
	VirtualFileSystem& getVirtualFileSystem() { return *_virtualFileSystem; }

	void setPlatformProcessSystem(PlatformProcessSystem* platformProcessSystem) { _platformProcessSystem = platformProcessSystem; }
	PlatformProcessSystem& getPlatformProcessSystem() { return *_platformProcessSystem; }

	void setPlatformWindowSystem(PlatformWindowSystem* platformWindowSystem) { _platformWindowSystem = platformWindowSystem; }
	PlatformWindowSystem& getPlatformWindowSystem() { return *_platformWindowSystem; }

	void setGraphicsSystem(GraphicsSystem* graphicsSystem) { _graphicsSystem = graphicsSystem; }
	GraphicsSystem& getGraphicsSystem() { return *_graphicsSystem; }

	void setResourceSystem(ResourceSystem* resourceSystem) { _resourceSystem = resourceSystem; }
	ResourceSystem& getResourceSystem() { return *_resourceSystem; }

	void setTimeSystem(TimeSystem* timeSystem) { _timeSystem = timeSystem; }
	TimeSystem& getTimeSystem() { return *_timeSystem; }
};


} // namespace OpenXcom
