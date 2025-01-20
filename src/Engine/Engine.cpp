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

#include "Engine.h"
#include "Options.h"
#include "../Game/Game.h"
#include "Filesystem/VirtualFileSystem.h"
#include "Platform/WindowSystem.h"
#include "Platform/ProcessSystem.h"
#include "Graphics/GraphicsSystem.h"
#include "Resource/ResourceSystem.h"
#include "../version.h"

#include <simplerttr.h>

namespace OpenXcom
{

// singleton access for the engine
Engine* theEngine = nullptr;

Engine::Engine(const std::vector<std::string>& args)
{
	if(theEngine != nullptr)
	{
		throw std::runtime_error("Engine already exists. Only one instance of Engine per process.");
	}

	theEngine = this;

	// Initialize the options
	_options = std::make_unique<Options>(args);

	// Initialize the virtual file system
	_virtualFileSystem = std::make_unique<VirtualFileSystem>(getOptions());

	// Initialize the process system
	_platformProcessSystem = std::make_unique<PlatformProcessSystem>();

	// Initialize the window system
	_platformWindowSystem = std::make_unique<PlatformWindowSystem>();

	// Initialize the graphics system
	_graphicsSystem = createGraphicsSystem(getOptions());
		
	// Initialize the resource system
	_resourceSystem = std::make_unique<ResourceSystem>(*_virtualFileSystem, *_graphicsSystem, *_options);


	std::ostringstream title;
	title << "OpenXcom " << OPENXCOM_VERSION_SHORT << OPENXCOM_VERSION_GIT;

	// Initialize the game
	_game = std::make_unique<Game>(title.str());
}

Engine::~Engine()
{
	// shut down the game
	_game.reset();

	// shut down the resource system
	_resourceSystem.reset();

	// shut down window
	_platformWindowSystem.reset();

	// shut down graphics
	_graphicsSystem.reset();

	SimpleRTTR::shutdown();
}

int Engine::run()
{
	// some options (like -version or -help) don't need to run the game
	if (getOptions().get<&GameOptions::_shouldRun>() == false)
	{
		return EXIT_SUCCESS;
	}

	// run until the game is done
	while (_platformProcessSystem->isRunning() == true)
	{
		_platformProcessSystem->update();
		_platformWindowSystem->update();

		_game->update();
	}

	return EXIT_SUCCESS;
}

void Engine::exit()
{
	_platformProcessSystem->exit();
}

Engine& getEngine()
{
	return *theEngine;
}

} // namespace OpenXcom
