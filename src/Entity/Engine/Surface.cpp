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

entt::handle SurfaceFactory::createSurface(const std::string& name, int width, int height, int x, int y)
{
	entt::entity entity = _registry.create();
	_registry.emplace<Name>(entity, name);

	DrawableComponent& drawableComponent = _registry.emplace<DrawableComponent>(entity);

	std::unique_ptr<Surface> surface = std::make_unique<Surface>(width, height, x, y);
	SurfaceComponent& surfaceComponent = _registry.emplace<SurfaceComponent>(entity, surface);
	PaletteComponent& paletteComponent = _registry.emplace<PaletteComponent>(entity, surfaceComponent.getSurface());

	// TODO: we need to remove this setSurfaceComponent hack!
	drawableComponent.setSurfaceComponent(&surfaceComponent);
	drawableComponent.addDrawable(std::bind(&SurfaceComponent::blit, &surfaceComponent));

	return entt::handle(_registry, entity);
}

entt::handle SurfaceFactory::createInteractiveSurface(const std::string& name, int width, int height, int x, int y)
{
	entt::handle entity = createSurface(name, width, height, x, y);

	return entity;
}

} // namespace OpenXcom
