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
extern int FORCE_LINK_RTTRPACKEDCOLOR;

Engine::Engine(const std::vector<std::string>& args)
{
	// hack to force the linker to include the RTTR stuff
	FORCE_LINK_RTTRGLM = 42;
	FORCE_LINK_RTTRPACKEDCOLOR = 42;

	_engineContext = std::make_unique<EngineContext>(*this);

	// Initialize the options
	_options = std::make_unique<Options>(args);
	_engineContext->setOptions(_options.get());

	// Initialize the virtual file system
	_virtualFileSystem = std::make_unique<VirtualFileSystem>(*_options);
	_engineContext->setVirtualFileSystem(_virtualFileSystem.get());

	// Initialize the process system
	_platformProcessSystem = std::make_unique<PlatformProcessSystem>();
	_engineContext->setPlatformProcessSystem(_platformProcessSystem.get());

	// Initialize the graphics system
	_graphicsSystem = createGraphicsSystem(*_engineContext);
	_engineContext->setGraphicsSystem(_graphicsSystem.get());
		
	// Initialize the resource system
	_resourceSystem = std::make_unique<ResourceSystem>(*_engineContext);
	_engineContext->setResourceSystem(_resourceSystem.get());

	std::ostringstream title;
	title << "OpenXcom " << OPENXCOM_VERSION_SHORT << OPENXCOM_VERSION_GIT;
	_engineContext->setTitle(title.str());
}

Engine::~Engine()
{
	// shut down the resource system
	_resourceSystem.reset();

	// shut down graphics
	_graphicsSystem.reset();
}

void Engine::update()
{
	_platformProcessSystem->update();
}

void Engine::exit()
{
	_platformProcessSystem->exit();
}

} // namespace OpenXcom
