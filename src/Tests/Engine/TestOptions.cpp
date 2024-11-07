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

#include "../../Engine/Options.h"

using namespace OpenXcom;

std::vector<std::string> getTestArgv()
{
	std::vector<std::string> argv;

	std::filesystem::path test_path = std::filesystem::path(TEST_DATA_DIR) / "Config";
	argv.push_back("-config");
	argv.push_back(test_path.string());

	return argv;
}

TEST(OptionsTest, TestOptionsLoad)
{
	std::vector<std::string> argv = getTestArgv();
	Options options(argv);
}

TEST(OptionsTest, TestOptionsSave)
{
	std::vector<std::string> argv = getTestArgv();

	Options options(argv);

	EXPECT_TRUE(options.save());
}

TEST(OptionsTest, TestOptionsVersion)
{
	std::vector<std::string> argv = getTestArgv();
	argv.push_back("-version");

	Options options(argv);

	EXPECT_FALSE(options.get<&GameOptions::_shouldRun>());
}

TEST(OptionsTest, TestOptionsCommandLineArgumentNotValid)
{
	std::vector<std::string> argv = getTestArgv();
	argv.push_back("-thisIsNotAValidArgument");

	EXPECT_ANY_THROW(Options options(argv));
}

TEST(OptionsTest, TestOptionsHelp)
{
	std::vector<std::string> argv = getTestArgv();
	argv.push_back("-help");

	Options options(argv);

	EXPECT_FALSE(options.get<&GameOptions::_shouldRun>());
}

TEST(OptionsTest, TestOptionsData)
{
	std::vector<std::string> argv = getTestArgv();
	argv.push_back("-data");
	argv.push_back(".");

	Options options(argv);

	EXPECT_EQ(options.get<&GameOptions::_dataPath>().size(), 1);
	EXPECT_EQ(options.get<&GameOptions::_dataPath>()[0], ".");
}

TEST(OptionsTest, TestOptionsUser)
{
	std::vector<std::string> argv = getTestArgv();

	argv.push_back("-user");
	argv.push_back(".");

	Options options(argv);

	EXPECT_EQ(options.get<&GameOptions::_userPath>(), ".");
}

TEST(OptionsTest, TestOptionsMaster)
{
	std::vector<std::string> argv = getTestArgv();

	argv.push_back("-master");
	argv.push_back("xcom1");

	Options options(argv);
}

TEST(OptionsTest, TestOptionsMasterNotValid)
{
	std::vector<std::string> argv = getTestArgv();

	argv.push_back("-master");
	argv.push_back("not_xcom1_or_xcom2");
		
	EXPECT_ANY_THROW(Options options(argv));
}

TEST(OptionsTest, TestOptionsMasterNotPresent)
{
	std::vector<std::string> argv = getTestArgv();
	argv.push_back("-master");

	EXPECT_ANY_THROW(Options options(argv));
}
