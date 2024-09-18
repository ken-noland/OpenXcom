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

TEST(OptionsTest, TestOptionsLoad)
{
	Options options;

	std::vector<std::string> argv;
	EXPECT_TRUE(options.load(argv));
}

TEST(OptionsTest, TestOptionsSave)
{
	Options options;

	std::vector<std::string> argv;
	options.load(argv);//ignore the output of load

	EXPECT_TRUE(options.save());
}

TEST(OptionsTest, TestOptionsVersion)
{
	Options options;

	std::vector<std::string> argv;
	argv.push_back("-version");

	EXPECT_TRUE(options.load(argv));

	EXPECT_FALSE(options.get<&GameOptions::_shouldRun>());
}

TEST(OptionsTest, TestOptionsCommandLineArgumentNotValid)
{
	Options options;

	std::vector<std::string> argv;
	argv.push_back("-thisIsNotAValidArgument");

	EXPECT_FALSE(options.load(argv));
}

TEST(OptionsTest, TestOptionsHelp)
{
	Options options;

	std::vector<std::string> argv;
	argv.push_back("-help");

	EXPECT_TRUE(options.load(argv));

	EXPECT_FALSE(options.get<&GameOptions::_shouldRun>());
}

TEST(OptionsTest, TestOptionsData)
{
	Options options;

	std::vector<std::string> argv;
	argv.push_back("-data");
	argv.push_back(".");

	EXPECT_TRUE(options.load(argv));
}

TEST(OptionsTest, TestOptionsUser)
{
	Options options;

	std::vector<std::string> argv;
	argv.push_back("-user");
	argv.push_back(".");

	EXPECT_TRUE(options.load(argv));
}

TEST(OptionsTest, TestOptionsCfg)
{
	Options options;

	std::vector<std::string> argv;
	argv.push_back("-cfg");
	argv.push_back(".");

	EXPECT_TRUE(options.load(argv));
}

TEST(OptionsTest, TestOptionsMaster)
{
	Options options;

	std::vector<std::string> argv;
	argv.push_back("-master");
	argv.push_back("xcom1");

	EXPECT_TRUE(options.load(argv));
}

TEST(OptionsTest, TestOptionsMasterNotValid)
{
	Options options;

	std::vector<std::string> argv;
	argv.push_back("-master");
	argv.push_back("not_xcom1_or_xcom2");

	EXPECT_FALSE(options.load(argv));
}

TEST(OptionsTest, TestOptionsMasterNotPresent)
{
	Options options;

	std::vector<std::string> argv;
	argv.push_back("-master");

	EXPECT_FALSE(options.load(argv));
}

TEST(OptionsTest, TestOptionsContinue)
{
	Options options;

	std::vector<std::string> argv;
	argv.push_back("-continue");

	EXPECT_TRUE(options.load(argv));
}
