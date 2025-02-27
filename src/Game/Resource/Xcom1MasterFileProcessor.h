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
#include <filesystem>

#include <semver/semver.hpp>

namespace OpenXcom
{

class EngineContext;

enum class ModType
{
	Master,
	Mod
};

enum class VersionOperator
{
	Equal,            // ==
	GreaterThan,      // >
	GreaterThanEqual, // >=
	LessThan,         // <
	LessThanEqual,    // <=
	// You can add more operators as needed
};

struct VersionConstraint
{
	VersionOperator op;
	semver::version version;
};

struct DependencyExpression
{
	enum class LogicalOperator
	{
		None, // for leaf nodes
		And,
		Or,
	};

	std::string mod;

	// A dependency might include multiple constraints (e.g. >=1.0.0 and <2.0.0)
	std::vector<VersionConstraint> constraints;

	// For a composite dependency (AND/OR):
	LogicalOperator op = LogicalOperator::None;
	std::vector<DependencyExpression> children;

	// Helper to check if this node is a leaf.
	bool isLeaf() const
	{
		return op == LogicalOperator::None && children.empty();
	}
};

struct ModInfo
{
	std::string id;
	std::string name;
	std::string description;
	std::string author;
	semver::version version = semver::version();

	ModType type = ModType::Mod;

	std::string requiredEngine;
	semver::version requiredVersion = semver::version();

	std::vector<DependencyExpression> dependencies;
	std::vector<DependencyExpression> conflicts;

	std::filesystem::path resourceConfigFile;
	std::vector<std::filesystem::path> resourceDirectories;
};

class Xcom1MasterFileProcessor
{
protected:
	EngineContext& _engine;

	// These helper methods mirror functionality previously in mod.cpp.
	bool loadVanillaResources();
	bool loadBattlescapeResources();
	bool loadExtraResources();

public:
	Xcom1MasterFileProcessor(EngineContext& engine);
	~Xcom1MasterFileProcessor();

	bool load(const std::filesystem::path& path);

	bool isValid(const std::filesystem::path& path);
};

} // namespace OpenXcom
