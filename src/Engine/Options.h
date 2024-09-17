#pragma once
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
#include <string>
#include <vector>
#include <array>
#include <optional>
#include <map>
#include <functional>
#include <filesystem>

namespace OpenXcom
{

// helper struct that contains the information for a command line options and
// the function to execute if the option is found
struct CommandLineOption
{
	// If the option has a parameter
	bool hasParameter;

	// The name of the parameter(only used if hasParameter is true)
	std::string parameterName;

	// The description of the option. Used for -help
	std::string description;

	// The function to execute if the option is found
	std::function<bool(const std::string&)> parser;
};


enum class OptionLevel
{
	COMMAND, // command line
	CONFIG,  // config file
	DEFAULT, // default values
	MAX
};

constexpr size_t OPTION_LEVEL_COUNT = static_cast<size_t>(OptionLevel::MAX);

// helper struct for all game options to allow us to check if an option has
// been set or not
template<typename Type>
struct Option
{
	using ValueType = Type;

	std::array<std::optional<ValueType>, OPTION_LEVEL_COUNT> values;

	
    // Default constructor
	Option() = default;

	// Constructor that accepts a ValueType
	Option(const ValueType& val) { set(OptionLevel::DEFAULT, val); }

	void set(OptionLevel level, const ValueType& value)
	{
		values[static_cast<size_t>(level)] = value;
	}

	ValueType get() const
	{
		for (int i = 0; i < OPTION_LEVEL_COUNT; i++)
		{
			if (values[i].has_value())
			{
				return values[i].value();
			}
		}
		throw std::runtime_error("Option not set at any level");
	}

	ValueType getAt(OptionLevel level) const
	{
		return values[(int)level].value();
	}

	bool isSet() const
	{
		for (int i = 0; i < OPTION_LEVEL_COUNT; i++)
		{
			if (values[i].has_value())
			{
				return true;
			}
		}
		return false;
	}
};


// Helper template to extract ClassType and OptionType from MemberPtrType
template <typename MemberPtrType>
struct MemberPtrClass;

template <typename ClsType, typename OptType>
struct MemberPtrClass<OptType ClsType::*>
{
	using ClassType = ClsType;
	using OptionType = OptType;
};

// Helper template to extract the Class and Type from member pointer provided
template <auto MemberPtr>
struct OptionMemberPointerTraits;

template <auto MemberPtr>
struct OptionMemberPointerTraits
{
	// MemberPtrType is of type 'Option<ValueType> ClassType::*'
	using MemberPtrType = decltype(MemberPtr);

	// Helper struct to extract ClassType and OptionType from MemberPtrType
	using ClassType = typename MemberPtrClass<MemberPtrType>::ClassType;
	using OptionType = typename MemberPtrClass<MemberPtrType>::OptionType;
	using ValueType = typename OptionType::ValueType;
};

// Container for the options
template <typename OptionsStruct>
class OptionCategory
{
public:
	OptionsStruct optionsStruct;
};

// Game options
struct GameOptions
{
	Option<bool> _shouldRun = true;

	Option<std::vector<std::filesystem::path>> _dataPath = {};
	Option<std::filesystem::path> _userPath = std::filesystem::path("./user");
	Option<std::filesystem::path> _cfgPath = std::filesystem::path("./user");

	Option<std::string> _language = "en";

	Option<bool> _continueSave = false;
	Option<std::filesystem::path> _savePath = std::filesystem::path();

	Option<std::filesystem::path> _logPath = std::filesystem::path("log.txt");
};

// Graphics options
struct GraphicsOptions
{
	Option<bool> _fullscreen;

	Option<int> _screenWidth;
	Option<int> _screenHeight;
};

/**
 * Class to handle options for the game.
 *
 * The default use of this class is to load options from the command line and
 * an options file. The command line options will override the options file.
 *
 * 
 */
class Options
{
private:
	using OptionsMapKey = std::vector<std::string>;
	using OptionsMapValue = CommandLineOption;
	using OptionsMap = std::vector<std::pair<OptionsMapKey, OptionsMapValue>>;

	OptionsMap _commandLineOptions;

	void showVersion();
	void showHelp();

	bool loadCommandLine(const std::vector<std::string>& argv);

public:
	/// Constructor.
	Options();
	/// Destructor.
	~Options();

	/// Load options from file.
	bool load(const std::vector<std::string>& argv);
	/// Save options to file.
	void save();
	/// Reset options to default values.
	void reset();

    // Set the value of an option at a specific level
	template <auto MemberPtr>
	void set(OptionLevel level, const typename OptionMemberPointerTraits<MemberPtr>::ValueType& value)
	{
		using ClassType = typename OptionMemberPointerTraits<MemberPtr>::ClassType;
		auto& category = getOptionCategory<ClassType>();
		auto& optionMember = category.optionsStruct.*MemberPtr;
		optionMember.set(level, value);
	}

	// Get the value of an option
	template <auto MemberPtr>
	typename OptionMemberPointerTraits<MemberPtr>::ValueType get() const
	{
		using ClassType = typename OptionMemberPointerTraits<MemberPtr>::ClassType;
		const auto& category = getOptionCategory<ClassType>();
		const auto& optionMember = category.optionsStruct.*MemberPtr;
		return optionMember.get();
	}

	// Get the value of an option
	template <auto MemberPtr>
	typename OptionMemberPointerTraits<MemberPtr>::ValueType getAt(OptionLevel level) const
	{
		using ClassType = typename OptionMemberPointerTraits<MemberPtr>::ClassType;
		const auto& category = getOptionCategory<ClassType>();
		const auto& optionMember = category.optionsStruct.*MemberPtr;
		return optionMember.getAt();
	}


	// Check if an option is set(note: if a defualt value is provided, then this will always return true)
	template <auto MemberPtr>
	bool isSet() const
	{
		using ClassType = typename OptionMemberPointerTraits<MemberPtr>::ClassType;
		const auto& category = getOptionCategory<ClassType>();
		const auto& optionMember = category.optionsStruct.*MemberPtr;
		return optionMember.isSet();
	}

	// Get where the option was set(commandline, options, or default). If it hasn't been set, then it will return OPTION_LEVELS::MAX
	template <auto MemberPtr>
	OptionLevel getSetLevel() const
	{
		using ClassType = typename OptionMemberPointerTraits<MemberPtr>::ClassType;
		const auto& category = getOptionCategory<ClassType>();
		const auto& optionMember = category.optionsStruct.*MemberPtr;
		for (int i = 0; i < OPTION_LEVEL_COUNT; i++)
		{
			if (optionMember.values[i].has_value())
			{
				return static_cast<OptionLevel>(i);
			}
		}
		return OptionLevel::MAX;
	}

	// Check if an option is set to the default value
	template <auto MemberPtr>
	bool isDefault() const
	{
		return getSetLevel<MemberPtr>() == OptionLevel::DEFAULT;
	}


private:
	// Helper methods to get the OptionCategory instance
	template <typename OptionsStructType>
	OptionCategory<OptionsStructType>& getOptionCategory()
	{
		return std::get<OptionCategory<OptionsStructType>>(optionCategories);
	}

	template <typename OptionsStructType>
	const OptionCategory<OptionsStructType>& getOptionCategory() const
	{
		return std::get<OptionCategory<OptionsStructType>>(optionCategories);
	}

	// Store OptionCategory instances in a tuple
	std::tuple<
		OptionCategory<GameOptions>,
		OptionCategory<GraphicsOptions>
		>
		optionCategories;
};

}
