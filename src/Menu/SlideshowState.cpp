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
#include "SlideshowState.h"
#include "CutsceneState.h"
#include "../Engine/FileMap.h"
#include "../Engine/Game.h"
#include "../Engine/InteractiveSurface.h"
#include "../Engine/Screen.h"
#include "../Engine/Timer.h"
#include "../Interface/Text.h"
#include "../Mod/Mod.h"
#include "../Engine/Options.h"
#include "../Interface/Cursor.h"

namespace OpenXcom
{

SlideshowState::SlideshowState(const SlideshowHeader &slideshowHeader, const std::vector<SlideshowSlide> *slideshowSlides)
	: State("SlideshowState", true), _slideshowHeader(slideshowHeader), _slideshowSlides(slideshowSlides), _curScreen(-1)
{
	_wasLetterboxed = CutsceneState::initDisplay();

	// pre-render and queue up all the frames
	for (const auto& def : *_slideshowSlides)
	{
		InteractiveSurface *slide =
			new InteractiveSurface(Screen::ORIGINAL_WIDTH, Screen::ORIGINAL_HEIGHT, 0, 0);
		slide->loadImage(def.imagePath);
		slide->onMouseClick((ActionHandler)&SlideshowState::screenClick);
		slide->onKeyboardPress((ActionHandler)&SlideshowState::screenClick, Options::keyOk);
		slide->onKeyboardPress((ActionHandler)&SlideshowState::screenSkip, Options::keyCancel);
		slide->setVisible(false);
		_slides.push_back(slide);
		setStatePalette(slide->getPalette());
		add(slide);

		// initialize with default rect; may get overridden by
		// category/id definition
		Text *caption = new Text(def.w, def.h, def.x, def.y);
		caption->setColor(def.color);
		caption->setText(tr(def.caption));
		caption->setAlign(def.align);
		caption->setVerticalAlign(def.valign);
		caption->setWordWrap(true);
		caption->setVisible(false);
		_captions.push_back(caption);
		add(caption);
	}

	centerAllSurfaces();

	int transitionSeconds = _slideshowHeader.transitionSeconds;
	if (_slideshowSlides->front().transitionSeconds > 0)
		transitionSeconds = _slideshowSlides->front().transitionSeconds;
	_transitionTimer = new Timer(transitionSeconds * 1000);
	_transitionTimer->onState(std::bind(&SlideshowState::screenTimer, this));

	getGame()->getMod()->playMusic(_slideshowHeader.musicId);
	getGame()->getCursor()->setVisible(false);
	screenClick(0);
}

SlideshowState::~SlideshowState()
{
	delete _transitionTimer;
}

/**
 * Shows the next screen on a timed basis.
 */
void SlideshowState::screenTimer()
{
	screenClick(0);
}

/**
 * Handle timers.
 */
void SlideshowState::update()
{
	_transitionTimer->think(true, false);
}

/**
 * Shows the next screen in the slideshow; pops the state when there are no more slides
 */
void SlideshowState::screenClick(Action *action)
{
	if (_curScreen >= 0)
	{
		_slides[_curScreen]->setVisible(false);
		_captions[_curScreen]->setVisible(false);
	}

	++_curScreen;

	// next screen
	if (_curScreen < (int)_slideshowSlides->size())
	{
		int transitionSeconds = _slideshowHeader.transitionSeconds;
		if (_slideshowSlides->at(_curScreen).transitionSeconds > 0)
			transitionSeconds = _slideshowSlides->at(_curScreen).transitionSeconds;
		_transitionTimer->setInterval(transitionSeconds * 1000);
		_transitionTimer->start();
		setStatePalette(_slides[_curScreen]->getPalette());
		_slides[_curScreen]->setVisible(true);
		_captions[_curScreen]->setVisible(true);
		init();
	}
	else
	{
		screenSkip(action);
	}
}

/**
 * Skips the slideshow
 */
void SlideshowState::screenSkip(Action *)
{
	// slideshow is over.  restore the screen scale and pop the state
	getGame()->getCursor()->setVisible(true);
	CutsceneState::resetDisplay(_wasLetterboxed);
	getGame()->popState();
}

}
