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
#include <filesystem>

#include "../../Engine/Engine.h"

using namespace OpenXcom;

TEST(TestEngine, TestEngineStart)
{
	std::filesystem::path path = TEST_DATA_DIR;
	std::filesystem::path dataPath = path / "Data";
	std::filesystem::path configPath = path / "Config";
	std::filesystem::path userPath = path / "User";

	std::vector<std::string> args = {"-data", dataPath.string(), "-config", configPath.string(), "-user", userPath.string(), "-headless"};
	Engine engine(args);
}
