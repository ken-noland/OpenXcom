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
#include "State.h"
#include <climits>
#include "InteractiveSurface.h"
#include "Game.h"
#include "Screen.h"
#include "Surface.h"
#include "Language.h"
#include "LocalizedText.h"
#include "Palette.h"
#include "../Engine/Sound.h"
#include "../Mod/Mod.h"
#include "../Interface/Window.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextEdit.h"
#include "../Interface/TextList.h"
#include "../Interface/BattlescapeButton.h"
#include "../Interface/ComboBox.h"
#include "../Interface/Cursor.h"
#include "../Interface/FpsCounter.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Mod/RuleInterface.h"

#include "../Entity/Engine/Surface.h"

namespace OpenXcom
{

/**
 * Initializes a brand new state with no child elements.
 * By default states are full-screen.
 * @param game Pointer to the core game.
 */
State::State(const std::string& name, bool screen) : _name(name), _screen(screen), _soundPlayed(false), _modal(0), _ruleInterface(0), _ruleInterfaceParent(0), _customSound(nullptr)
{
	// initialize palette to all black
	memset(_palette, 0, sizeof(_palette));
	_cursorColor = getGame()->getCursor()->getColor();
}

/**
 * Deletes all the child elements contained in the state.
 */
State::~State()
{
	for(entt::entity surfaceEnt : _surfaces)
	{
		getRegistry().raw().destroy(surfaceEnt);
	}
}

/**
 * Set interface data from the ruleset, also sets the palette for the state.
 * @param category Name of the interface set.
 * @param alterPal Should we swap out the backpal colors?
 * @param battleGame Should we use battlescape palette? (this only applies to options screens)
 */
void State::setInterface(const std::string& category, bool alterPal, SavedBattleGame *battleGame)
{
	int backPal = -1;
	std::string pal = "PAL_GEOSCAPE";

	_interfaceCategory = category;
	_ruleInterface = getGame()->getMod()->getInterface(category);
	if (_ruleInterface)
	{
		_ruleInterfaceParent = getGame()->getMod()->getInterface(_ruleInterface->getParent());
		pal = _ruleInterface->getPalette();
		Element *element = _ruleInterface->getElement("palette");
		if (_ruleInterfaceParent)
		{
			if (!element)
			{
				element = _ruleInterfaceParent->getElement("palette");
			}
			if (pal.empty())
			{
				pal = _ruleInterfaceParent->getPalette();
			}
		}
		if (element)
		{
			int color = alterPal ? element->color2 : element->color;
			if (color != INT_MAX)
			{
				backPal = color;
			}
		}
	}
	if (battleGame)
	{
		battleGame->setPaletteByDepth(this);
	}
	else if (pal.empty())
	{
		pal = "PAL_GEOSCAPE";
		setStandardPalette(pal, backPal);
	}
	else
	{
		setStandardPalette(pal, backPal);
	}
}

/**
 * Set window background from the ruleset.
 * @param window Window handle.
 * @param s ID of the interface ruleset entry.
 */
void State::setWindowBackground(entt::entity window, const std::string& s)
{
	auto& bgImageName = getGame()->getMod()->getInterface(s)->getBackgroundImage();
	setWindowBackgroundImage(window, bgImageName);
}

/**
 * Set window background by image name (instead of by interface name).
 * @param window Window handle.
 * @param s ID of the image.
 */
void State::setWindowBackgroundImage(entt::entity window, const std::string& bgImageName)
{
	const auto* bgImage = getGame()->getMod()->getSurface(bgImageName);

	WindowComponent& windowComponent = getRegistry().raw().get<WindowComponent>(window);
	windowComponent.setBackground(bgImage);
}

/**
 * Adds a new child surface for the state to take care of,
 * giving it the game's display palette. Once associated,
 * the state handles all of the surface's behaviour
 * and management automatically.
 * @param surface Child surface.
 * @note Since visible elements can overlap one another,
 * they have to be added in ascending Z-Order to be blitted
 * correctly onto the screen.
 */
void State::add(Surface *surface)
{
	// Set palette
	surface->setPalette(_palette);

	// Set default text resources
	if (getGame()->getLanguage() && getGame()->getMod())
		surface->initText(getGame()->getMod()->getFont("FONT_BIG"), getGame()->getMod()->getFont("FONT_SMALL"), getGame()->getLanguage());

	entt::registry& registry = getRegistry().raw();
	entt::entity surfaceEnt = registry.create();
	DrawableComponent& drawableComponent = registry.emplace<DrawableComponent>(surfaceEnt);
	std::unique_ptr<Surface> surfacePtr(surface);
	SurfaceComponent& surfaceComponent = registry.emplace<SurfaceComponent>(surfaceEnt, drawableComponent, surfacePtr);
	drawableComponent.setSurfaceComponent(&surfaceComponent);

	_surfaces.push_back(surfaceEnt);
}

/**
 * As above, except this adds a surface based on an
 * interface element defined in the ruleset.
 * @note that this function REQUIRES the ruleset to have been loaded prior to use.
 * @param surface Child surface.
 * @param id the ID of the element defined in the ruleset, if any.
 * @param category the category of elements this interface is associated with.
 * @param parent the surface to base the coordinates of this element off.
 * @note if no parent is defined the element will not be moved.
 */
void State::add(Surface *surface, const std::string &id, const std::string &category, Surface *parent)
{
	// Set palette
	surface->setPalette(_palette);

	// this only works if we're dealing with a battlescape button
	BattlescapeButton *bsbtn = dynamic_cast<BattlescapeButton*>(surface);

	if (getGame()->getMod()->getInterface(category))
	{
		Element *element = getGame()->getMod()->getInterface(category)->getElement(id);
		if (element)
		{
			if (parent && element->w != INT_MAX && element->h != INT_MAX)
			{
				surface->setWidth(element->w);
				surface->setHeight(element->h);
			}

			if (parent && element->x != INT_MAX && element->y != INT_MAX)
			{
				surface->setX(parent->getX() + element->x);
				surface->setY(parent->getY() + element->y);
			}

			auto inter = dynamic_cast<InteractiveSurface*>(surface);
			if (inter)
			{
				inter->setTFTDMode(element->TFTDMode);
			}

			if (element->color != INT_MAX)
			{
				surface->setColor(element->color);
			}
			if (element->color2 != INT_MAX)
			{
				surface->setSecondaryColor(element->color2);
			}
			if (element->border != INT_MAX)
			{
				surface->setBorderColor(element->border);
			}
		}
	}

	if (bsbtn)
	{
		// this will initialize the graphics and settings of the battlescape button.
		bsbtn->copy(parent);
		bsbtn->initSurfaces();
	}

	// Set default text resources
	if (getGame()->getLanguage() && getGame()->getMod())
		surface->initText(getGame()->getMod()->getFont("FONT_BIG"), getGame()->getMod()->getFont("FONT_SMALL"), getGame()->getLanguage());

	entt::registry& registry = getRegistry().raw();
	entt::entity surfaceEnt = registry.create();
	DrawableComponent& drawableComponent = registry.emplace<DrawableComponent>(surfaceEnt);
	std::unique_ptr<Surface> surfacePtr(surface);
	SurfaceComponent& surfaceComponent = registry.emplace<SurfaceComponent>(surfaceEnt, drawableComponent, surfacePtr);
	drawableComponent.setSurfaceComponent(&surfaceComponent);

	_surfaces.push_back(surfaceEnt);
}

void State::add(entt::entity entity, const std::string& id, const std::string& category, Surface* parent)
{
	Surface* surface = getRegistry().raw().get<SurfaceComponent>(entity).getSurface();

	// Set palette
	surface->setPalette(_palette);

	// this only works if we're dealing with a battlescape button
	BattlescapeButton* bsbtn = dynamic_cast<BattlescapeButton*>(surface);

	if (getGame()->getMod()->getInterface(category))
	{
		Element* element = getGame()->getMod()->getInterface(category)->getElement(id);
		if (element)
		{
			if (parent && element->w != INT_MAX && element->h != INT_MAX)
			{
				surface->setWidth(element->w);
				surface->setHeight(element->h);
			}

			if (parent && element->x != INT_MAX && element->y != INT_MAX)
			{
				surface->setX(parent->getX() + element->x);
				surface->setY(parent->getY() + element->y);
			}

			auto inter = dynamic_cast<InteractiveSurface*>(surface);
			if (inter)
			{
				inter->setTFTDMode(element->TFTDMode);
			}

			if (element->color != INT_MAX)
			{
				surface->setColor(element->color);
			}
			if (element->color2 != INT_MAX)
			{
				surface->setSecondaryColor(element->color2);
			}
			if (element->border != INT_MAX)
			{
				surface->setBorderColor(element->border);
			}
		}
	}

	if (bsbtn)
	{
		// this will initialize the graphics and settings of the battlescape button.
		bsbtn->copy(parent);
		bsbtn->initSurfaces();
	}

	// Set default text resources
	if (getGame()->getLanguage() && getGame()->getMod())
		surface->initText(getGame()->getMod()->getFont("FONT_BIG"), getGame()->getMod()->getFont("FONT_SMALL"), getGame()->getLanguage());

	_surfaces.push_back(entity);
}

/**
 * Returns whether this is a full-screen state.
 * This is used to optimize the state machine since full-screen
 * states automatically cover the whole screen, (whether they
 * actually use it all or not) so states behind them can be
 * safely ignored since they'd be covered up.
 * @return True if it's a screen, False otherwise.
 */
bool State::isScreen() const
{
	return _screen;
}

/**
 * Toggles the full-screen flag. Used by windows to
 * keep the previous screen in display while the window
 * is still "popping up".
 */
void State::toggleScreen()
{
	_screen = !_screen;
}

/**
 * Initializes the state and its child elements. This is
 * used for settings that have to be reset every time the
 * state is returned to focus (eg. palettes), so can't
 * just be put in the constructor (remember there's a stack
 * of states, so they can be created once while being
 * repeatedly switched back into focus).
 */
void State::init()
{
	getGame()->getScreen()->setPalette(_palette);
	getGame()->getCursor()->setPalette(_palette);
	getGame()->getCursor()->setColor(_cursorColor);
	getGame()->getCursor()->draw();
	getGame()->getFpsCounter()->setPalette(_palette);
	getGame()->getFpsCounter()->setColor(_cursorColor);
	getGame()->getFpsCounter()->draw();

	// Highest priority: custom sound set explicitly in the code
	// Medium priority: sound defined by the interface ruleset
	// Lowest priority: default window popup sound
	bool muteWindowPopupSound = false;
	if (!_soundPlayed)
	{
		_soundPlayed = true;
		if (!_customSound && _ruleInterface && _ruleInterface->getSound() != Mod::NO_SOUND)
		{
			_customSound = getGame()->getMod()->getSound("GEO.CAT", _ruleInterface->getSound());
		}
		if (_customSound)
		{
			muteWindowPopupSound = true;
			_customSound->play();
		}
	}

	for (entt::entity surfaceEnt : _surfaces)
	{
		if (getRegistry().raw().all_of<WindowComponent, SurfaceComponent>(surfaceEnt))
		{
			WindowComponent& windowComponent = getRegistry().raw().get<WindowComponent>(surfaceEnt);
			SurfaceComponent& surfaceComponent = getRegistry().raw().get<SurfaceComponent>(surfaceEnt);
			if (muteWindowPopupSound)
			{
				windowComponent.mute();
			}
			surfaceComponent.getSurface()->invalidate();
		}
	}
	if (_ruleInterface != 0 && !_ruleInterface->getMusic().empty())
	{
		getGame()->getMod()->playMusic(_ruleInterface->getMusic());
	}
}

/**
 * Runs any code the state needs to keep updating every
 * game cycle, like timers and other real-time elements.
 */
void State::think()
{
	for (entt::entity surfaceEnt : _surfaces)
	{
		Surface* surface = getRegistry().raw().get<SurfaceComponent>(surfaceEnt).getSurface();
		surface->think();
	}
}

/**
 * Takes care of any events from the core game engine,
 * and passes them on to its InteractiveSurface child elements.
 * @param action Pointer to an action.
 */
void State::handle(Action *action)
{
	if (!_modal)
	{
		for (std::vector<entt::entity>::reverse_iterator i = _surfaces.rbegin(); i != _surfaces.rend(); ++i)
		{
			Surface* surface = getRegistry().raw().get<SurfaceComponent>(*i).getSurface();
			InteractiveSurface* j = dynamic_cast<InteractiveSurface*>(surface);
			if (j != 0)
				j->handle(action, this);
		}
	}
	else
	{
		_modal->handle(action, this);
	}
}

/**
 * Blits all the visible Surface child elements onto the
 * display screen, by order of addition.
 */
void State::blit()
{
	DrawableSystem& drawable = getSystem<DrawableSystem>();
	for (entt::entity& surfaceEnt : _surfaces)
	{
		drawable.draw(surfaceEnt);
	}
}

/**
 * Hides all the Surface child elements on display.
 */
void State::hideAll()
{
	for (entt::entity surfaceEnt : _surfaces)
	{
		Surface* surface = getRegistry().raw().get<SurfaceComponent>(surfaceEnt).getSurface();
		surface->setHidden(true);
	}
}

/**
 * Shows all the hidden Surface child elements.
 */
void State::showAll()
{
	for (entt::entity surfaceEnt : _surfaces)
	{
		Surface* surface = getRegistry().raw().get<SurfaceComponent>(surfaceEnt).getSurface();
		surface->setHidden(false);
	}
}

/**
 * Resets the status of all the Surface child elements,
 * like unpressing buttons.
 */
void State::resetAll()
{
	for (entt::entity surfaceEnt : _surfaces)
	{
		Surface* surface = getRegistry().raw().get<SurfaceComponent>(surfaceEnt).getSurface();
		InteractiveSurface* s = dynamic_cast<InteractiveSurface*>(surface);
		if (s != 0)
		{
			s->unpress(this);
			//s->setFocus(false);
		}
	}
}

/**
 * Get the localized text for dictionary key @a id.
 * This function forwards the call to Language::getString(const std::string &).
 * @param id The dictionary key to search for.
 * @return The localized text.
 */
LocalizedText State::tr(const std::string &id) const
{
	return getGame()->getLanguage()->getString(id);
}

/**
* Get the localized text from dictionary.
* This function forwards the call to Language::getString(const std::string &).
* @param id The (prefix of) dictionary key to search for.
* @param alt Used to construct the (suffix of) dictionary key to search for.
* @return The localized text.
*/
LocalizedText State::trAlt(const std::string &id, int alt) const
{
	std::ostringstream ss;
	ss << id;
	// alt = 0 is the original, alt > 0 are the alternatives
	if (alt > 0)
	{
		ss << "_" << alt;
	}
	return getGame()->getLanguage()->getString(ss.str());
}

/**
 * Get a modifiable copy of the localized text for dictionary key @a id.
 * This function forwards the call to Language::getString(const std::string &, unsigned).
 * @param id The dictionary key to search for.
 * @param n The number to use for the proper version.
 * @return The localized text.
 */
LocalizedText State::tr(const std::string &id, unsigned n) const
{
	return getGame()->getLanguage()->getString(id, n);
}

/**
 * Get the localized text for dictionary key @a id.
 * This function forwards the call to Language::getString(const std::string &, SoldierGender).
 * @param id The dictionary key to search for.
 * @param gender Current soldier gender.
 * @return The localized text.
 */
LocalizedText State::tr(const std::string &id, SoldierGender gender) const
{
	return getGame()->getLanguage()->getString(id, gender);
}

/**
 * centers all the surfaces on the screen.
 */
void State::centerAllSurfaces()
{
	for (entt::entity surfaceEnt : _surfaces)
	{
		Surface* surface = getRegistry().raw().get<SurfaceComponent>(surfaceEnt).getSurface();
		surface->setX(surface->getX() + getGame()->getScreen()->getDX());
		surface->setY(surface->getY() + getGame()->getScreen()->getDY());
	}
}

/**
 * drop all the surfaces by half the screen height
 */
void State::lowerAllSurfaces()
{
	for (entt::entity surfaceEnt : _surfaces)
	{
		Surface* surface = getRegistry().raw().get<SurfaceComponent>(surfaceEnt).getSurface();
		surface->setY(surface->getY() + getGame()->getScreen()->getDY() / 2);
	}
}

/**
 * switch all the colours to something a little more battlescape appropriate.
 */
void State::applyBattlescapeTheme(const std::string& category)
{
	Element * element = getGame()->getMod()->getInterface("mainMenu")->getElement("battlescapeTheme");
	std::string altBg = getGame()->getMod()->getInterface(category)->getAltBackgroundImage();
	if (altBg.empty())
	{
		altBg = "TAC00.SCR";
	}
	for (entt::entity surfaceEnt : _surfaces)
	{
		Surface* surface = getRegistry().raw().get<SurfaceComponent>(surfaceEnt).getSurface();
		surface->setColor(element->color);
		surface->setHighContrast(true);


		if(getRegistry().raw().all_of<WindowComponent, SurfaceComponent>(surfaceEnt))
		{
			WindowComponent& windowComponent = getRegistry().raw().get<WindowComponent>(surfaceEnt);
			windowComponent.setBackground(getGame()->getMod()->getSurface(altBg));
		}
		TextList* list = dynamic_cast<TextList*>(surface);
		if (list)
		{
			list->setArrowColor(element->border);
		}
		ComboBox *combo = dynamic_cast<ComboBox*>(surface);
		if (combo)
		{
			combo->setArrowColor(element->border);
		}
	}
}

/**
 * redraw all the text-type surfaces.
 */
void State::redrawText()
{
	for (entt::entity surfaceEnt : _surfaces)
	{
		Surface* surface = getRegistry().raw().get<SurfaceComponent>(surfaceEnt).getSurface();
		Text* text = dynamic_cast<Text*>(surface);
		TextButton* button = dynamic_cast<TextButton*>(surface);
		TextEdit* edit = dynamic_cast<TextEdit*>(surface);
		TextList* list = dynamic_cast<TextList*>(surface);
		if (text || button || edit || list)
		{
			surface->draw();
		}
	}
}

/**
 * Changes the current modal surface. If a surface is modal,
 * then only that surface can receive events. This is used
 * when an element needs to take priority over everything else,
 * eg. focus.
 * @param surface Pointer to modal surface, NULL for no modal.
 */
void State::setModal(InteractiveSurface *surface)
{
	_modal = surface;
}

/**
 * Replaces a certain amount of colors in the state's palette.
 * @param colors Pointer to the set of colors.
 * @param firstcolor Offset of the first color to replace.
 * @param ncolors Amount of colors to replace.
 */
void State::setStatePalette(const SDL_Color *colors, int firstcolor, int ncolors)
{
	if (colors)
	{
		memcpy(_palette + firstcolor, colors, ncolors * sizeof(SDL_Color));
	}
}

/**
 * Set palette for helper surfaces like cursor or fps counter.
 */
void State::setModPalette()
{
	{
		getGame()->getCursor()->setPalette(_palette);
		getGame()->getCursor()->draw();
		getGame()->getFpsCounter()->setPalette(_palette);
		getGame()->getFpsCounter()->draw();
	}
}

/**
 * Loads palettes from the game resources into the state.
 * @param palette String ID of the palette to load.
 * @param backpals BACKPALS.DAT offset to use.
 */
void State::setStandardPalette(const std::string &palette, int backpals)
{
	setStatePalette(getGame()->getMod()->getPalette(palette)->getColors(), 0, 256);
	if (palette == "PAL_GEOSCAPE")
	{
		_cursorColor = Mod::GEOSCAPE_CURSOR;
	}
	else if (palette == "PAL_BASESCAPE")
	{
		_cursorColor = Mod::BASESCAPE_CURSOR;
	}
	else if (palette == "PAL_UFOPAEDIA")
	{
		_cursorColor = Mod::UFOPAEDIA_CURSOR;
	}
	else if (palette == "PAL_GRAPHS")
	{
		_cursorColor = Mod::GRAPHS_CURSOR;
	}
	else
	{
		_cursorColor = Mod::BATTLESCAPE_CURSOR;
	}
	if (backpals != -1)
		setStatePalette(getGame()->getMod()->getPalette("BACKPALS.DAT")->getColors(Palette::blockOffset(backpals)), Palette::backPos, 16);
	setModPalette(); // delay actual update to the end
}

/**
* Loads palettes from the given resources into the state.
* @param colors Pointer to the set of colors.
* @param cursorColor Cursor color to use.
*/
void State::setCustomPalette(SDL_Color *colors, int cursorColor)
{
	setStatePalette(colors, 0, 256);
	_cursorColor = cursorColor;
	setModPalette(); // delay actual update to the end
}

/**
 * Returns the state's 8bpp palette.
 * @return Pointer to the palette's colors.
 */
SDL_Color *State::getPalette()
{
	return _palette;
}

/**
 * Each state will probably need its own resize handling,
 * so this space intentionally left blank
 * @param dX delta of X;
 * @param dY delta of Y;
 */
void State::resize(int &dX, int &dY)
{
	recenter(dX, dY);
}

/**
 * Re-orients all the surfaces in the state.
 * @param dX delta of X;
 * @param dY delta of Y;
 */
void State::recenter(int dX, int dY)
{
	for (entt::entity surfaceEnt : _surfaces)
	{
		Surface* surface = getRegistry().raw().get<SurfaceComponent>(surfaceEnt).getSurface();
		surface->setX(surface->getX() + dX / 2);
		surface->setY(surface->getY() + dY / 2);
	}
}

}
