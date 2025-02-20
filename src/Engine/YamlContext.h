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

#include "EngineContext.h"
#include "Filesystem/VirtualFileSystem.h"
#include <ryml.hpp>

namespace OpenXcom
{

class YamlContext
{
	ryml::EventHandlerTree evtentHandler = {};
	ryml::Parser parser = ryml::Parser(&evtentHandler, ryml::ParserOptions().locations(true));
	EngineContext& context;

public:
	YamlContext(EngineContext& ctx) : context(ctx) {}

	YamlContext(const YamlContext&) = delete;
	YamlContext& operator=(const YamlContext&) = delete;

	template <typename T>
	T LoadAndParse(const std::filesystem::path& yamlPath)
	{
		std::unique_ptr<FileEntry> file = context.getVirtualFileSystem().getDataFileSystem().getFile(yamlPath);

		if (!file)
		{
			throw std::runtime_error(std::format("FontPack: Could not find font file: {}", yamlPath.string()));
		}

		std::string contents(std::istreambuf_iterator<char>(*file->openRead()), {});

		ryml::Tree tree = ryml::parse_in_arena(&parser, yamlPath.string().c_str(), contents.c_str());

		T result{};
		fromYaml(tree.rootref(), result, parser);

		return result;
	}
};

}