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
#include <iostream>
#include <iosfwd>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include <inja.hpp>

#include "../Entity/Common/RTTR.h"

//some string helpers
//  trim from start (in place)
inline void ltrim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
				return !std::isspace(ch);
			}));
}

// trim from end (in place)
inline void rtrim(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
				return !std::isspace(ch);
			}).base(),
			s.end());
}

//  trim from both ends (in place)
inline void trim(std::string& s)
{
	rtrim(s);
	ltrim(s);
}

// trim from start (copying)
inline std::string ltrim_copy(std::string s)
{
	ltrim(s);
	return s;
}

// trim from end (copying)
inline std::string rtrim_copy(std::string s)
{
	rtrim(s);
	return s;
}

// trim from both ends (copying)
inline std::string trim_copy(std::string s)
{
	trim(s);
	return s;
}


struct TypeComparator
{
	bool operator()(const SimpleRTTR::Type& lhs, const SimpleRTTR::Type& rhs) const
	{
		return lhs.FullyQualifiedName() < rhs.FullyQualifiedName();
	}
};

struct CommandLineArguments
{
	std::filesystem::path templatePath;
	std::filesystem::path outputPath;
};

using TypeSet = std::set<SimpleRTTR::Type, TypeComparator>;

void collectTemplateTypes(TypeSet& types, const SimpleRTTR::Type& type, const std::vector<std::function<bool(const SimpleRTTR::Type&)>>& filters)
{
	// Check if the type is a template type
	if (type.TemplateParams().size() > 0)
	{
		// Get the template arguments
		for (const SimpleRTTR::TypeReference& templateRefArg : type.TemplateParams())
		{
			const SimpleRTTR::Type& templateArg = templateRefArg.Type();

			bool passesAllFilters = true;

			// Apply all filters
			for (const auto& filter : filters)
			{
				if (!filter(templateArg))
				{
					passesAllFilters = false;
					break;
				}
			}

			if (passesAllFilters)
			{
				types.insert(templateArg);
				// Recursively collect template types
				collectTemplateTypes(types, templateArg, filters);
			}
		}
	}
}

void collectProperties(TypeSet& types, const SimpleRTTR::Type& type, const std::vector<std::function<bool(const SimpleRTTR::Type&)>>& filters)
{
	for (const SimpleRTTR::Property& prop : type.Properties())
	{
		const SimpleRTTR::Type& propType = prop.Type();
		bool passesAllFilters = true;
		// Apply all filters
		for (const auto& filter : filters)
		{
			if (!filter(propType))
			{
				passesAllFilters = false;
				break;
			}
		}
		if (passesAllFilters)
		{
			types.insert(propType);
		}
	}
}

void collectTypes(TypeSet& types, const std::vector<std::function<bool(const SimpleRTTR::Type&)>>& filters)
{
	// go through the types
	for (const SimpleRTTR::Type& type : SimpleRTTR::Types())
	{
		bool passesAllFilters = true;

		// Apply all filters
		for (const auto& filter : filters)
		{
			if (!filter(type))
			{
				passesAllFilters = false;
				break;
			}
		}

		if (passesAllFilters)
		{
			types.insert(type);
		}
	}

	std::vector<std::function<bool(const SimpleRTTR::Type&)>> otherFilters;

	//exclude std::string, std::filesystem::path, and others that we will be implicitly handling
	otherFilters.push_back([](const SimpleRTTR::Type& type) {
		static std::vector<SimpleRTTR::Type> typesToExclude = {
			SimpleRTTR::Types().GetType<std::string>(),
			SimpleRTTR::Types().GetType<std::filesystem::path>(),
			SimpleRTTR::Types().GetType<bool>(),
			SimpleRTTR::Types().GetType<int>(),
			SimpleRTTR::Types().GetType<short>(),
			SimpleRTTR::Types().GetType<char>(),
			SimpleRTTR::Types().GetType<unsigned int>(),
			SimpleRTTR::Types().GetType<unsigned short>(),
			SimpleRTTR::Types().GetType<unsigned char>(),
			SimpleRTTR::Types().GetType<char*>()
		};

		for (const SimpleRTTR::Type& excludeType : typesToExclude)
		{
			if (type == excludeType)
			{
				return false;
			}
		}

		return true;
	});

	//exclude std::allocator
	otherFilters.push_back([](const SimpleRTTR::Type& type) {
		if (type.Namespaces().size() > 0)
		{
			if (type.Namespaces().back() == "std")
			{
				if (type.Name() == "allocator")
				{
					return false;
				}
			}
		}

		return true;
	});

	for (const SimpleRTTR::Type& type : types)
	{
		collectProperties(types, type, otherFilters);
	}

	for (const SimpleRTTR::Type& type : types)
	{
		collectTemplateTypes(types, type, otherFilters);
	}

	//CollectParameters(types, paramFilters);
}

