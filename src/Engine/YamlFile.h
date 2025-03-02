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
#include "YamlException.h"
#include <ryml.hpp>

namespace OpenXcom
{

inline void yaml_error(const char* msg, size_t msg_len, c4::yml::Location location, void* user_data)
{
	throw std::runtime_error(std::string(msg, msg_len));
}

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
		ryml::set_callbacks(ryml::Callbacks(nullptr,
			[](size_t len, void* hint, void* user_data) { return malloc(len); },
			[](void* mem, size_t size, void* user_data) { free(mem); },
			[](const char* msg, size_t msg_len, c4::yml::Location location, void* user_data) { yaml_error(msg, msg_len, location, user_data); }
		));

		std::unique_ptr<std::istream> fileStream = fileEntry->openRead();
		std::string contents((std::istreambuf_iterator<char>(*fileStream)), std::istreambuf_iterator<char>());

		std::string filenameStdStr = fileEntry->getPath().string();
		c4::csubstr filename = c4::csubstr(filenameStdStr.c_str(), filenameStdStr.size());

		ryml::Tree tree = ryml::parse_in_arena(&parser, filename, contents.c_str());
		YamlContext context(parser);

		T result{};
		fromYaml(tree.rootref(), result, context);

		return result;
	}
};
	
} // namespace OpenXcom
