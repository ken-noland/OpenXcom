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
#include "BasescapeState.h"
#include "BaseInfoState.h"
#include "BaseView.h"
#include "BuildFacilitiesState.h"
#include "CraftInfoState.h"
#include "CraftsState.h"
#include "DismantleFacilityState.h"
#include "ManageAlienContainmentState.h"
#include "ManufactureState.h"
#include "MiniBaseView.h"
#include "PlaceFacilityState.h"
#include "PurchaseState.h"
#include "ResearchState.h"
#include "SellState.h"
#include "SoldiersState.h"
#include "TransferBaseState.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/BriefingState.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Unicode.h"
#include "../Entity/Common/GeoPosition.h"
#include "../Entity/Common/Type.h"
#include "../Entity/Game/BaseFactory.h"
#include "../Geoscape/AllocatePsiTrainingState.h"
#include "../Geoscape/AllocateTrainingState.h"
#include "../Geoscape/BuildNewBaseState.h"
#include "../Geoscape/Globe.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextEdit.h"
#include "../Menu/ErrorMessageState.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Mod/RuleGlobe.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleRegion.h"
#include "../Savegame/AreaSystem.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseSystem.h"
#include "../Savegame/BaseFacility.h"
#include "../Savegame/Region.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/SavedGame.h"
#include "../Ufopaedia/Ufopaedia.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Basescape screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param globe Pointer to the Geoscape globe.
 */