// Run a template given a metaKey that contains a ObjectSerialize value which is used to determine if the type should be serialized
bool collectTypeInfo(TypeSet& types, const std::string& metaKey)
{
	// Create a vector of filters
	std::vector<std::function<bool(const SimpleRTTR::Type&)>> filters;

	// Add a filter to check if the type is in the "OpenXcom" namespace
	filters.push_back([](const SimpleRTTR::Type& type) {
		if (type.Namespaces().size() > 0)
		{
			for (const std::string& ns : type.Namespaces())
			{
				if (ns == "OpenXcom")
				{
					return true;
				}
			}
		}
		return false;
	});

	// Add a filter to check if the type has any properties
	filters.push_back([](const SimpleRTTR::Type& type) {
		if (type.Properties().Size() > 0)
		{
			return true;
		}
		return false;
	});

	// Add a filter to see if the type is serializable
	filters.push_back([&metaKey](const SimpleRTTR::Type& type) {
		if (type.Meta().Has(metaKey.c_str()))	// need to have the key as a "const char*", otherwise it doesn't recognize the type
		{
			OpenXcom::ObjectSerialize serialize = type.Meta().Get(metaKey.c_str()).Value().GetAs<OpenXcom::ObjectSerialize>();
			return serialize == OpenXcom::ObjectSerialize::ALWAYS;
		}
		return false;
	});

	collectTypes(types, filters);

	return 0;
}


std::vector<std::string> removeEmptyPreprocessorBlocksRecursive(const std::vector<std::string>& directives, size_t& index)
{
	std::vector<std::string> result;
	while (index < directives.size())
	{
		const std::string& line = directives[index];
		if (line.starts_with("#if"))
		{
			size_t nestedIndex = index + 1;
			std::vector<std::string> nestedResult = removeEmptyPreprocessorBlocksRecursive(directives, nestedIndex);
			if (!nestedResult.empty())
			{
				result.push_back(line);
				result.insert(result.end(), nestedResult.begin(), nestedResult.end());
				result.push_back(directives[nestedIndex]);
			}
			index = nestedIndex;
		}
		else if (line.starts_with("#else"))
		{
			size_t nestedIndex = index + 1;
			std::vector<std::string> nestedResult = removeEmptyPreprocessorBlocksRecursive(directives, nestedIndex);
			if (!nestedResult.empty())
			{
				result.push_back(line);
				result.insert(result.end(), nestedResult.begin(), nestedResult.end());
			}
			index = nestedIndex;
		}
		else if (line.starts_with("#endif"))
		{
			return result;
		}
		else
		{
			result.push_back(line);
		}
		++index;
	}
	return result;
}

std::vector<std::string> removeEmptyPreprocessorBlocks(const std::vector<std::string>& directives)
{
	size_t index = 0;
	return removeEmptyPreprocessorBlocksRecursive(directives, index);
}

