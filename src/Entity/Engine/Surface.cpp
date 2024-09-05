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
#include "Surface.h"
#include "Palette.h"
#include "Drawable.h"
#include "../Common/Name.h"
#include "../../Engine/Game.h"
#include "../../Engine/Screen.h"

namespace OpenXcom
{

SurfaceComponent::SurfaceComponent(std::unique_ptr<Surface>& surface)
	: _surface(std::move(surface))
{
}

void SurfaceComponent::blit()
{
	_surface->blit(getGame()->getScreen()->getSurface());
}

SurfaceFactory::SurfaceFactory(entt::registry& registry)
	: _registry(registry)
{
}

SurfaceFactory::~SurfaceFactory()
{
}

entt::handle SurfaceFactory::createSurface(const std::string& name, int x, int y, int width, int height, PaletteHandle palette)
{
	entt::handle entity = entt::handle(_registry, _registry.create());
	entity.emplace<Name>(name);

	//this is mostly a hack until I get surface converted to a pure component
	std::unique_ptr<Surface> surface = std::make_unique<Surface>(width, height, x, y);
	SurfaceComponent& surfaceComponent = entity.emplace<SurfaceComponent>(surface);
	entity.emplace<PaletteComponent>(surfaceComponent.getSurface(), palette);
	entity.emplace<ScreenRectComponent>(x, y, width, height);

	// KN NOTE: DrawableComponent is going away... soon
	DrawableComponent& drawableComponent = entity.emplace<DrawableComponent>();
	//	drawableComponent.setSurfaceComponent(&surfaceComponent);
	//drawableComponent.addDrawable(std::bind(&SurfaceComponent::blit, &surfaceComponent));


	return entt::handle(_registry, entity);
}

entt::handle SurfaceFactory::createInteractiveSurface(const std::string& name, int width, int height, int x, int y, PaletteHandle palette)
{
	entt::handle entity = createSurface(name, width, height, x, y, palette);

	return entity;
}

} // namespace OpenXcom
