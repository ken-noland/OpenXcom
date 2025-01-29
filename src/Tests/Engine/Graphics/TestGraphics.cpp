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
#include <gtest/gtest.h>

#include "../../../Engine/Engine.h"
#include "../../../Engine/Options.h"
#include "../../../Engine/Graphics/GraphicsSystem.h"
#include "../../../Engine/Graphics/GraphicsSurface.h"
#include "../../../Engine/Graphics/Common/GameSurface.h"
#include "../../../Engine/Graphics/Common/WindowSurface.h"
#include "../../../Engine/Platform/Window.h"

using namespace OpenXcom;

TEST(TestGraphics, TestGraphicsInitialization)
{
	Options options;

	options.set<&GraphicsOptions::_headless>(OptionLevel::CONFIG, true);
	std::unique_ptr<GraphicsSystem> graphicsSystem = createGraphicsSystem(options);
}

TEST(TestGraphics, TestGraphicsSurface)
{
	std::filesystem::path path = TEST_DATA_DIR;
	std::filesystem::path dataPath = path / "Data";
	std::filesystem::path configPath = path / "Config";
	std::filesystem::path userPath = path / "User";

	std::vector<std::string> args = {"-data", dataPath.string(), "-config", configPath.string(), "-user", userPath.string(), "-headless"};
	Engine engine(args);

	GameSurface gameSurface(engine.getEngineContext());
	WindowSurface windowSurface(engine.getEngineContext(), gameSurface);

	// render one frame

}