bool collectHeaders(std::vector<std::string>& headers, const TypeSet& types)
{
	//collect all the cpp files where the type information was registered
	std::set<std::filesystem::path> sourceFiles;
	for (const SimpleRTTR::Type& type : types)
	{
		if (type.Meta().Has("source_filename"))
		{
			std::filesystem::path source = type.Meta().Get("source_filename").Value().GetAs<const char*>();
			sourceFiles.insert(source);
		}
	}

	// for debugging purposes only
	//for (const std::filesystem::path& source : sourceFiles)
	//{
	//	std::cout << source << std::endl;
	//}

	std::vector<std::string> preprocessorDirectives;

	// collect all the includes from the source files
	for (const std::filesystem::path& source : sourceFiles)
	{
		// get the full path to the source file
		std::filesystem::path sourcePath = source.parent_path();

		std::ifstream file(source);
		if (!file.is_open())
		{
			std::cerr << "Failed to open file: " << source << std::endl;
			return false;
		}
		std::string line;
		while (std::getline(file, line))
		{
			trim(line);
			//if trimmed line starts with a "#", then it is a preprocessor directive
			if (line.starts_with('#'))
			{
				//check to see if the proceprocessor directive is an include
				if (line.starts_with("#include"))
				{
					// get a string_view of the actual include
					std::string include = line.substr(8);
					trim(include);

					//if the include uses "", then it's relative path, so we need to expand that path
					if (include.size() >= 2 && include.front() == '"' && include.back() == '"')
					{
						// relative path, so strip the beginning and ending quotation marks
						include = include.substr(1, include.size() - 2);

						std::filesystem::path includePath = sourcePath / include;

						// convert the path to an absolute path
						includePath = std::filesystem::absolute(includePath);

						preprocessorDirectives.push_back("#include \"" + includePath.string() + "\"");
					}
					else
					{
						// include uses compilers search path, so we don't need to do anything
						preprocessorDirectives.push_back(line);
					}
				}
				else
				{
					//this is for all the #ifdefs, #defines, and #endifs out there
					preprocessorDirectives.push_back(line);
				}
			}
		}
	}

	headers = removeEmptyPreprocessorBlocks(preprocessorDirectives);

	// for debugging purposes only
	//for (const std::string& preprocessorDirective : preprocessorDirectives)
	//{
	//	std::cout << preprocessorDirective << std::endl;
	//}

	return true;
}

bool loadTemplate(inja::Template& templ, std::filesystem::path path, inja::Environment& env)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << path << std::endl;
		return false;
	}
	std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	try
	{
		templ = env.parse(contents);
	}
	catch (const inja::ParserError& e)
	{
		std::cerr << "Parser error in template: " << e.what() << std::endl;
		return false;
	}
	catch (const inja::InjaError& e)
	{
		std::cerr << "Inja error in template: " << e.what() << std::endl;
		return false;
	}

	return true;
}


std::string getTemplateForType(const SimpleRTTR::Type& type)
{
	if (type.Meta().Has("Codegen-Override-Template"))
	{
		return type.Meta().Get("Codegen-Override-Template").Value().GetAs<const char*>();
	}
	else if (type.Namespaces().size() == 1 && type.Namespaces()[0] == "std")
	{
		if (type.Name() == "vector")
		{
			return "vector_template";
		}
		else if (type.Name() == "map" || type.Name() == "unordered_map")
		{
			return "map_template";
		}
		else if (type.Name() == "array")
		{
			return "array_template";
		}
		else
		{
			std::cerr << "No template for std type: " << type.FullyQualifiedName() << std::endl;
		}
	}

	return "object_template";
}

std::string getFullNameForType(const SimpleRTTR::Type& type)
{
	std::string fullName;

	// add namespace
	if (type.Namespaces().size() > 0)
	{
		for (const std::string& ns : type.Namespaces())
		{
			fullName += ns + "::";
		}
	}

	fullName += type.Name();

	// add template parameters
	if (type.TemplateParams().size() > 0)
	{
		fullName += "<";
		for (size_t i = 0; i < type.TemplateParams().size(); ++i)
		{
			if (i > 0)
			{
				fullName += ", ";
			}

			fullName += getFullNameForType(type.TemplateParams()[i].Type());
		}
		fullName += ">";
	}
	return fullName;
}