BasescapeState::BasescapeState(BasescapeSystem& basescapeSystem, Globe *globe) : State("BasescapeState", true),
	_globe(globe), _basescapeSystem(basescapeSystem)
{
	_unsubscribeId = _basescapeSystem.connectListener(std::bind(&BasescapeState::init, this));

	// Create objects
	_txtFacility = new Text(192, 9, 0, 0);
	_view = new BaseView(192, 192, 0, 8);
	_mini = new MiniBaseView(_basescapeSystem, 128, 16, 192, 41);
	_edtBase = new TextEdit(this, 127, 17, 193, 0);
	_txtLocation = new Text(126, 9, 194, 16);
	_leftArrow = new Text(7, 9, 192, 32); 
	_rightArrow = new Text(7, 9, 310, 32); 	
	_txtFunds = new Text(126, 9, 194, 24);
	_btnNewBase = new TextButton(128, 12, 192, 58);
	_btnBaseInfo = new TextButton(128, 12, 192, 71);
	_btnSoldiers = new TextButton(128, 12, 192, 84);
	_btnCrafts = new TextButton(128, 12, 192, 97);
	_btnFacilities = new TextButton(128, 12, 192, 110);
	_btnResearch = new TextButton(128, 12, 192, 123);
	_btnManufacture = new TextButton(128, 12, 192, 136);
	_btnTransfer = new TextButton(128, 12, 192, 149);
	_btnPurchase = new TextButton(128, 12, 192, 162);
	_btnSell = new TextButton(128, 12, 192, 175);
	_btnGeoscape = new TextButton(128, 12, 192, 188);

	// Set palette
	setInterface("basescape");

	add(_view, "baseView", "basescape");
	add(_mini, "miniBase", "basescape");
	add(_txtFacility, "textTooltip", "basescape");
	add(_edtBase, "text1", "basescape");
	add(_txtLocation, "text2", "basescape");
	add(_txtFunds, "text3", "basescape");
	add(_leftArrow,"text3", "basescape");
	add(_rightArrow,"text3", "basescape");	
	add(_btnNewBase, "button", "basescape");
	add(_btnBaseInfo, "button", "basescape");
	add(_btnSoldiers, "button", "basescape");
	add(_btnCrafts, "button", "basescape");
	add(_btnFacilities, "button", "basescape");
	add(_btnResearch, "button", "basescape");
	add(_btnManufacture, "button", "basescape");
	add(_btnTransfer, "button", "basescape");
	add(_btnPurchase, "button", "basescape");
	add(_btnSell, "button", "basescape");
	add(_btnGeoscape, "button", "basescape");

	centerAllSurfaces();

	// Set up objects
	if (Element* itf = getGame()->getMod()->getInterface("basescape")->getElement("trafficLights"))
	{
		_view->setOtherColors(itf->color, itf->color2, itf->border, !itf->TFTDMode);
	}
	_view->setTexture(getGame()->getMod()->getSurfaceSet("BASEBITS.PCK"));
	_view->onMouseClick((ActionHandler)&BasescapeState::viewLeftClick, SDL_BUTTON_LEFT);
	_view->onMouseClick((ActionHandler)&BasescapeState::viewRightClick, SDL_BUTTON_RIGHT);
	_view->onMouseClick((ActionHandler)&BasescapeState::viewMiddleClick, SDL_BUTTON_MIDDLE);
	_view->onMouseOver((ActionHandler)&BasescapeState::viewMouseOver);
	_view->onMouseOut((ActionHandler)&BasescapeState::viewMouseOut);

	_mini->setTexture(getGame()->getMod()->getSurfaceSet("BASEBITS.PCK"));

	_edtBase->setBig();
	_edtBase->onChange((ActionHandler)&BasescapeState::edtBaseChange);

	_btnNewBase->setText(tr("STR_BUILD_NEW_BASE_UC"));
	_btnNewBase->onMouseClick((ActionHandler)&BasescapeState::btnNewBaseClick);
	_btnNewBase->onKeyboardPress((ActionHandler)&BasescapeState::btnNewBaseClick, Options::keyBasescapeBuildNewBase);

	_btnBaseInfo->setText(tr("STR_BASE_INFORMATION"));
	_btnBaseInfo->onMouseClick((ActionHandler)&BasescapeState::btnBaseInfoClick);
	_btnBaseInfo->onKeyboardPress((ActionHandler)&BasescapeState::btnBaseInfoClick, Options::keyBasescapeBaseInfo);

	_btnSoldiers->setText(tr("STR_SOLDIERS_UC"));
	_btnSoldiers->onMouseClick((ActionHandler)&BasescapeState::btnSoldiersClick);
	_btnSoldiers->onKeyboardPress((ActionHandler)&BasescapeState::btnSoldiersClick, Options::keyBasescapeSoldiers);

	_btnCrafts->setText(tr("STR_EQUIP_CRAFT"));
	_btnCrafts->onMouseClick((ActionHandler)&BasescapeState::btnCraftsClick);
	_btnCrafts->onKeyboardPress((ActionHandler)&BasescapeState::btnCraftsClick, Options::keyBasescapeCrafts);

	_btnFacilities->setText(tr("STR_BUILD_FACILITIES"));
	_btnFacilities->onMouseClick((ActionHandler)&BasescapeState::btnFacilitiesClick);
	_btnFacilities->onKeyboardPress((ActionHandler)&BasescapeState::btnFacilitiesClick, Options::keyBasescapeFacilities);

	_btnResearch->setText(tr("STR_RESEARCH"));
	_btnResearch->onMouseClick((ActionHandler)&BasescapeState::btnResearchClick);
	_btnResearch->onKeyboardPress((ActionHandler)&BasescapeState::btnResearchClick, Options::keyBasescapeResearch);

	_btnManufacture->setText(tr("STR_MANUFACTURE"));
	_btnManufacture->onMouseClick((ActionHandler)&BasescapeState::btnManufactureClick);
	_btnManufacture->onKeyboardPress((ActionHandler)&BasescapeState::btnManufactureClick, Options::keyBasescapeManufacture);

	_btnTransfer->setText(tr("STR_TRANSFER_UC"));
	_btnTransfer->onMouseClick((ActionHandler)&BasescapeState::btnTransferClick);
	_btnTransfer->onKeyboardPress((ActionHandler)&BasescapeState::btnTransferClick, Options::keyBasescapeTransfer);

	_btnPurchase->setText(tr("STR_PURCHASE_RECRUIT"));
	_btnPurchase->onMouseClick((ActionHandler)&BasescapeState::btnPurchaseClick);
	_btnPurchase->onKeyboardPress((ActionHandler)&BasescapeState::btnPurchaseClick, Options::keyBasescapePurchase);

	_btnSell->setText(tr("STR_SELL_SACK_UC"));
	_btnSell->onMouseClick((ActionHandler)&BasescapeState::btnSellClick);
	_btnSell->onKeyboardPress((ActionHandler)&BasescapeState::btnSellClick, Options::keyBasescapeSell);

	_btnGeoscape->setText(tr("STR_GEOSCAPE_UC"));
	_btnGeoscape->onMouseClick((ActionHandler)&BasescapeState::btnGeoscapeClick);
	_btnGeoscape->onKeyboardPress((ActionHandler)&BasescapeState::btnGeoscapeClick, Options::keyCancel);
}

/**
 *
 */
BasescapeState::~BasescapeState()
{
	_basescapeSystem.disconnectListener(_unsubscribeId);
	// We should have no temporary bases.
	// Clean up any temporary bases
	// 
	// bool exists = false;
	// for (const Base* xbase : getGame()->getSavedGame()->getBases())
	// {
	// 	if (xbase == _base)
	// 	{
	// 		exists = true;
	// 		break;
	// 	}
	// }
	// if (!exists)
	// {
	// 	delete _base;
	// }
}

