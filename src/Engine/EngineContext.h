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
class GraphicsSystem;
class ResourceSystem;

class EngineContext
{
protected:
	Engine& _engine;

	Options& _options;
	VirtualFileSystem& _virtualFileSystem;
	PlatformProcessSystem& _platformProcessSystem;
	GraphicsSystem& _graphicsSystem;
	ResourceSystem& _resourceSystem;

	std::string _title;

public:
	EngineContext(Engine& engine,
				  Options& options,
				  VirtualFileSystem& virtualFileSystem,
				  PlatformProcessSystem& platformProcessSystem,
				  GraphicsSystem& graphicsSystem,
				  ResourceSystem& resourceSystem)
		: _engine(engine),
		  _options(options),
		  _virtualFileSystem(virtualFileSystem),
		  _platformProcessSystem(platformProcessSystem),
		  _graphicsSystem(graphicsSystem),
		  _resourceSystem(resourceSystem)
	{
	}

	void setTitle(const std::string& title) { _title = title; }
	const std::string& getTitle() const { return _title; }

	Engine& getEngine() { return _engine; }

	Options& getOptions() { return _options; }
	VirtualFileSystem& getVirtualFileSystem() { return _virtualFileSystem; }
	GraphicsSystem& getGraphicsSystem() { return _graphicsSystem; }
	ResourceSystem& getResourceSystem() { return _resourceSystem; }
};


} // namespace OpenXcom