bool convertTypesToJson(nlohmann::json::reference typesJson, const TypeSet& types)
{
	typesJson = nlohmann::json::array();
	for (const SimpleRTTR::Type& type : types)
	{
		nlohmann::json typeJson;
		typeJson["name"] = type.Name();
		typeJson["codegen_template"] = getTemplateForType(type);
		typeJson["fullName"] = getFullNameForType(type);
		typeJson["namespace"] = type.Namespaces();
		typeJson["fully_qualified_name"] = type.FullyQualifiedName();

		typeJson["properties"] = nlohmann::json::array();
		for (const SimpleRTTR::Property& prop : type.Properties())
		{
			nlohmann::json propJson;

			propJson["name"] = prop.Name();
			propJson["fulltype"] = getFullNameForType(prop.Type());
			propJson["type"] = prop.Type().Name();

			typeJson["properties"].push_back(propJson);
		}

		if (type.TemplateParams().size() > 0)
		{
			typeJson["template_params"] = nlohmann::json::array();
			for (const SimpleRTTR::TypeReference& templateRefArg : type.TemplateParams())
			{
				const SimpleRTTR::Type& templateArg = templateRefArg.Type();
				nlohmann::json templateArgJson;
				templateArgJson["name"] = templateArg.Name();
				templateArgJson["fulltype"] = getFullNameForType(templateArg);
				templateArgJson["type"] = templateArg.Name();
				typeJson["template_params"].push_back(templateArgJson);
			}
		}

		typesJson.push_back(typeJson);
	}
	return true;
}

bool generateCode(const CommandLineArguments& args)
{
	TypeSet types;
	collectTypeInfo(types, "Serialize"); // collect all types that are serializable, as well as any dependant types

	// for debugging purposes only
	//for (const SimpleRTTR::Type& type : types)
	//{
	//	std::cout << type.FullyQualifiedName() << std::endl;
	//}

	std::vector<std::string> headers;
	if (!collectHeaders(headers, types))
	{
		std::cerr << "Failed to collect headers" << std::endl;
		return false;
	}

	// load up the template definition file
	std::ifstream file(args.templatePath);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << args.templatePath << std::endl;
		return false;
	}

	//read the json
	nlohmann::json templateJsonDefinition;
	file >> templateJsonDefinition;
	file.close();

	inja::Environment env;
	std::map<std::string, inja::Template> typeTemplates;
	
	// add callback which selectively renderers the correct template for each type
	env.add_callback("render_type", 1, [&env, &typeTemplates](inja::Arguments& args) {
		nlohmann::json type = args.at(0)->get<nlohmann::json>();

		// get the template for the type
		std::string templateName = type["codegen_template"];

		// check if the template exists in the map, and if not, fail
		std::map<std::string, inja::Template>::iterator it = typeTemplates.find(templateName);
		if (it == typeTemplates.end())
		{
			throw new inja::RenderError("Template not found: " + templateName, inja::SourceLocation(0, 0));
		}

		nlohmann::json data;
		data["type"] = type;

		// render the template
		return env.render(it->second, data);
	});

	// load up the header template
	std::filesystem::path headerTemplatePath = args.templatePath.parent_path() / templateJsonDefinition["header_template"];
	inja::Template headerTemplate;
	if (!loadTemplate(headerTemplate, headerTemplatePath, env)) { return false; }
	env.include_template("header_template", headerTemplate);

	// load up the main template
	std::filesystem::path mainTemplatePath = args.templatePath.parent_path() / templateJsonDefinition["main_template"];
	inja::Template mainTemplate;
	if (!loadTemplate(mainTemplate, mainTemplatePath, env)) { return false; }

	// load up the template specializations which are stored in key:value pairs inside "specializations"
	nlohmann::json specializations = templateJsonDefinition["specializations"];
	for (nlohmann::json::iterator it = specializations.begin(); it != specializations.end(); ++it)
	{
		std::filesystem::path specializationTemplatePath = args.templatePath.parent_path() / it.value();
		inja::Template specializationTemplate;
		if (!loadTemplate(specializationTemplate, specializationTemplatePath, env))
		{
			return false;
		}
		typeTemplates[it.key()] = specializationTemplate;
	}

	// add headers to the environment
	nlohmann::json data;
	data["headers"] = headers;
	convertTypesToJson(data["types"], types);

	// render the main template
	std::string result;
	try
	{
		
		result = env.render(mainTemplate, data);
	}
	catch (const inja::RenderError& e)
	{
		std::cerr << "Render error in main template: " << e.what() << std::endl;
		return false;
	}
	catch (const inja::InjaError& e)
	{
		std::cerr << "Inja error in main template: " << e.what() << std::endl;
		return false;
	}

	if (result.empty())
	{
		std::cerr << "Failed to render main template" << std::endl;
		return false;
	}

	// write the result to the output file
	std::ofstream outputFile(args.outputPath);
	if (!outputFile.is_open())
	{
		std::cerr << "Failed to open file: " << args.outputPath << std::endl;
		return false;
	}

	outputFile << result;
	outputFile.close();

	return true;
}

