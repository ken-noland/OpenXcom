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
#include "Country.h"
#include "../Engine/RNG.h"
#include "../Engine/ScriptBind.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleCountry.h"
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{

/**
 * Initializes a country of the specified type.
 * @param rules Pointer to ruleset.
 * @param gen Generate new funding.
 */
Country::Country(RuleCountry* rules, bool gen) : _rules(rules), _pact(false), _newPact(false), _cancelPact(false), _funding(0), _satisfaction(Satisfaction::SATISFIED)
{
	if (gen)
	{
		_funding.push_back(_rules->generateFunding());
	}
	_activityAlien.push_back(0);
	_activityXcom.push_back(0);
}

/**
 * Loads the country from a YAML file.
 * @param node YAML node.
 */
void Country::load(const YAML::Node& node, const ScriptGlobal* shared)
{
	_funding = node["funding"].as<std::vector<int> >(_funding);
	_activityXcom = node["activityXcom"].as<std::vector<int> >(_activityXcom);
	_activityAlien = node["activityAlien"].as<std::vector<int> >(_activityAlien);
	_pact = node["pact"].as<bool>(_pact);
	_newPact = node["newPact"].as<bool>(_newPact);
	_cancelPact = node["cancelPact"].as<bool>(_cancelPact);

	_scriptValues.load(node, shared);
}

/**
 * Saves the country to a YAML file.
 * @return YAML node.
 */
YAML::Node Country::save(const ScriptGlobal* shared) const
{
	YAML::Node node;
	node["type"] = _rules->getType();
	node["funding"] = _funding;
	node["activityXcom"] = _activityXcom;
	node["activityAlien"] = _activityAlien;
	if (_pact)
	{
		node["pact"] = _pact;
		if (_cancelPact)
		{
			node["cancelPact"] = _cancelPact;
		}
	}
	// Note: can have a _newPact flag, even if already has a _pact from earlier (when xcom liberates and aliens retake a country during the same month)
	if (_newPact)
	{
		node["newPact"] = _newPact;
	}

	_scriptValues.save(node, shared);

	return node;
}

namespace {
	const int FUNDING_INCREASE = +1;
	const int FUNDING_DECREASE = -1;
	const int FUNDING_NO_CHANGE = 0;
}

/**
 * reset all the counters,
 * calculate this month's funding,
 * set the change value for the month.
 * @param xcomTotal the council's xcom score
 * @param alienTotal the council's alien score
 * @param pactScore the penalty for signing a pact
 * @param averageFunding current average funding across all countries (including withdrawn countries) rounded to thousands place
 */
void Country::newMonth(int xcomTotal, int alienTotal, int pactScore, int64_t averageFunding, const SavedGame* save)
{	
	const int funding = getFunding().back();
	int randomFunding = std::max(1000, ((funding / 1000) * RNG::generate(5, 20) / 100) * 1000); // change is always at least 1000

	const int xcomPoints  = (xcomTotal  / 10) + _activityXcom.back();
	const int alienPoints = (alienTotal / 20) + _activityAlien.back();

	// funding increases if xcom outscores the aliens by 30 and passes the rng check.
	int fundingDirection = xcomPoints > alienPoints + 30 && RNG::generate(0, xcomPoints) > alienPoints ? FUNDING_INCREASE
					     : RNG::generate(0, alienPoints) > xcomPoints ? FUNDING_DECREASE
					     : FUNDING_NO_CHANGE;
	int fundingChange = fundingDirection * randomFunding;

	// clamp funding change so that change + funding cannot exceed the cap, or go below 0.
	fundingChange = std::clamp(randomFunding, -funding, (getRules()->getFundingCap() * 1000) - funding);

	_satisfaction = fundingChange > 0 ? Satisfaction::SATISFIED
				  : fundingChange < 0 ? Satisfaction::UNHAPPY
		          : Satisfaction::SATISFIED;

	if (_cancelPact && funding <= 0)
	{
		_satisfaction = Satisfaction::SATISFIED; // satisfied, not happy or unhappy
		fundingChange = (int)averageFunding;
	}

	// call script which can adjust values.
	ModScript::NewMonthCountry::Output args{fundingChange, static_cast<int>(_satisfaction), _newPact, _cancelPact};
	ModScript::NewMonthCountry::Worker work{this, save, xcomTotal, alienTotal};
	work.execute(_rules->getScript<ModScript::NewMonthCountry>(), args);

	fundingChange = std::get<0>(args.data);
	_satisfaction = static_cast<Satisfaction>(std::get<1>(args.data));
	_newPact = static_cast<bool>(std::get<2>(args.data));
	_cancelPact = static_cast<bool>(std::get<3>(args.data));

	// form/cancel pacts
	if (_newPact)
	{
		_pact = true;
		addActivityAlien(pactScore);
	}
	else if (_cancelPact)
	{
		_pact = false;
	}

	// reset pact change states
	_newPact = false;
	_cancelPact = false;

	// set the new funding and reset the activity meters
	if (_pact)
		_funding.push_back(0); // yes, hardcoded!
	else
		_funding.push_back(funding + fundingChange);

	_activityAlien.push_back(0);
	_activityXcom.push_back(0);
	if (_activityAlien.size() > 12)
		_activityAlien.erase(_activityAlien.begin());
	if (_activityXcom.size() > 12)
		_activityXcom.erase(_activityXcom.begin());
	if (_funding.size() > 12)
		_funding.erase(_funding.begin());
}

/**
 * sign a new pact at month's end.
 */
void Country::setNewPact()
{
	_newPact = true;
	_cancelPact = false;
}

/**
 * cancel or prevent a pact.
 */
void Country::setCancelPact()
{
	if (_pact)
	{
		// cancel an existing signed pact
		_cancelPact = true;
		_newPact = false;
	}
	else
	{
		// prevent a not-yet-signed pact
		_cancelPact = false;
		_newPact = false;
	}
}

/**
 * can be (re)infiltrated?
 */
const bool Country::canBeInfiltrated() const
{
	if (!_pact && !_newPact)
	{
		// completely new infiltration; or retaking a previously liberated country
		return true;
	}
	if (_pact && _cancelPact)
	{
		// xcom tried to liberate them this month, but the aliens were not amused... who shall they listen to at the end?
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////
//					Script binding
////////////////////////////////////////////////////////////

namespace
{

std::string debugDisplayScript(const Country* c)
{
	if (c)
	{
		std::string s;
		s += Country::ScriptName;
		s += "(name: \"";
		s += c->getRules()->getType();
		s += "\")";
		return s;
	}
	else
	{
		return "null";
	}
}

} // namespace

void Country::ScriptRegister(ScriptParserBase* parser)
{
	parser->registerPointerType<RuleCountry>();

	Bind<Country> c = {parser};

	c.addRules<RuleCountry, &Country::getRules>("getRuleCountry");

	c.add<&Country::getPact>("getPact", "Get if the country has signed an alien pact or not.");

	c.add<&Country::getCurrentFunding>("getCurrentFunding", "Get the country's current funding.");
	c.add<&Country::getCurrentActivityAlien>("getCurrentActivityAlien", "Get the country's current alien activity.");
	c.add<&Country::getCurrentActivityXcom>("getCurrentActivityXcom", "Get the country's current xcom activity.");

	c.addScriptValue<&Country::_scriptValues>();
	c.addDebugDisplay<&debugDisplayScript>();

	c.addCustomConst("SATISFACTION_ALIENPACT", 0);
	c.addCustomConst("SATISFACTION_UNHAPPY", 1);
	c.addCustomConst("SATISFACTION_SATISIFIED", 2);
	c.addCustomConst("SATISFACTION_HAPPY", 3);
}

/**
 * Constructor of new month country script parser.
 * Called every new month for every country.
 */
ModScript::NewMonthCountryParser::NewMonthCountryParser(ScriptGlobal* shared, const std::string& name, Mod* mod) : ScriptParserEvents{shared, name,
																																	  "fundingChange", "satisfaction", "formPact", "cancelPact",
																																	  "country", "geoscape_game", "totalXcomScore", "totalAlienScore"}
{
	BindBase b{this};

	b.addCustomPtr<const Mod>("rules", mod);
}

}
