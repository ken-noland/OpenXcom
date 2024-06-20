#pragma once
/*
 * Copyright 2010-2024 OpenXcom Developers.
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
#include <vector>
#include <yaml-cpp/yaml.h>
#include "../Engine/Script.h"

namespace OpenXcom
{

class RuleCountry;
class SavedGame;

/**
 * Represents a country that funds the player.
 * Contains variable info about a country like
 * monthly funding and various activities.
 */
class Country
{
public:
	enum class Satisfaction : int { ALIEN_PACT, UNHAPPY, SATISFIED, HAPPY };

	/// Name of class used in script.
	static constexpr const char* ScriptName = "Country";
	/// Register all useful function used by script.
	static void ScriptRegister(ScriptParserBase* parser);

private:
	RuleCountry *_rules;
	bool _pact, _newPact, _cancelPact;
	std::vector<int> _funding, _activityXcom, _activityAlien;
	Satisfaction _satisfaction;
	ScriptValues<Country> _scriptValues;

public:
	/// Creates a new country of the specified type.
	Country(RuleCountry *rules, bool gen = true);
	/// Loads the country from YAML.
	void load(const YAML::Node& node, const ScriptGlobal* shared);
	/// Saves the country to YAML.
	YAML::Node save(const ScriptGlobal* shared) const;
	/// Gets the country's ruleset.
	[[nodiscard]] const RuleCountry* getRules() const { return _rules; }
	/// Gets the country's funding.
	[[nodiscard]] std::vector<int>& getFunding() { return _funding; }
	/// Gets the country's funding.
	[[nodiscard]] const std::vector<int>& getFunding() const { return _funding; }
	/// Changes the countries current funding.
	void setFunding(int funding) { _funding.back() = funding; }

	/// get the country's satisfaction level
	[[nodiscard]] Satisfaction getSatisfaction() const { return _pact ? Satisfaction::ALIEN_PACT : _satisfaction; }
	/// add xcom activity in this country
	void addActivityXcom(int activity) { _activityXcom.back() += activity; }
	/// add alien activity in this country
	void addActivityAlien(int activity) { _activityAlien.back() += activity; }
	/// get xcom activity to this country
	[[nodiscard]] std::vector<int>& getActivityXcom() { return _activityXcom; }
	/// get xcom activity to this country
	[[nodiscard]] const std::vector<int>& getActivityXcom() const { return _activityXcom; }
	/// get xcom activity to this country
	[[nodiscard]] std::vector<int>& getActivityAlien() { return _activityAlien; }
	/// get xcom activity to this country
	[[nodiscard]] const std::vector<int>& getActivityAlien() const { return _activityAlien; }
	/// store last month's counters, start new counters, set this month's change.
	void newMonth(int xcomTotal, int alienTotal, int pactScore, int64_t averageFunding, const SavedGame* save);
	/// are we signing a new pact?
	[[nodiscard]] bool getNewPact() const { return _newPact; }
	/// sign a pact at the end of this month.
	void setNewPact();
	/// are we cancelling an existing pact?
	[[nodiscard]] bool getCancelPact() const { return _cancelPact; }
	/// cancel or prevent a pact.
	void setCancelPact();
	/// has a new pact been signed?
	[[nodiscard]] bool getPact() const { return _pact; }
	/// sign a pact immediately
	void setPact() { _pact = true; }
	/// can be (re)infiltrated?
	[[nodiscard]] const bool canBeInfiltrated() const;

private:
	int getCurrentFunding() const { return _funding.back(); }
	int getCurrentActivityAlien()  const { return _activityAlien.back(); }
	int getCurrentActivityXcom() const { return _activityXcom.back(); }
};

}
