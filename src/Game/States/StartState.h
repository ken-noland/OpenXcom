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
#include "State.h"
#include "../../Engine/Resource/Handle.h"
#include <string>
#include <sstream>

namespace OpenXcom
{

class GameContext;
class RenderTarget;
class Font;
class Palette;
class TextPrimitive;

class StartState : public State
{
protected:
	GameContext& _game;

	OwningHandle<Font> _dosFont;
	OwningHandle<Palette> _dosFontPalette;

	std::unique_ptr<TextPrimitive> _text;
	std::unique_ptr<TextPrimitive> _cursor;

	void createDosFont();

public:
	StartState(GameContext& game);
	virtual ~StartState();

	virtual void onRender(GraphicsCommand& command) override;
};

}
