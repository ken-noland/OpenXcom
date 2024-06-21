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
#include "../../Engine/Surface.h"
#include <entt/entt.hpp>

namespace OpenXcom
{

class DrawableComponent;

/// The SurfaceComponent class will eventually be the raw data model, but for now I am
/// using it as a temporary placeholder for the Surface class.
class SurfaceComponent
{
protected:
	// temp
	std::unique_ptr<Surface> _surface;

public:
	SurfaceComponent(DrawableComponent& drawable, std::unique_ptr<Surface>& surface);
	~SurfaceComponent() = default;

	// temp
	Surface* getSurface() { return _surface.get(); }

	void blit();

};

class SurfaceFactory
{
protected:
	entt::registry& _registry;
public:
	SurfaceFactory(entt::registry& registry); // KN NOTE: I do plan on passing the renderer here so that Surface is abstracted.
	~SurfaceFactory();

	entt::entity createSurface(const std::string& name, int width, int height, int x = 0, int y = 0);
};

} // namespace OpenXcom
