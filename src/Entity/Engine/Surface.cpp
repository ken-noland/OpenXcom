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

int SurfaceComponent::getX() const
{
	return _surface->getX();
}

int SurfaceComponent::getY() const
{
	return _surface->getY();
}

int SurfaceComponent::getWidth() const
{
	return _surface->getWidth();
}

int SurfaceComponent::getHeight() const
{
	return _surface->getHeight();
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

entt::handle SurfaceFactory::createSurface(const std::string& name, int x, int y, int width, int height, SDL_Color* palette, int firstColor, int nColors)
{
	entt::entity entity = _registry.create();
	_registry.emplace<Name>(entity, name);

	//this is mostly a hack until I get surface converted to a pure component
	std::unique_ptr<Surface> surface = std::make_unique<Surface>(width, height, x, y);
	SurfaceComponent& surfaceComponent = _registry.emplace<SurfaceComponent>(entity, surface);

	PaletteComponent& paletteComponent = _registry.emplace<PaletteComponent>(entity, surfaceComponent.getSurface());
	if (palette)
	{
		paletteComponent.setPalette(palette, firstColor, nColors);
	}

	// KN NOTE: DrawableComponent is going away... soon
	DrawableComponent& drawableComponent = _registry.emplace<DrawableComponent>(entity);
//	drawableComponent.setSurfaceComponent(&surfaceComponent);
	drawableComponent.addDrawable(std::bind(&SurfaceComponent::blit, &surfaceComponent));


	return entt::handle(_registry, entity);
}

entt::handle SurfaceFactory::createInteractiveSurface(const std::string& name, int width, int height, int x, int y, SDL_Color* palette, int firstColor, int nColors)
{
	entt::handle entity = createSurface(name, width, height, x, y, palette, firstColor, nColors);

	return entity;
}

} // namespace OpenXcom
