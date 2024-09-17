#include "Options.h"
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
#include "Options.h"
#include "Logger.h"
#include "../version.h"

#include <sstream>
#include <iostream>

namespace OpenXcom
{




Options::Options()
{
	_commandLineOptions = {
		{{"-version"}, {
			false, "",
			"Display the version information.",
			[this](const std::string&) -> bool {
				// since the version information is requested, we can skip loading the game and just display the version information
				showVersion();
				set<&GameOptions::_shouldRun>(OptionLevel::COMMAND, false);
				return true;
			}
		}},
		{{"-help", "-?"}, {
			false, "",
			"Show command line help.",
			[this](const std::string&) -> bool {
				// since the help information is requested, we can skip loading the game and just display the help information
				showHelp();
				set<&GameOptions::_shouldRun>(OptionLevel::COMMAND, false);
				return true;
			}
		}},
		{{"-data"}, {
			true, "PATH",
			"Use PATH as the default Data Folder. Bypasses the auto-detection.",
			[this](const std::string&) -> bool {
				return false;
			}
		}},
		{{"-user"}, {
			true, "PATH",
			"Use PATH as the default User Folder. Bypasses the auto-detection.",
			[this](const std::string&) -> bool {
				return false;
			}
		}},
		{{"-cfg", "-config"}, {
			true, "PATH",
			"Use PATH as the default Config Folder. Bypasses the auto-detection.",
			[this](const std::string&) -> bool {
				return false;
			}
		}},
		{{"-log"}, {
			true, "FILE",
			"Use FILE to output the log information",
			[this](const std::string&) -> bool {
				return false;
			}
		}},
		{{"-logLevel"}, {
			true, "LEVEL",
			"Set the log output level. Level can be FATAL, ERROR, WARN, INFO, DEBUG, VERB, or ALL",
			[this](const std::string&) -> bool {
				return false;
			}
		}},
		{{"-continue"}, {
			false, "",
			"Load last saved game",
			[this](const std::string&) -> bool {
				return false;
			}
		}},
		{{"-save"}, {
			true, "FILE",
			"Load the save file specified. This will load the file specified regardless if -continue is specified.",
			[this](const std::string&) -> bool {
				return false;
			}
		}},
		{{"-lang", "-language"}, {
			true, "LANGUAGE",
			"Override the default language. Use LIST as a parameter to this option to see all available language options.",
			[this](const std::string&) -> bool {
				return false;
			}
		}}
	};
}

Options::~Options()
{
}

void Options::showVersion()
{
}

void Options::showHelp()
{
	std::ostringstream help;
	help << "OpenXcom " << OPENXCOM_VERSION_SHORT << " - " << "Open-source reimplementation of the original X-Com game" << std::endl << std::endl;
	help << "Usage: openxcom [OPTION]..." << std::endl << std::endl;

	for (OptionsMap::iterator it = _commandLineOptions.begin(); it != _commandLineOptions.end(); ++it)
	{
		const CommandLineOption& option = it->second;
		for (const std::string& key : it->first)
		{
			help << "  " << key << " ";
			if (option.hasParameter)
			{
				help << "{" << option.parameterName << "} ";
			}
			help << std::endl;
		}
		help << "    " << option.description << std::endl;
	}

	std::cout << help.str();
}

bool Options::loadCommandLine(const std::vector<std::string>& argv) {


	std::vector<std::string>::const_iterator argumentsIter = argv.begin();
	while (argumentsIter != argv.end())
	{
		const std::string& argument = *argumentsIter;

		if (argument == "--")
		{
			break;
		}

		OptionsMap::iterator optionsIterator = std::find_if(_commandLineOptions.begin(), _commandLineOptions.end(), [argument](const OptionsMap::value_type& v) {
			return std::find(v.first.begin(), v.first.end(), argument) != v.first.end();
		});

		if (optionsIterator == _commandLineOptions.end())
		{
			Log(LOG_ERROR) << "Unknown command line argument \"" + argument + "\".";
			return false;
		}

		const CommandLineOption& option = optionsIterator->second;
		if (option.hasParameter)
		{
			++argumentsIter;
			const std::string& parameter = *argumentsIter;
			if (argumentsIter == argv.end())
			{
				Log(LOG_ERROR) << "Missing parameter for command line argument \"" + argument + "\".";
				return false;
			}
			if (!option.parser(parameter))
			{
				Log(LOG_ERROR) << "Invalid parameter for command line argument \"" + argument + "\".";
				return false;
			}
		}
		else
		{
			if (!option.parser(""))
			{
				Log(LOG_ERROR) << "Invalid parameter for command line argument \"" + argument + "\".";
				return false;
			}
		}

		++argumentsIter;
	}

	return false;
}

/// Load options from file.
bool Options::load(const std::vector<std::string>& argv)
{
	// first, load up the arguments from the command line
	if (!loadCommandLine(argv)) { return false; }

	// then, load up the options from the config file



	return true;
}

/// Save options to file.
void Options::save()
{
	// we ignore command line options for saving the config file

}

/// Reset options to default values.
void Options::reset()
{
}

}
