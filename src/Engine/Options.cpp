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

#if defined(_WIN32)
#include <Windows.h>
#include <ShlObj_core.h>
#endif

#include "../Entity/Common/RTTR.h"

namespace OpenXcom
{

SIMPLERTTR
{
	SimpleRTTR::Registration().Type<GameOptions>()
		.Meta("Serialize", ObjectSerialize::ALWAYS) // special case for options, since we only want to serialize a particular layer of the options to disk.
		.Property(REGISTER_PROPERTY(GameOptions, _shouldRun))
		.Meta("FriendlyName", "shouldRun")
		.Meta("Description", "Determines if the game will run. Only for internal use.")
		.Property(REGISTER_PROPERTY(GameOptions, _dataPath))
		.Meta("FriendlyName", "dataPath")
		.Meta("Description", "A list of all the paths to search for data and mods.")
		.Property(REGISTER_PROPERTY(GameOptions, _userPath))
		.Meta("FriendlyName", "userPath")
		.Meta("Description", "The path to the user folder where save games will be stored.")
		.Property(REGISTER_PROPERTY(GameOptions, _cfgPath))
		.Meta("FriendlyName", "cfgPath")
		.Meta("Description", "The path to the config folder where the config files will be loaded from. Typically the same as \"userPath\".")
		.Property(REGISTER_PROPERTY(GameOptions, _locale))
		.Meta("FriendlyName", "locale")
		.Meta("Serialize", PropertySerialize::IF_SET) // serialize this option if the user sets it
		.Meta("Description", "The locale used for translations.")
		.Property(REGISTER_PROPERTY(GameOptions, _continueSave))
		.Meta("FriendlyName", "continueSave")
		.Meta("Description", "If the game should load the last save")
		.Property(REGISTER_PROPERTY(GameOptions, _lastSave))
		.Meta("FriendlyName", "lastSave")
		.Meta("Description", "The full path to the last save game.")
		.Property(REGISTER_PROPERTY(GameOptions, _logPath))
		.Meta("FriendlyName", "logPath")
		.Meta("Description", "The full path to the log file.")
		.Property(REGISTER_PROPERTY(GameOptions, _mods))
		.Meta("FriendlyName", "mods")
		.Meta("Description", "List of mods to load.")
		.Property(REGISTER_PROPERTY(GameOptions, _master))
		.Meta("FriendlyName", "master")
		.Meta("Serialize", PropertySerialize::ALWAYS) // always serialize this
		.Meta("Description", "Master game to use. Can be either xcom1 or xcom2");

	SimpleRTTR::Registration().Type<GraphicsOptions>().Meta("Serialize", ObjectSerialize::ALWAYS) // special case for options, since we only want to serialize a particular layer of the options to disk.
		.Property(REGISTER_PROPERTY(GraphicsOptions, _fullscreen))
		.Meta("FriendlyName", "fullscreen")
		.Meta("Serialize", PropertySerialize::ALWAYS) // always serialize this option
		.Meta("Description", "Should the game be fullscreen or not")
		.Property(REGISTER_PROPERTY(GraphicsOptions, _screenWidth))
		.Meta("FriendlyName", "screenWidth")
		.Meta("Serialize", PropertySerialize::ALWAYS) // always serialize this option
		.Meta("Description", "Width of the screen in pixels")
		.Property(REGISTER_PROPERTY(GraphicsOptions, _screenHeight))
		.Meta("FriendlyName", "screenHeight")
		.Meta("Serialize", PropertySerialize::ALWAYS) // always serialize this option
		.Meta("Description", "Height of the screen in pixels");

	// okay, this is going to look a bit strange, but since Options is a template class, and I don't want to have to go through
	//  and define each possible combination, I simply iterate on all types(they should have been pre-registered by inclusion
	//  as a parameter) and add in special metadata which signals the codegen to handle these types in a unique way
	for (const SimpleRTTR::Type& type : SimpleRTTR::Types())
	{
		if (type.Name() == "Option")
		{
			SimpleRTTR::Registration().Type(type)
				.Meta("Serialize", ObjectSerialize::ALWAYS) // always serialize this option
				.Meta("Description", "A generic option type. This should not be used directly.")
				.Meta("Codegen-Override-Template", "option_template");	// this tag tells the codegen that this is a special case for
																		//  serialization and to use the override template specified
																		//  in the template definition file
		}
	}
}

Options::Options()
{
	_commandLineOptions = {
		{{"-version"}, {
			false, "",
			"Display the version information.",
			[this](const std::string&) -> bool
			{
				// since the version information is requested, we can skip loading the game and just display the version information
				showVersion();
				set<&GameOptions::_shouldRun>(OptionLevel::COMMAND, false);
				return true;
			}
		}},
		{{"-help", "-?"}, {
			false, "",
			"Show command line help.",
			[this](const std::string&) -> bool
			{
				// since the help information is requested, we can skip loading the game and just display the help information
				showHelp();
				set<&GameOptions::_shouldRun>(OptionLevel::COMMAND, false);
				return true;
			}
		}},
		{{"-master"}, {
			true, "GAME",
			"Choose the game you wish to play. Can be either \"xcom1\" or \"xcom2\"",
			[this](const std::string& master) -> bool {
				return false;
			}
		}},
		{{"-data"}, {
			true, "PATH",
			"Use PATH as the default Data Folder. Bypasses the auto-detection.",
			[this](const std::string& path) -> bool
			{
				std::filesystem::path dataPath(path);
				if (!std::filesystem::exists(dataPath) || !std::filesystem::is_directory(dataPath))
				{
					Log(LOG_ERROR) << "Invalid data path \"" + path + "\".";
					return false;
				}
				std::vector<std::filesystem::path> paths = {dataPath};
				set<&GameOptions::_dataPath>(OptionLevel::COMMAND, paths);
				return true;
			}
		}},
		{{"-user"}, {
			true, "PATH",
			"Use PATH as the default User Folder. Bypasses the auto-detection.",
			[this](const std::string& path) -> bool
			{
				std::filesystem::path userPath(path);
				if (!std::filesystem::exists(userPath) || !std::filesystem::is_directory(userPath))
				{
					Log(LOG_ERROR) << "Invalid user path \"" + path + "\".";
					return false;
				}
				set<&GameOptions::_userPath>(OptionLevel::COMMAND, userPath);
				return true;
			}
		}},
		{{"-cfg", "-config"}, {
			true, "PATH",
			"Use PATH as the default Config Folder. Bypasses the auto-detection.",
			[this](const std::string& path) -> bool
			{
				std::filesystem::path cfgPath(path);
				if (!std::filesystem::exists(cfgPath) || !std::filesystem::is_directory(cfgPath))
				{
					Log(LOG_ERROR) << "Invalid config path \"" + path + "\".";
					return false;
				}
				set<&GameOptions::_cfgPath>(OptionLevel::COMMAND, cfgPath);
				return true;
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
		{{"-locale"}, {
			true, "LOCALE",
			"Override the default locale settings. Use LIST as a parameter to this option to see all available locale options.",
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

bool Options::loadCommandLine(const std::vector<std::string>& argv)
{
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
			if (argumentsIter == argv.end())
			{
				Log(LOG_ERROR) << "Missing parameter for command line argument \"" + argument + "\".";
				return false;
			}

			const std::string& parameter = *argumentsIter;
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
				Log(LOG_ERROR) << "Unknown error with \"" + argument + "\".";
				return false;
			}
		}

		++argumentsIter;
	}

	return true;
}

bool Options::loadFile()
{
	// read the options from the config file

//	fromJson<>
	return true;
}


std::filesystem::path getUserDocumentsDirectory()
{
#ifdef _WIN32
	PWSTR path;
	if (SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path) != S_OK)
	{
		return std::filesystem::path();
	}
	std::filesystem::path documentsPath(path);
	CoTaskMemFree(path);
	return documentsPath;
#else
	assert(!"Not yet implemented");
	return std::filesystem::path();
#endif
}

std::filesystem::path getXcomDocumentsDirectory()
{
	return getUserDocumentsDirectory() / "OpenXcom" / OPENXCOM_VERSION_ENGINE;
}

std::filesystem::path getExecutableDirectory()
{
#ifdef _WIN32
	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	std::filesystem::path executablePath(buffer);
	return executablePath.parent_path();
#else
	assert(!"Not yet implemented");
	return std::filesystem::path();
#endif
}

std::string getDefaultLocale()
{
	//determine language from users platform
	return std::locale("").name();
}

bool Options::loadDefaults()
{
	// load the data folders 
	if (!isSet<&GameOptions::_dataPath>())
	{
		// No data path specified, so we should put in a few places where the data could be found.
		std::vector<std::filesystem::path> dataPaths;

		// add the current executables directory to the search paths
		std::filesystem::path executablePath = getExecutableDirectory();
		dataPaths.push_back(executablePath);

		// if the current working directory is different from the executable
		// directory, add it to the search paths
		std::filesystem::path cwd = std::filesystem::current_path();
		if (cwd != executablePath) { dataPaths.push_back(cwd); }

		// add the documents directory to the search paths
		dataPaths.push_back(getXcomDocumentsDirectory());

		set<&GameOptions::_dataPath>(OptionLevel::DEFAULT, dataPaths);
	}

	if (!isSet<&GameOptions::_userPath>())
	{
		// No user path specified, so use the platform options to find it.
		set<&GameOptions::_userPath>(OptionLevel::DEFAULT, getXcomDocumentsDirectory());
	}

	if (!isSet<&GameOptions::_cfgPath>())
	{
		// No config path specified, so use the platform options to find it.
		set<&GameOptions::_cfgPath>(OptionLevel::DEFAULT, getXcomDocumentsDirectory());
	}

	if (!isSet<&GameOptions::_locale>())
	{
		// No language specified, so use the platform options to find it.
		set<&GameOptions::_locale>(OptionLevel::DEFAULT, getDefaultLocale());
	}

	return true;
}


/// Load options from file.
bool Options::load(const std::vector<std::string>& argv)
{
	// First, load up the arguments from the command line
	if (!loadCommandLine(argv)) { return false; }

	// Load up defaults for things that require run time initialization. That includes
	// things like data path and user path(if they aren't set by the command line)
	if (!loadDefaults()) { return false; }

	// Then, load up the options from the config file
	if (!loadFile()) { return false; }

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
