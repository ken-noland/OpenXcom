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
#include "Platform/ProcessSystem.h"
#include "Graphics/GraphicsSystem.h"
#include "Resource/ResourceSystem.h"
#include "../version.h"

#include <simplerttr.h>

namespace OpenXcom
{

// annoyingly, I have to put this in somewhere so it correctly links the RTTR stuff
extern int FORCE_LINK_RTTRGLM;

Engine::Engine(const std::vector<std::string>& args)
{
	// hack to force the linker to include the RTTR stuff
	FORCE_LINK_RTTRGLM = 42; 

	// Initialize the options
	_options = std::make_unique<Options>(args);

	// Initialize the virtual file system
	_virtualFileSystem = std::make_unique<VirtualFileSystem>(*_options);

	// Initialize the process system
	_platformProcessSystem = std::make_unique<PlatformProcessSystem>();

	// Initialize the graphics system
	_graphicsSystem = createGraphicsSystem(*_options);
		
	// Initialize the resource system
	_resourceSystem = std::make_unique<ResourceSystem>(*_virtualFileSystem, *_graphicsSystem, *_options);

	//finally, now that the systems have all been initialized, let's make the engine context which is used to get access to the systems from within the game
	_engineContext = std::make_unique<EngineContext>(*this, *_options, *_virtualFileSystem, *_platformProcessSystem, *_graphicsSystem, *_resourceSystem);

	std::ostringstream title;
	title << "OpenXcom " << OPENXCOM_VERSION_SHORT << OPENXCOM_VERSION_GIT;
	_engineContext->setTitle(title.str());

	// Initialize the game
	_game = std::make_unique<Game>(*_engineContext);
}

Engine::~Engine()
{
	// shut down the game
	_game.reset();

	// shut down the resource system
	_resourceSystem.reset();

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
	while (_platformProcessSystem->isRunning() == true && _game->isRunning())
	{
		_platformProcessSystem->update();

		_game->update();
	}

	return EXIT_SUCCESS;
}

void Engine::exit()
{
	_platformProcessSystem->exit();
}

} // namespace OpenXcom
