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

#include "../../Engine/Mod/ModInfo.h"
#include "../../Engine/Yaml.h"
#include "../../Engine/YamlContext.h"
#include "../../Engine/YamlException.h"

using namespace OpenXcom;

void yaml_error(const char* msg, size_t msg_len, c4::yml::Location location, void* user_data)
{
	std::string message(msg, msg_len);
	FAIL() << "Error: " << message << std::endl;
}

TEST(TestModInfo, TestBasicParse)
{
	// Test basic parsing of modinfo
	std::string modinfo = R"(
name: "My Mod"
id: "my_mod"
description: "does some cool stuff"
author: "John Doe"
version: "1.0.0"
dependencies:
  - mod: "Another Mod"
    version: ">=1.0.0"
  - mod: "Yet Another Mod"
    version: "==1.0.0"
)";

	// Parse the modinfo
	ModInfo info;

	ryml::set_callbacks(ryml::Callbacks(nullptr,
		[](size_t len, void* hint, void* user_data) { return malloc(len); },
		[](void* mem, size_t size, void* user_data) { free(mem); },
		[](const char* msg, size_t msg_len, c4::yml::Location location, void *user_data) { yaml_error(msg, msg_len, location, user_data); }));

	ryml::EventHandlerTree eventHandler = {};

	ryml::Parser parser = ryml::Parser(&eventHandler, ryml::ParserOptions().locations(true));
	ryml::Tree tree = ryml::parse_in_arena(&parser, "test.yml", modinfo.c_str());
	YamlContext context(parser);

	ASSERT_TRUE(fromYaml(tree, info, context));
	
	// Check the modinfo
	EXPECT_EQ(info.name, "My Mod");
	EXPECT_EQ(info.version, semver::version(1, 0, 0));
	ASSERT_EQ(info.dependencies.size(), 2);
	EXPECT_EQ(info.dependencies[0].mod, "Another Mod");
	ASSERT_EQ(info.dependencies[0].constraints.size(), 1);
	EXPECT_EQ(info.dependencies[0].constraints[0].op, VersionOperator::GreaterThanEqual);
	EXPECT_EQ(info.dependencies[0].constraints[0].version, semver::version(1, 0, 0));
	EXPECT_EQ(info.dependencies[1].mod, "Yet Another Mod");
	ASSERT_EQ(info.dependencies[1].constraints.size(), 1);
	EXPECT_EQ(info.dependencies[1].constraints[0].op, VersionOperator::Equal);
	EXPECT_EQ(info.dependencies[1].constraints[0].version, semver::version(1, 0, 0));
}

TEST(TestModInfo, TestMissingName)
{
	// Test basic parsing of modinfo
	std::string modinfo = R"(
id: "my_mod"
description: "does some cool stuff"
author: "John Doe"
version: "1.0.0"
dependencies:
  - mod: "Another Mod"
    version: ">=1.0.0"
  - mod: "Yet Another Mod"
    version: "==1.0.0"
)";

	// Parse the modinfo
	ModInfo info;

	c4::yml::pfn_allocate allocFunc = [](size_t len, void* hint, void* user_data) { return malloc(len); };
	c4::yml::pfn_free freeFunc = [](void* mem, size_t size, void* user_data) { free(mem); };
	c4::yml::pfn_error errorFunc = [](const char* msg, size_t msg_len, c4::yml::Location location, void* user_data) { yaml_error(msg, msg_len, location, user_data); };
	ryml::set_callbacks(ryml::Callbacks(nullptr, allocFunc, freeFunc, errorFunc));

	ryml::EventHandlerTree eventHandler = {};

	ryml::Parser parser = ryml::Parser(&eventHandler, ryml::ParserOptions().locations(true));
	ryml::Tree tree = ryml::parse_in_arena(&parser, "test.yml", modinfo.c_str());
	YamlContext context(parser);

	EXPECT_THROW({
		fromYaml(tree, info, context);
	}, YamlException);
}