std::vector<std::string>::const_iterator getCommandLineArgument(const std::vector<std::string>& args, const std::string& argument)
{
	std::vector<std::string>::const_iterator it = std::find(args.begin(), args.end(), "-template");
	if (it != args.end() && ++it != args.end())
	{
		return it;
	}
	return args.end();
}

void printUsage()
{
	std::cout << "Usage: Codegen -template <template definition file> -output <output file>" << std::endl;
}

bool parseCommandLine(CommandLineArguments& args, int argc, char* argv[])
{
	// convert to vector for easier handling
	std::vector<std::string> cmdArgs(argv, argv + argc);

	std::vector<std::pair<std::string, std::function<void(const std::string&)>>> argHandlers = {
		{"-template", [&args](const std::string& arg) { args.templatePath = arg; }},
		{"-output", [&args](const std::string& arg) { args.outputPath = arg; }}};

	std::vector<std::string>::iterator it = ++cmdArgs.begin(); // skip the first argument which is the executable name
	while (it != cmdArgs.end())
	{
		bool handlerFound = false;
		for (const auto& handler : argHandlers)
		{
			if (*it == handler.first)
			{
				handlerFound = true;
				++it;
				if(it != cmdArgs.end())
				{
					handler.second(*it);
				}
				else
				{
					std::cerr << "No argument provided for \"" << handler.first << "\"." << std::endl;
					return false;
				}
			}
		}
		if (!handlerFound)
		{
			std::cerr << "Unknown argument \"" << *it << "\"." << std::endl;
			return false;
		}
		++it;
	}

	// check that the template was specified in the command line
	if (args.templatePath.empty())
	{
		std::cerr << "No template file specified" << std::endl;
		return false;
	}

	// check the template actually exists
	if (!std::filesystem::exists(args.templatePath))
	{
		std::cerr << "Template file does not exist" << std::endl;
		return 1;
	}

	// check that the output was specified in the command line
	if (args.outputPath.empty())
	{
		std::cerr << "No output file specified" << std::endl;
		return false;
	}

	// check the path to the output actually exists, and if not, create it
	if (!std::filesystem::exists(args.outputPath.parent_path()))
	{
		if (!std::filesystem::create_directories(args.outputPath.parent_path()))
		{
			std::cerr << "Failed to create output directory" << std::endl;
			return false;
		}
	}

	return true;
}

int main(int argc, char* argv[])
{
	CommandLineArguments args;
	if (!parseCommandLine(args, argc, argv))
	{
		printUsage();
		return 1;
	}

	if (!generateCode(args))
	{
		return 1;
	}

	return 0;
}
