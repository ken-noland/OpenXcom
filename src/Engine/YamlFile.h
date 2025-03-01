#pragma once
/*
 * Copyright 2024-2024 OpenXcom Developers.
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
#include "YamlContext.h"
#include "Yaml.h"
#include <ryml.hpp>

namespace OpenXcom
{

class YamlFile
{
	ryml::EventHandlerTree eventHandler = {};
	ryml::Parser parser = ryml::Parser(&eventHandler, ryml::ParserOptions().locations(true));

public:
	YamlFile() = default;

	YamlFile(const YamlFile&) = delete;
	YamlFile& operator=(const YamlFile&) = delete;

	template <typename T>
	T load(std::unique_ptr<FileEntry>& fileEntry)
	{
		std::unique_ptr<std::istream> fileStream = fileEntry->openRead();
		std::string contents((std::istreambuf_iterator<char>(*fileStream)), std::istreambuf_iterator<char>());

		ryml::Tree tree = ryml::parse_in_arena(&parser, fileEntry->getPath().string().c_str(), contents.c_str());
		YamlContext context(parser);

		T result{};
		fromYaml(tree.rootref(), result, context);

		return result;
	}
};
	
} // namespace OpenXcom
