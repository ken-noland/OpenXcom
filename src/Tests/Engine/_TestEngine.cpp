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
#include "_TestEngine.h"

std::unique_ptr<OpenXcom::Engine> TestEngineSuite::_engine(nullptr);

std::filesystem::path TestEngineSuite::_dataPath;
std::filesystem::path TestEngineSuite::_configPath;
std::filesystem::path TestEngineSuite::_userPath;

std::unique_ptr<OpenXcom::GameSurface> TestEngineSuite::_gameSurface(nullptr);
std::unique_ptr<OpenXcom::WindowSurface> TestEngineSuite::_windowSurface(nullptr);