/**
 * The player can change the selected base
 * or change info on other screens.
 */
void BasescapeState::init()
{
	State::init();

	entt::handle selectedBase = _basescapeSystem.getSelectedBase();
	_view->setBase(&selectedBase.get<Base>());
	updateArrows();	
	_mini->draw();
	_edtBase->setText(selectedBase.get<Base>().getName());

	if (const GeoPosition* position = selectedBase.try_get<GeoPosition>(); position->region)
	{
		_txtLocation->setText(tr(position->region.get<Type>()));
	}

	_txtFunds->setText(tr("STR_FUNDS").arg(Unicode::formatFunding(getGame()->getSavedGame()->getFunds())));
	_btnNewBase->setVisible(_basescapeSystem.getBaseCount() < Options::maxNumberOfBases);

	if (!getGame()->getMod()->getNewBaseUnlockResearch().empty()
		&& !getGame()->getSavedGame()->isResearched(getGame()->getMod()->getNewBaseUnlockResearch(), true)) {
		_btnNewBase->setVisible(false);
	}
}

/**
 * Goes to the Build New Base screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnNewBaseClick(Action *)
{
	entt::handle newBaseHandle = getRegistry().getService<BaseFactory>().create(*getGame()->getMod());

	getGame()->popState();
	getGame()->pushState(new BuildNewBaseState(newBaseHandle, _globe, false));
}

/**
 * Goes to the Base Info screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnBaseInfoClick(Action *) 
{
	getGame()->pushState(new BaseInfoState(_basescapeSystem));
}

/**
 * Goes to the Soldiers screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnSoldiersClick(Action *)
{
	getGame()->pushState(new SoldiersState(&_basescapeSystem.getSelectedBase().get<Base>()));
}

/**
 * Goes to the Crafts screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnCraftsClick(Action *)
{
	getGame()->pushState(new CraftsState(&_basescapeSystem.getSelectedBase().get<Base>()));
}

/**
 * Opens the Build Facilities window.
 * @param action Pointer to an action.
 */
void BasescapeState::btnFacilitiesClick(Action *)
{
	getGame()->pushState(new BuildFacilitiesState(_basescapeSystem.getSelectedBase(), this));
}

/**
 * Goes to the Research screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnResearchClick(Action *)
{
	getGame()->pushState(new ResearchState(&_basescapeSystem.getSelectedBase().get<Base>()));
}

/**
 * Goes to the Manufacture screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnManufactureClick(Action *)
{
	getGame()->pushState(new ManufactureState(&_basescapeSystem.getSelectedBase().get<Base>()));
}

/**
 * Goes to the Purchase screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnPurchaseClick(Action *)
{
	getGame()->pushState(new PurchaseState(&_basescapeSystem.getSelectedBase().get<Base>()));
}

/**
 * Goes to the Sell screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnSellClick(Action *)
{
	getGame()->pushState(new SellState(&_basescapeSystem.getSelectedBase().get<Base>(), 0));
}

/**
 * Goes to the Select Destination Base window.
 * @param action Pointer to an action.
 */
