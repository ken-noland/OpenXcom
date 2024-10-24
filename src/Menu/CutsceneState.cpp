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

#include "CutsceneState.h"
#include "MainMenuState.h"
#include "SlideshowState.h"
#include "VideoState.h"
#include "../Engine/CrossPlatform.h"
#include "../Engine/Game.h"
#include "../Engine/Logger.h"
#include "../Engine/Options.h"
#include "../Engine/Screen.h"
#include "../Engine/FileMap.h"
#include "../Mod/Mod.h"
#include "../Savegame/SavedGame.h"
#include "StatisticsState.h"

namespace OpenXcom
{

CutsceneState::CutsceneState(const std::string &cutsceneId)
	: State("CutsceneState", true), _cutsceneId(cutsceneId)
{
	// empty
}

CutsceneState::~CutsceneState()
{
	// empty
}

void CutsceneState::init()
{
	State::init();

	// pop self off stack and replace with actual player state
	getGame()->popState();

	const RuleVideo *videoRule = getGame()->getMod()->getVideo(_cutsceneId, true);
	if (getGame()->getSavedGame() && getGame()->getSavedGame()->getEnding() != END_NONE)
	{
		if (getGame()->getSavedGame()->getMonthsPassed() > -1)
		{
			getGame()->setState(new StatisticsState);
		}
		else
		{
			getGame()->setSavedGame(0);
			getGame()->setState(new GoToMainMenuState);
		}
	}

	bool fmv = false, slide = false;
	if (!videoRule->getVideos()->empty())
	{
		fmv = FileMap::fileExists(videoRule->getVideos()->front());
	}
	if (!videoRule->getSlides()->empty())
	{
		slide = FileMap::fileExists(videoRule->getSlides()->front().imagePath);
	}

	if (fmv && (!slide || Options::preferredVideo == VIDEO_FMV))
	{
		getGame()->pushState(new VideoState(videoRule->getVideos(), videoRule->getAudioTracks(), videoRule->useUfoAudioSequence()));
	}
	else if (slide && (!fmv || Options::preferredVideo == VIDEO_SLIDE))
	{
		getGame()->pushState(new SlideshowState(videoRule->getSlideshowHeader(), videoRule->getSlides()));
	}
	else
	{
		Log(LOG_WARNING) << "cutscene definition empty: " << _cutsceneId;
	}
}

bool CutsceneState::initDisplay()
{
	bool letterboxed = Options::keepAspectRatio;
	Options::keepAspectRatio = true;
	Options::baseXResolution = Screen::ORIGINAL_WIDTH;
	Options::baseYResolution = Screen::ORIGINAL_HEIGHT;
	getGame()->getScreen()->resetDisplay(false);
	return letterboxed;
}

void CutsceneState::resetDisplay(bool wasLetterboxed)
{
	Options::keepAspectRatio = wasLetterboxed;
	Screen::updateScale(Options::geoscapeScale, Options::baseXGeoscape, Options::baseYGeoscape, true);
	getGame()->getScreen()->resetDisplay(false);
}

}
