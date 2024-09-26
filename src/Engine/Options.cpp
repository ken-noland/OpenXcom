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
#include "Json.h"
#include "../version.h"

#include <sstream>
#include <iostream>
#include <fstream>

#if defined(_WIN32)
#include <Windows.h>
#include <ShlObj_core.h>
#endif

#include "../Entity/Common/RTTR.h"

namespace OpenXcom
{

SIMPLERTTR
{
	SimpleRTTR::registration().type<GameOptions>()
		.meta("Serialize", ObjectSerialize::ALWAYS) // always serialize this
		.property(REGISTER_PROPERTY(GameOptions, _shouldRun))
			.meta("FriendlyName", "shouldRun")
			.meta("Description", "Determines if the game will run. Only for internal use.")
		.property(REGISTER_PROPERTY(GameOptions, _dataPath))
			.meta("FriendlyName", "dataPath")
			.meta("Description", "A list of all the paths to search for data and mods.")
		.property(REGISTER_PROPERTY(GameOptions, _userPath))
			.meta("FriendlyName", "userPath")
			.meta("Description", "The path to the user folder where save games will be stored.")
		.property(REGISTER_PROPERTY(GameOptions, _cfgPath))
			.meta("FriendlyName", "cfgPath")
			.meta("Description", "The path to the config folder where the config files will be loaded from. Typically the same as \"userPath\".")
		.property(REGISTER_PROPERTY(GameOptions, _locale))
			.meta("FriendlyName", "locale")
			.meta("Serialize", PropertySerialize::ALWAYS) // always serialize this
			.meta("Description", "The locale used for translations.")
		.property(REGISTER_PROPERTY(GameOptions, _continueSave))
			.meta("FriendlyName", "continueSave")
			.meta("Description", "If the game should load the last save")
		.property(REGISTER_PROPERTY(GameOptions, _lastSave))
			.meta("FriendlyName", "lastSave")
			.meta("Description", "The full path to the last save game.")
		.property(REGISTER_PROPERTY(GameOptions, _logPath))
			.meta("FriendlyName", "logPath")
			.meta("Description", "The full path to the log file.")
		.property(REGISTER_PROPERTY(GameOptions, _logLevel))
			.meta("FriendlyName", "logLevel")
			.meta("Serialize", PropertySerialize::ALWAYS) // always serialize this
			.meta("Description", "Minimum level of messages to be output to the log")
		.property(REGISTER_PROPERTY(GameOptions, _mods))
			.meta("FriendlyName", "mods")
			.meta("Serialize", PropertySerialize::ALWAYS) // always serialize this
			.meta("Description", "List of mods to load.")
		.property(REGISTER_PROPERTY(GameOptions, _master))
			.meta("FriendlyName", "master")
			.meta("Serialize", PropertySerialize::ALWAYS) // always serialize this
			.meta("Description", "Master game to use. Can be either xcom1 or xcom2");

	SimpleRTTR::registration().type<GraphicsOptions>()
		.meta("Serialize", ObjectSerialize::ALWAYS) // special case for options, since we only want to serialize a particular layer of the options to disk.
		.property(REGISTER_PROPERTY(GraphicsOptions, _fullscreen))
			.meta("FriendlyName", "fullscreen")
			.meta("Serialize", PropertySerialize::ALWAYS) // always serialize this option
			.meta("Description", "Should the game be fullscreen or not")
		.property(REGISTER_PROPERTY(GraphicsOptions, _screenWidth))
			.meta("FriendlyName", "screenWidth")
			.meta("Serialize", PropertySerialize::ALWAYS) // always serialize this option
			.meta("Description", "Width of the screen in pixels")
		.property(REGISTER_PROPERTY(GraphicsOptions, _screenHeight))
			.meta("FriendlyName", "screenHeight")
			.meta("Serialize", PropertySerialize::ALWAYS) // always serialize this option
			.meta("Description", "Height of the screen in pixels");

	// okay, this is going to look a bit strange, but since Options is a template class, and I don't want to have to go through
	//  and define each possible combination, I simply iterate on all types(they should have been pre-registered by inclusion
	//  as a parameter) and add in special metadata which signals the codegen to handle these types in a unique way
	for (const SimpleRTTR::Type& type : SimpleRTTR::types())
	{
		if (type.name() == "Option")
		{
			SimpleRTTR::registration().type(type)
				.meta("Serialize", ObjectSerialize::ALWAYS) // always serialize this option
				.meta("Description", "A generic option type. This should not be used directly.")
				.meta("Codegen-Override-Template", "option_template");	// this tag tells the codegen that this is a special case for
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
				if (master != "xcom1" && master != "xcom2")
				{
					Log(LOG_ERROR) << "Invalid master game \"" + master + "\".";
					return false;
				}
				set<&GameOptions::_master>(OptionLevel::COMMAND, master);
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
			[this](const std::string& path) -> bool {
				std::filesystem::path cfgPath(path);
				if (!std::filesystem::exists(cfgPath.parent_path()))
				{
					Log(LOG_ERROR) << "Invalid log path \"" + path + "\".";
				}
				set<&GameOptions::_logPath>(OptionLevel::COMMAND, cfgPath);

				return true;
			}
		}},
		{{"-logLevel"}, {
			true, "LEVEL",
			"Set the log output level. Level can be FATAL, ERROR, WARN, INFO, DEBUG, VERB, or ALL",
			[this](const std::string& level) -> bool {
				if (level == "FATAL")
				{
					set<&GameOptions::_logLevel>(OptionLevel::COMMAND, SeverityLevel::LOG_FATAL);
				}
				else if (level == "ERROR")
				{
					set<&GameOptions::_logLevel>(OptionLevel::COMMAND, SeverityLevel::LOG_ERROR);
				}
				else if (level == "WARN")
				{
					set<&GameOptions::_logLevel>(OptionLevel::COMMAND, SeverityLevel::LOG_WARNING);
				}
				else if (level == "INFO")
				{
					set<&GameOptions::_logLevel>(OptionLevel::COMMAND, SeverityLevel::LOG_INFO);
				}
				else if (level == "DEBUG")
				{
					set<&GameOptions::_logLevel>(OptionLevel::COMMAND, SeverityLevel::LOG_DEBUG);
				}
				else if (level == "VERB")
				{
					set<&GameOptions::_logLevel>(OptionLevel::COMMAND, SeverityLevel::LOG_VERBOSE);
				}
				else if (level == "ALL")
				{
					set<&GameOptions::_logLevel>(OptionLevel::COMMAND, SeverityLevel::LOG_UNCENSORED);
				}
				else
				{
					Log(LOG_ERROR) << "Invalid log level \"" + level + "\".";
					return false;
				}

				return true;
			}
		}},
		{{"-continue"}, {
			false, "",
			"Load last saved game",
			[this](const std::string&) -> bool {
				set<&GameOptions::_continueSave>(OptionLevel::COMMAND, true);
				return false;
			}
		}},
		{{"-save"}, {
			true, "FILE",
			"Load the save file specified. This will load the file specified regardless if -continue is specified.",
			[this](const std::string& path) -> bool {
				std::filesystem::path savePath(path);
				if (!std::filesystem::exists(savePath) || !std::filesystem::is_regular_file(savePath))
				{
					Log(LOG_ERROR) << "Invalid save path \"" + path + "\".";
					return false;
				}

				set<&GameOptions::_lastSave>(OptionLevel::COMMAND, path);
				set<&GameOptions::_continueSave>(OptionLevel::COMMAND, true);
				return true;
			}
		}},
		{{"-locale"}, {
			true, "LOCALE",
			"Override the default locale settings. Use LIST as a parameter to this option to see all available locale options.",
			[this](const std::string& locale) -> bool {
				if(locale=="LIST")
				{
					std::cout << "Available locales:" << std::endl;
					std::cout << " ... TODO" << std::endl;
				}

				else
				{
					set<&GameOptions::_locale>(OptionLevel::COMMAND, locale);
				}

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
	//read in the config file
	std::filesystem::path configFile = getConfigFile();
	std::ifstream file(configFile);
	if (!file.is_open())
	{
		std::cerr << "Failed to open configuration file: " << configFile << std::endl;
		return false;
	}

	//parse out the json
	nlohmann::json json;
	file >> json;

	file.close();

	// read the options from the config file
	GameOptions& gameOptions = std::get<OptionCategory<GameOptions>>(_optionCategories).optionsStruct;
	if (!fromJson(json["gameOptions"], gameOptions))
	{
		Log(LOG_ERROR) << "Failed to load options from file.";
		return false;
	}

	return true;
}


bool OpenXcom::Options::saveFile()
{
	nlohmann::json json;

	const GameOptions& gameOptions = std::get<OptionCategory<GameOptions>>(_optionCategories).optionsStruct;
	toJson(gameOptions, json["gameOptions"]);

	std::filesystem::path configFile = getConfigFile();

	// ensure the path to the config file exists, and create it if it doesn't
	std::filesystem::path configPath = configFile.parent_path();
	if (!std::filesystem::exists(configPath))
	{
		if (!std::filesystem::create_directories(configPath))
		{
			std::cerr << "Failed to create configuration directory: " << configPath << std::endl;
			return false;
		}
	}

	// write out the config file
	std::ofstream file(configFile);
	if (!file.is_open())
	{
		std::cerr << "Failed to open configuration file: " << configFile << std::endl;
		return false;
	}

	file << json.dump(0, '\t', true);

	file.close();

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

	if (!isSet<&GameOptions::_logPath>())
	{
		// No log path specified, so use the platform options to find it.
		set<&GameOptions::_logPath>(OptionLevel::DEFAULT, getXcomDocumentsDirectory() / "log.txt");
	}

	if (!isSet<&GameOptions::_mods>())
	{
		// No mods specified, so default to no mods
		set<&GameOptions::_mods>(OptionLevel::DEFAULT, {});
	}

	if (!isSet<&GameOptions::_master>())
	{
		// No master game specified, so default to xcom1
		set<&GameOptions::_master>(OptionLevel::DEFAULT, "xcom1");
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
	loadFile();	//We honestly don't care if it succeeds or fails. We just want it to try!

	return true;
}

/// Save options to file.
bool Options::save()
{
	return saveFile();
}

/// Reset options to default values.
void Options::reset()
{
}

std::filesystem::path OpenXcom::Options::getConfigFile() const
{
	assert(isSet<&GameOptions::_cfgPath>());

	std::filesystem::path configPath = get<&GameOptions::_cfgPath>();
	std::filesystem::path configFile = configPath / "options.json";
	return configFile;
}

}