void BasescapeState::btnTransferClick(Action *)
{
	getGame()->pushState(new TransferBaseState(&_basescapeSystem.getSelectedBase().get<Base>(), nullptr));
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnGeoscapeClick(Action *) { getGame()->popState(); }

/**
 * Processes clicking on facilities.
 * @param action Pointer to an action.
 */
void BasescapeState::viewLeftClick(Action *)
{
	entt::handle selectedBaseHandle = _basescapeSystem.getSelectedBase();
	Base& base = selectedBaseHandle.get<Base>();
	if (BaseFacility* fac = _view->getSelectedFacility())
	{
		if (getGame()->isCtrlPressed() && Options::isPasswordCorrect())
		{
			// Ctrl + left click on a base facility allows moving it
			getGame()->pushState(new PlaceFacilityState(_basescapeSystem.getSelectedBase(), fac->getRules(), fac));
		}
		else
		{
			if (fac->getRules()->isLift() && base.getFacilities().size() > 1)
			{
				// Note: vehicles will not be deployed in the base preview
				if (base.getAvailableSoldiers(true, true) > 0/* || !base.getVehicles()->empty()*/)
				{
					int texture, shade;
					_globe->getPolygonTextureAndShade(base.getLongitude(), base.getLatitude(), &texture, &shade);
					auto* globeTexture = getGame()->getMod()->getGlobe()->getTexture(texture);

					SavedBattleGame* bgame = new SavedBattleGame(getGame()->getMod(), getGame()->getLanguage(), true);
					getGame()->getSavedGame()->setBattleGame(bgame);
					BattlescapeGenerator bgen = BattlescapeGenerator(getGame());
					bgame->setMissionType("STR_BASE_DEFENSE");
					bgen.setBase(&base);
					bgen.setWorldTexture(globeTexture, globeTexture);
					bgen.run();

					getGame()->pushState(new BriefingState(0, &base));
				}
				return;
			}
			int errorColor1 = getGame()->getMod()->getInterface("basescape")->getElement("errorMessage")->color;
			int errorColor2 = getGame()->getMod()->getInterface("basescape")->getElement("errorPalette")->color;
			// Is facility in use?
			if (BasePlacementErrors placementErrorCode = fac->inUse())
			{
				switch (placementErrorCode)
				{
				case BPE_Used_Stores:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_STORAGE"), _palette, errorColor1, "BACK13.SCR", errorColor2));
					break;
				case BPE_Used_Quarters:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_QUARTERS"), _palette, errorColor1, "BACK13.SCR", errorColor2));
					break;
				case BPE_Used_Laboratories:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_LABORATORIES"), _palette, errorColor1, "BACK13.SCR", errorColor2));
					break;
				case BPE_Used_Workshops:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_WORKSHOPS"), _palette, errorColor1, "BACK13.SCR", errorColor2));
					break;
				case BPE_Used_Hangars:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_HANGARS"), _palette, errorColor1, "BACK13.SCR", errorColor2));
					break;
				case BPE_Used_PsiLabs:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_PSI_LABS"), _palette, errorColor1, "BACK13.SCR", errorColor2));
					break;
				case BPE_Used_Gyms:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_GYMS"), _palette, errorColor1, "BACK13.SCR", errorColor2));
					break;
				case BPE_Used_AlienContainment:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE_PRISONS"), _palette, errorColor1, "BACK13.SCR", errorColor2));
					break;
				default:
					getGame()->pushState(new ErrorMessageState(tr("STR_FACILITY_IN_USE"), _palette, errorColor1, "BACK13.SCR", errorColor2));
				}
			}
			// Would base become disconnected?
			else if (!base.getDisconnectedFacilities(fac).empty() && fac->getRules()->getLeavesBehindOnSell().size() == 0)
			{
				getGame()->pushState(new ErrorMessageState(tr("STR_CANNOT_DISMANTLE_FACILITY"), _palette, errorColor1, "BACK13.SCR", errorColor2));
			}
			// Is this facility being built from a dismantled one or building over a previous building?
			else if (fac->getBuildTime() > 0 && fac->getIfHadPreviousFacility())
			{
				getGame()->pushState(new ErrorMessageState(tr("STR_CANNOT_DISMANTLE_FACILITY_UPGRADING"), _palette, errorColor1, "BACK13.SCR", errorColor2));
			}
			else
			{
				getGame()->pushState(new DismantleFacilityState(&base, _view, fac));
			}
		}
	}
}

/**
 * Processes right clicking on facilities.
 * @param action Pointer to an action.
 */
void BasescapeState::viewRightClick(Action *)
{
	entt::handle selectedBaseHandle = _basescapeSystem.getSelectedBase();
	Base& base = selectedBaseHandle.get<Base>();
	BaseFacility *f = _view->getSelectedFacility();
	if (f == 0)
	{
		getGame()->pushState(new BaseInfoState(_basescapeSystem));
	}
	else if (f->getRules()->getRightClickActionType() != 0)
	{
		switch (f->getRules()->getRightClickActionType())
		{
			case 1: getGame()->pushState(new ManageAlienContainmentState(&base, f->getRules()->getPrisonType(), OPT_GEOSCAPE)); break;
			case 2: getGame()->pushState(new ManufactureState(&base)); break;
			case 3: getGame()->pushState(new ResearchState(&base)); break;
			case 4: getGame()->pushState(new AllocateTrainingState(&base)); break;
			case 5: if (Options::anytimePsiTraining) getGame()->pushState(new AllocatePsiTrainingState(&base)); break;
			case 6: getGame()->pushState(new SoldiersState(&base)); break;
			case 7: getGame()->pushState(new SellState(&base, 0)); break;
			default: getGame()->popState(); break;
		}
	}
	else if (f->getRules()->isMindShield())
	{
		if (f->getBuildTime() == 0)
		{
			f->setDisabled(!f->getDisabled());
			_view->draw();
			_mini->draw();
		}
	}
	else if (f->getRules()->getCrafts() > 0)
	{
		if (f->getCraftsForDrawing().empty())
		{
			getGame()->pushState(new CraftsState(&base));
		}
		else
			for (size_t craft = 0; craft < base.getCrafts().size(); ++craft)
			{
				if (_view->getSelectedCraft() == base.getCrafts().at(craft))
				{
					getGame()->pushState(new CraftInfoState(&base, craft));
					break;
				}
			}
	}
	else if (f->getRules()->getStorage() > 0)
	{
		getGame()->pushState(new SellState(&base, 0));
	}
	else if (f->getRules()->getPersonnel() > 0)
	{
		getGame()->pushState(new SoldiersState(&base));
	}
	else if (f->getRules()->getPsiLaboratories() > 0 && Options::anytimePsiTraining && base.getAvailablePsiLabs() > 0)
	{
		getGame()->pushState(new AllocatePsiTrainingState(&base));
	}
	else if (f->getRules()->getTrainingFacilities() > 0 && base.getAvailableTraining() > 0)
	{
		getGame()->pushState(new AllocateTrainingState(&base));
	}
	else if (f->getRules()->getLaboratories() > 0)
	{
		getGame()->pushState(new ResearchState(&base));
	}
	else if (f->getRules()->getWorkshops() > 0)
	{
		getGame()->pushState(new ManufactureState(&base));
	}
	else if (f->getRules()->getAliens() > 0)
	{
		getGame()->pushState(new ManageAlienContainmentState(&base, f->getRules()->getPrisonType(), OPT_GEOSCAPE));
	}
	else if (f->getRules()->isLift() || f->getRules()->getRadarRange() > 0)
	{
		getGame()->popState();
	}
}

/**
* Opens the corresponding Ufopaedia article.
* @param action Pointer to an action.
*/
void BasescapeState::viewMiddleClick(Action *)
{
	if (BaseFacility* f = _view->getSelectedFacility())
	{
		std::string articleId = f->getRules()->getType();
		Ufopaedia::openArticle(getGame(), articleId);
	}
}

/**
 * Displays the name of the facility the mouse is over.
 * @param action Pointer to an action.
 */
void BasescapeState::viewMouseOver(Action *)
{
	std::ostringstream ss;
	if (BaseFacility* f = _view->getSelectedFacility())
	{
		if (f->getRules()->getCrafts() == 0 || f->getBuildTime() > 0)
		{
			ss << tr(f->getRules()->getType());
		}
		else
		{
			ss << tr(f->getRules()->getType());
			if (!(f->getCraftsForDrawing().empty()))
			{
				ss << " " << tr("STR_CRAFT_").arg(_view->getSelectedCraft()->getName(getGame()->getLanguage()));
			}
		}
	}
	_txtFacility->setText(ss.str());
}

/**
 * Clears the facility name.
 * @param action Pointer to an action.
 */
void BasescapeState::viewMouseOut(Action *) { _txtFacility->setText(""); }

/**
 * Moves the current base to the left in the list of bases.
 * @param action Pointer to an action.
 */
void BasescapeState::miniMiddleClick(Action *)
{
	// We cannot support this yet.
//	size_t baseIndex = _mini->getHoveredBaseIndex() + _mini->getVisibleBasesIndexOffset();
//	size_t numBases = getGame()->getSavedGame()->getRegistry().view<Base>().size();
//
//	if (baseIndex > 0 && baseIndex < numBases)
//	{
//		std::vector<Base*>& bases = getGame()->getSavedGame()->getBases();
//
//		// only able to move the currently selected base
//		if (bases[baseIndex] == _base)
//		{
//			std::swap(bases[baseIndex], bases[baseIndex - 1]);
//			init();
//		}
//	}
}

/**
 * Changes the Base name.
 * @param action Pointer to an action.
 */
void BasescapeState::edtBaseChange(Action *)
{
	_basescapeSystem.getSelectedBase().get<Base>().setName(_edtBase->getText());
}

/**
 *update visibility of arrows
 */
void BasescapeState::updateArrows()
{
	BasescapeData basescapeData = _basescapeSystem.getBasescapeData();
	size_t baseCount = _basescapeSystem.getBaseCount();

	std::string leftArrowText = basescapeData.visibleBasesOffset > 0 ? "<" : "";
	_leftArrow->setText(leftArrowText);

	std::string rightArrowText = baseCount - basescapeData.visibleBasesOffset > BasescapeData::MAX_VISIBLE_BASES ? ">" : "";
	_leftArrow->setText(rightArrowText);
}

}
