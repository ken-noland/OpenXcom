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
#include "ModInfo.h"
#include "../Logger.h"
#include "../Yaml.h"
#include "../Utility/RTTR.h"

#include "../../version.h"


namespace OpenXcom
{

SIMPLERTTR
{
	SimpleRTTR::registration().type<ModType>()
		.meta("Serialize", ObjectSerialize::ALWAYS) // always serialize this
		.value(ENUM_VALUE_REGISTRATION(ModType, Master))
		.value(ENUM_VALUE_REGISTRATION(ModType, Mod));
}

VersionConstraint parseVersionConstraint(std::string_view input, const ryml::ConstNodeRef& yaml, YamlContext& context)
{
	// Skip leading whitespace.
	while (!input.empty() && std::isspace(input.front()))
	{
		input.remove_prefix(1);
	}

	// Extract the operator.
	std::string_view op;
	if (input.size() >= 2)
	{
		auto twoChars = input.substr(0, 2);
		if (twoChars == "==" || twoChars == ">=" || twoChars == "<=")
		{
			op = twoChars;
			input.remove_prefix(2);
		}
	}
	if (op.empty() && !input.empty())
	{
		char c = input.front();
		if (c == '>' || c == '<')
		{
			op = input.substr(0, 1);
			input.remove_prefix(1);
		}
	}
	if (op.empty())
	{
		throw std::invalid_argument("Invalid version constraint: no valid operator found. " + context.toString(yaml));
	}

	// Skip any whitespace between the operator and the version number.
	while (!input.empty() && std::isspace(input.front()))
	{
		input.remove_prefix(1);
	}
	if (input.empty())
	{
		throw std::invalid_argument("Invalid version constraint: missing version number" + context.toString(yaml));
	}

	// The remainder is the version string.
	std::string_view versionStr = input;

	// Map the operator string to our enum.
	VersionOperator versionOp;
	if (op == "==")
	{
		versionOp = VersionOperator::Equal;
	}
	else if (op == ">=")
	{
		versionOp = VersionOperator::GreaterThanEqual;
	}
	else if (op == ">")
	{
		versionOp = VersionOperator::GreaterThan;
	}
	else if (op == "<=")
	{
		versionOp = VersionOperator::LessThanEqual;
	}
	else if (op == "<")
	{
		versionOp = VersionOperator::LessThan;
	}
	else
	{
		throw std::invalid_argument("Unknown operator: " + std::string(op) + ": " + context.toString(yaml));
	}

	// Convert versionStr to a std::string for the semver parser.
	semver::version ver(0,1,0);
	try
	{
		ver = semver::version::parse(std::string(versionStr), false);
	}
	catch (semver::semver_exception& e)
	{
		Log(LOG_WARNING) << "Invalid version constraint: " << e.what();
		Log(LOG_WARNING) << context.toString(yaml);
	}

	return VersionConstraint{versionOp, ver};
}

template <>
bool fromYaml<DependencyExpression>(ryml::ConstNodeRef const& yaml, DependencyExpression& expr, YamlContext& context)
{
	if (yaml.has_child("mod"))
	{
		// Treat this node as a leaf dependency.
		ryml::ConstNodeRef modNode = yaml["mod"];
		expr.mod = std::string(modNode.val().begin(), modNode.val().end());

		if (yaml.has_child("version"))
		{
			// read the version constraints as a string
			ryml::ConstNodeRef versionNode = yaml["version"];
			std::string versionStr(versionNode.val().begin(), versionNode.val().end());

			// parse the version constraints
			expr.constraints.push_back(parseVersionConstraint(versionStr, versionNode, context));
		}

		return true;
	}
	else if (yaml.has_child("and"))
	{
		// Check for a composite dependency using "and"
		expr.op = DependencyExpression::LogicalOperator::And;

		ryml::ConstNodeRef andNode = yaml["and"];
		for (auto child : andNode.children())
		{
			DependencyExpression childExpr;
			if (!fromYaml(child, childExpr, context))
			{
				return false;
			}
			expr.children.push_back(childExpr);
		}
		return true;
	}
	else if (yaml.has_child("or"))
	{
		// Check if the node defines a composite dependency using "or"
		expr.op = DependencyExpression::LogicalOperator::Or;

		ryml::ConstNodeRef orNode = yaml["or"];
		for (auto child : orNode.children())
		{
			DependencyExpression childExpr;
			if (!fromYaml(child, childExpr, context))
			{
				return false;
			}
			expr.children.push_back(childExpr);
		}
		return true;
	}
	else if (yaml.is_seq())
	{
		// If the node is a sequence (and not wrapped in a map), process each element as an implicit "and" group.
		expr.op = DependencyExpression::LogicalOperator::And;
		for (auto child : yaml.children())
		{
			DependencyExpression childExpr;
			if (!fromYaml(child, childExpr, context))
			{
				return false;
			}
			expr.children.push_back(childExpr);
		}
		return true;
	}
	else
	{
		throw OpenXcom::YamlException(yaml, context, "Expected 'mod', 'and', or 'or' field.");
	}
	return false;
}

// we have a custom specialization of the ModInfo so we can figure out the version and do custom overrides later
template <>
bool fromYaml<ModInfo>(ryml::ConstNodeRef const& yaml, ModInfo& modInfo, YamlContext& context)
{
	// ensure the node type is "map"
	if (!yaml.is_map()) { throw OpenXcom::YamlException(yaml, context, "Expected map type."); }
	if (!yaml.has_child("name")) { throw OpenXcom::YamlException(yaml, context, "Missing required 'name' field."); }
	if (!yaml.has_child("description")) { throw OpenXcom::YamlException(yaml, context, "Missing required 'description' field."); }
	if (!yaml.has_child("author")) { throw OpenXcom::YamlException(yaml, context, "Missing required 'author' field."); }

	if (!fromYaml(yaml["name"], modInfo.name, context)) { return false; }
	if (!fromYaml(yaml["description"], modInfo.description, context)) { return false; }
	if (!fromYaml(yaml["author"], modInfo.author, context)) { return false; }

	//try reading the id
	if (yaml.has_child("id"))
	{
		if (!fromYaml(yaml["id"], modInfo.id, context)) { return false; }
	}
	else
	{
		Log(LOG_WARNING) << "Missing 'id' field";
		Log(LOG_WARNING) << context.toString(yaml);
		modInfo.id = modInfo.name;
	}


	// try reading the version
	if (yaml.has_child("version"))
	{
		ryml::ConstNodeRef versionNode = yaml["version"];
		std::string versionStr(versionNode.val().begin(), versionNode.val().end());
		semver::version ver(0, 1, 0);
		try
		{
			ver = semver::version::parse(std::string(versionStr), false);
		}
		catch (semver::semver_exception& e)
		{
			Log(LOG_WARNING) << "Invalid version constraint: " << e.what();
			Log(LOG_WARNING) << context.toString(yaml);
		}
		modInfo.version = ver;
	}
	else
	{
		Log(LOG_WARNING) << "Missing 'version' field";
		Log(LOG_WARNING) << context.toString(yaml);
		modInfo.version = semver::version(1, 0, 0);
	}

	// try reading the type
	bool typeIsSet = false;
	if (yaml.has_child("type"))
	{
		if (!fromYaml(yaml["type"], modInfo.type, context))
		{
			return false;
		}
		typeIsSet = true;
	}

	// try reading the requiredEngine
	modInfo.requiredEngine = OPENXCOM_VERSION_ENGINE;
	if (yaml.has_child("requiredEngine"))
	{
		if (!fromYaml(yaml["requiredEngine"], modInfo.requiredEngine, context))
		{
			return false;
		}
	}

	// try reading the requiredVersion
	if (yaml.has_child("requiredVersion"))
	{
		ryml::ConstNodeRef requiredVersionNode = yaml["requiredVersion"];
		std::string versionStr(requiredVersionNode.val().begin(), requiredVersionNode.val().end());
		modInfo.requiredVersion = semver::version::parse(versionStr, false);
	}

	// try reading the dependencies
	if (yaml.has_child("dependencies"))
	{
		ryml::ConstNodeRef dependenciesNode = yaml["dependencies"];
		for (auto child : dependenciesNode.children())
		{
			DependencyExpression dep;
			if (!fromYaml(child, dep, context))
			{
				return false;
			}
			modInfo.dependencies.push_back(dep);
		}
	}

	// try reading the conflicts
	if (yaml.has_child("conflicts"))
	{
		ryml::ConstNodeRef conflictsNode = yaml["conflicts"];
		for (auto child : conflictsNode.children())
		{
			DependencyExpression dep;
			if (!fromYaml(child, dep, context))
			{
				return false;
			}
			modInfo.conflicts.push_back(dep);
		}
	}

	// try reading the "resourceConfigFile" field
	if (yaml.has_child("resourceConfigFile"))
	{
		ryml::ConstNodeRef resourceConfigFileNode = yaml["resourceConfigFile"];
		std::string resourceConfigFileStr(resourceConfigFileNode.val().begin(), resourceConfigFileNode.val().end());
		modInfo.resourceConfigFile = std::filesystem::path(resourceConfigFileStr);
	}

	// try reading the "loadResources" field
	if (yaml.has_child("loadResources"))
	{
		ryml::ConstNodeRef loadNode = yaml["loadResources"];
		for (auto child : loadNode.children())
		{
			std::string loadResourcesStr(child.val().begin(), child.val().end());
			modInfo.resourceDirectories.push_back(std::filesystem::path(loadResourcesStr));
		}
	}

	//---
	// The following are deprecated fields, but still useful to parse so we know how to deal with deserializing values going forward

	// try reading the version display
	if (yaml.has_child("versionDisplay"))
	{
		Log(LOG_WARNING) << "The versionDisplay field is deprecated. Please use the 'version' field instead.";
		Log(LOG_WARNING) << context.toString(yaml);
		// TODO: put in documentation link here
	}

	// try reading the "isMaster" field
	if (yaml.has_child("isMaster"))
	{
		Log(LOG_WARNING) << "The isMaster field is deprecated. Please use the 'type' field instead.";
		Log(LOG_WARNING) << context.toString(yaml);
		// TODO: put in documentation link here

		if (typeIsSet)
		{
			Log(LOG_ERROR) << "You can't specify 'type' and 'isMaster' in the same file."
				"'isMaster' is the old way, and 'type' is the new way. Using both just confuses me.";
			Log(LOG_ERROR) << context.toString(yaml);

			return false;
		}

		// we still need to set the type to master
		bool isMaster = false;
		yaml["isMaster"] >> isMaster;
		if (isMaster)
		{
			modInfo.type = ModType::Master;
		}
	}

	// try reading "master" field
	if (yaml.has_child("master"))
	{
		Log(LOG_WARNING) << "The master field is deprecated. Please use the 'type' field instead.";
		Log(LOG_WARNING) << context.toString(yaml);

		// well, we might as well read it it and record it as a dependency
		ryml::ConstNodeRef masterNode = yaml["master"];
		std::string master(masterNode.val().begin(), masterNode.val().end());
		modInfo.dependencies.push_back(DependencyExpression{master});
	}

	// try reading the "reservedSpace" field
	if (yaml.has_child("reservedSpace"))
	{
		Log(LOG_WARNING) << "The reservedSpace field is deprecated. It is now ignored.";
		Log(LOG_WARNING) << context.toString(yaml);
	}

	return true;
}


} // namespace OpenXcom
