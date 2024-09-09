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
#include "MainMenuState.h"
#include <sstream>
#include "../version.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Screen.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "NewGameState.h"
#include "NewBattleState.h"
#include "ListLoadState.h"
#include "OptionsVideoState.h"
#include "ModListState.h"
#include "../Engine/Options.h"
#include "../Engine/FileMap.h"
#include "../Engine/SDL2Helpers.h"
#include "../Entity/Interface/Interface.h"
#include <fstream>
#include <functional>

namespace OpenXcom
{

GoToMainMenuState::GoToMainMenuState(bool updateCheck)
	: State("GoToMainMenuState", true), _updateCheck(updateCheck)
{
	// empty
}

GoToMainMenuState::~GoToMainMenuState()
{
	// empty
}

void GoToMainMenuState::init()
{
	Screen::updateScale(Options::geoscapeScale, Options::baseXGeoscape, Options::baseYGeoscape, true);
	getGame()->getScreen()->resetDisplay(false);
	getGame()->setState(new MainMenuState(_updateCheck));
}

/**
 * Initializes all the elements in the Main Menu window.
 * @param updateCheck Perform update check?
 */
MainMenuState::MainMenuState(bool updateCheck) : State("MainMenuState", true)
{
#ifdef _WIN32
	_debugInVisualStudio = false;
#endif
	InterfaceFactory& factory = getGame()->getECS().getFactory<InterfaceFactory>();

	// Set palette
	setInterface("mainMenu");

	// Create objects

	// Window
	InterfaceFactory::CreateWindowParams windowParams{
		.name = "wndMainMenu",
		.x = 32,
		.y = 20,
		.width = 256,
		.height = 160,
		.popup = WindowPopup::POPUP_BOTH,
		.ruleID = "mainMenu",
	};
	_window = factory.createWindow(windowParams);
		
	// Title
	std::ostringstream title;
	title << tr("STR_OPENXCOM") << Unicode::TOK_NL_SMALL;
	title << "OpenXcom " << OPENXCOM_VERSION_SHORT << OPENXCOM_VERSION_GIT;
	InterfaceFactory::CreateTextParams textTitleParams{
		.name = "txtTitle",
		.text = title.str(),
		.x = 32,
		.y = 45,
		.width = 256,
		.height = 30,
		.align = TextHAlign::ALIGN_CENTER,
		.verticalAlign = TextVAlign::ALIGN_MIDDLE,
		.isSmall = false,
		.ruleID = "mainMenu",
		.parent = _window
	};

	_txtTitle = factory.createText(textTitleParams);

	// New Game
	std::function<void(Action*)> btnNewGameClick = [this](Action* action) { getGame()->pushState(new NewGameState); };
	InterfaceFactory::CreateTextButtonParams btnNewGameParams {
		.name = "btnNewGame",
		.text = tr("STR_NEW_GAME"),
		.x = 64,
		.y = 90,
		.width = 92,
		.height = 20,

		.onLeftClickCallback = btnNewGameClick,

		.ruleID = "mainMenu",
		.parent = _window
	};
	_btnNewGame = factory.createTextButton(btnNewGameParams);

	// New Battle
	std::function<void(Action*)> btnNewBattleClick = [this](Action* action)	{ getGame()->pushState(new NewBattleState); };
	InterfaceFactory::CreateTextButtonParams btnNewBattleParams{
		.name = "btnNewBattle",
		.text = tr("STR_NEW_BATTLE"),
		.x = 164,
		.y = 90,
		.width = 92,
		.height = 20,

		.onLeftClickCallback = btnNewBattleClick,

		.ruleID = "mainMenu",
		.parent = _window
	};
	_btnNewBattle = factory.createTextButton(btnNewBattleParams);

	// Load
	std::function<void(Action*)> btnLoadClick = [this](Action* action) { getGame()->pushState(new ListLoadState(OPT_MENU)); };
	InterfaceFactory::CreateTextButtonParams btnLoadParams{
		.name = "btnLoad",
		.text = tr("STR_LOAD_SAVED_GAME"),
		.x = 64,
		.y = 118,
		.width = 92,
		.height = 20,

		.onLeftClickCallback = btnLoadClick,

		.ruleID = "mainMenu",
		.parent = _window
	};
	_btnLoad = factory.createTextButton(btnLoadParams);

	// Options
	std::function<void(Action*)> btnOptionsClick = [this](Action* action) { getGame()->pushState(new OptionsVideoState(OPT_MENU)); };
	InterfaceFactory::CreateTextButtonParams btnOptionsParams{
		.name = "btnOptions",
		.text = tr("STR_OPTIONS"),
		.x = 164,
		.y = 118,
		.width = 92,
		.height = 20,

		.onLeftClickCallback = btnOptionsClick,

		.ruleID = "mainMenu",
		.parent = _window
	};
	_btnOptions = factory.createTextButton(btnOptionsParams);

	// Mods
	std::function<void(Action*)> btnModsClick = [this](Action* action) { getGame()->pushState(new ModListState); };
	InterfaceFactory::CreateTextButtonParams btnModsParams
	{
		.name = "btnMods",
		.text = tr("STR_MODS"),
		.x = 64,
		.y = 146,
		.width = 92,
		.height = 20,

		.onLeftClickCallback = btnModsClick,

		.ruleID = "mainMenu",
		.parent = _window
	};
	_btnMods = factory.createTextButton(btnModsParams);

	// Quit
	std::function<void(Action*)> btnQuitClick = [this](Action* action) { getGame()->quit(); };
	InterfaceFactory::CreateTextButtonParams btnQuitParams
	{
		.name = "btnQuit",
		.text = tr("STR_QUIT"),
		.x = 164,
		.y = 146,
		.width = 92,
		.height = 20,

		.onLeftClickCallback = btnQuitClick,

		.ruleID = "mainMenu",
		.parent = _window
	};
	_btnQuit = factory.createTextButton(btnQuitParams);


	add(_window, "window", "mainMenu");
}

void MainMenuState::init()
{
	State::init();
	if (Options::getLoadLastSave() && getGame()->getSavedGame()->getList(getGame()->getLanguage(), true).size() > 0)
	{
		Log(LOG_INFO) << "Loading last saved game";
		btnLoadClick(NULL);
	}
}

/**
 *
 */
MainMenuState::~MainMenuState()
{

}

/**
 * Opens the New Game window.
 * @param action Pointer to an action.
 */
void MainMenuState::btnNewGameClick(Action *)
{
	
}

/**
 * Opens the New Battle screen.
 * @param action Pointer to an action.
 */
void MainMenuState::btnNewBattleClick(Action *)
{
	getGame()->pushState(new NewBattleState);
}

/**
 * Opens the Load Game screen.
 * @param action Pointer to an action.
 */
void MainMenuState::btnLoadClick(Action *)
{
	getGame()->pushState(new ListLoadState(OPT_MENU));
}

/**
 * Opens the Options screen.
 * @param action Pointer to an action.
 */
void MainMenuState::btnOptionsClick(Action *)
{
	Options::backupDisplay();
	getGame()->pushState(new OptionsVideoState(OPT_MENU));
}

/**
* Opens the Mods screen.
* @param action Pointer to an action.
*/
void MainMenuState::btnModsClick(Action *)
{
	getGame()->pushState(new ModListState);
}

/**
 * Quits the game.
 * @param action Pointer to an action.
 */
void MainMenuState::btnQuitClick(Action *)
{
	getGame()->quit();
}

/**
 * Updates OpenXcom to the newest version.
 * @param action Pointer to an action.
 */
void MainMenuState::btnUpdateClick(Action*)
{
//#ifdef _WIN32
//	const std::string subdir = "v" + _newVersion + "/";
//
//#ifdef _WIN64
//	const std::string relativeExeZipFileName = _debugInVisualStudio ? "Debug/exe64.zip" : "exe64.zip";
//#else
//	const std::string relativeExeZipFileName = _debugInVisualStudio ? "Debug/exe.zip" : "exe.zip";
//#endif
//	const std::string relativeExeNewFileName = _debugInVisualStudio ? "Debug/OpenXcomEx.exe.new" : "OpenXcomEx.exe.new";
//
//	const std::string commonDirFilename = Options::getDataFolder() + "common";
//	const std::string commonZipFilename = Options::getDataFolder() + "common.zip";
//	const std::string commonZipUrl = "https://openxcom.org/oxce/" + subdir + "common.zip";
//
//	const std::string standardDirFilename = Options::getDataFolder() + "standard";
//	const std::string standardZipFilename = Options::getDataFolder() + "standard.zip";
//	const std::string standardZipUrl = "https://openxcom.org/oxce/" + subdir + "standard.zip";
//
//	const std::string now = CrossPlatform::now();
//
//	const std::string exePath = CrossPlatform::getExeFolder();
//	const std::string exeFilenameOnly = CrossPlatform::getExeFilename(false);
//	const std::string exeFilenameFullPath = CrossPlatform::getExeFilename(true);
//
//#ifdef _WIN64
//	const std::string exeZipFilename = exePath + "exe64.zip";
//#else
//	const std::string exeZipFilename = exePath + "exe.zip";
//#endif
//	const std::string exeNewFilename = exePath + "OpenXcomEx.exe.new";
//#ifdef _WIN64
//	const std::string exeZipUrl = "https://openxcom.org/oxce/" + subdir + "exe64.zip";
//#else
//	const std::string exeZipUrl = "https://openxcom.org/oxce/" + subdir + "exe.zip";
//#endif
//
//	// stop using the common/standard zip files, so that we can back them up
//	FileMap::clear(true, false);
//
//	// 0. backup the exe
//	if (CrossPlatform::fileExists(exeFilenameFullPath))
//	{
//		if (CrossPlatform::copyFile(exeFilenameFullPath, exeFilenameFullPath + "-" + now + ".bak"))
//			Log(LOG_INFO) << "Update step 0 done.";
//		else return;
//	}
//
//	// 1. backup common dir
//	if (CrossPlatform::fileExists(commonDirFilename))
//	{
//		if (CrossPlatform::moveFile(commonDirFilename, commonDirFilename + "-" + now))
//			Log(LOG_INFO) << "Update step 1 done.";
//		else return;
//	}
//
//	// 2. backup common zip
//	if (CrossPlatform::fileExists(commonZipFilename))
//	{
//		if (CrossPlatform::moveFile(commonZipFilename, commonZipFilename + "-" + now + ".bak"))
//			Log(LOG_INFO) << "Update step 2 done.";
//		else return;
//	}
//
//	// 3. backup standard dir
//	if (CrossPlatform::fileExists(standardDirFilename))
//	{
//		if (CrossPlatform::moveFile(standardDirFilename, standardDirFilename + "-" + now))
//			Log(LOG_INFO) << "Update step 3 done.";
//		else return;
//	}
//
//	// 4. backup standard zip
//	if (CrossPlatform::fileExists(standardZipFilename))
//	{
//		if (CrossPlatform::moveFile(standardZipFilename, standardZipFilename + "-" + now + ".bak"))
//			Log(LOG_INFO) << "Update step 4 done.";
//		else return;
//	}
//
//	// 5. delete exe zip
//	if (CrossPlatform::fileExists(exeZipFilename))
//	{
//		if (CrossPlatform::deleteFile(exeZipFilename))
//			Log(LOG_INFO) << "Update step 5 done.";
//		else return;
//	}
//
//	// 6. delete unpacked exe zip
//	if (CrossPlatform::fileExists(exeNewFilename))
//	{
//		if (CrossPlatform::deleteFile(exeNewFilename))
//			Log(LOG_INFO) << "Update step 6 done.";
//		else return;
//	}
//
//	// 7. download common zip
//	if (CrossPlatform::downloadFile(commonZipUrl, commonZipFilename))
//	{
//		Log(LOG_INFO) << "Update step 7 done.";
//	}
//	else return;
//
//	// 8. download standard zip
//	if (CrossPlatform::downloadFile(standardZipUrl, standardZipFilename))
//	{
//		Log(LOG_INFO) << "Update step 8 done.";
//	}
//	else return;
//
//	// 9. download exe zip
//	if (CrossPlatform::downloadFile(exeZipUrl, exeZipFilename))
//	{
//		Log(LOG_INFO) << "Update step 9 done.";
//	}
//	else return;
//
//	// 10. extract exe zip
//	if (CrossPlatform::fileExists(exeZipFilename) && CrossPlatform::fileExists(relativeExeZipFileName))
//	{
//		const std::string file_to_extract = "OpenXcomEx.exe.new";
//		SDL_RWops *rwo_read = FileMap::zipGetFileByName(relativeExeZipFileName, file_to_extract);
//		if (!rwo_read) {
//			Log(LOG_ERROR) << "Step 10a: failed to unzip file.";
//			return;
//		}
//		size_t size = 0;
//		auto data = SDL_LoadFile_RW(rwo_read, &size, SDL_TRUE);
//		if (!data) {
//			Log(LOG_ERROR) << "Step 10b: failed to unzip file." << SDL_GetError(); // out of memory for a copy ?
//			return;
//		}
//		SDL_RWops *rwo_write = SDL_RWFromFile(relativeExeNewFileName.c_str(), "wb");
//		if (!rwo_write) {
//			Log(LOG_ERROR) << "Step 10c: failed to open exe.new file for writing." << SDL_GetError();
//			return;
//		}
//		auto wsize = SDL_RWwrite(rwo_write, data, (int)size, 1);
//		if (wsize != 1) {
//			Log(LOG_ERROR) << "Step 10d: failed to write exe.new file." << SDL_GetError();
//			return;
//		}
//		if (SDL_RWclose(rwo_write)) {
//			Log(LOG_ERROR) << "Step 10e: failed to write exe.new file." << SDL_GetError();
//			return;
//		}
//	} else {
//		Log(LOG_ERROR) << "Update step 10 failed."; // exe dir and working dir not the same
//		return;
//	}
//
//	// 11. check if extracted exe exists
//	if (!CrossPlatform::fileExists(exeNewFilename))
//	{
//		Log(LOG_ERROR) << "Update step 11 failed.";
//		return;
//	}
//
//	// 12. delete exe zip (again)
//	if (CrossPlatform::fileExists(exeZipFilename))
//	{
//		if (CrossPlatform::deleteFile(exeZipFilename))
//			Log(LOG_INFO) << "Update step 12 done.";
//		else return;
//	}
//
//	// 13. create the update batch file
//	{
//		std::ofstream batch;
//		batch.open("oxce-upd.bat", std::ios::out);
//
//		batch << "@echo OFF\n";
//		batch << "echo OpenXcom is updating, please wait...\n";
//		batch << "timeout 5\n";
//		if (!_debugInVisualStudio)
//		{
//			batch << "echo Removing the old version...\n";
//			batch << "del " << exeFilenameOnly << "\n";
//			batch << "echo Preparing the new version...\n";
//			batch << "ren OpenXcomEx.exe.new " << exeFilenameOnly << "\n";
//			batch << "echo Starting the new version...\n";
//			batch << "timeout 2\n";
//			batch << "start " << exeFilenameOnly << "\n"; // asynchronous
//			batch << "exit\n";
//		}
//
//		batch.close();
//	}
//
//	// 14. Clear the SDL event queue (i.e. ignore input from impatient users)
//	SDL_Event e;
//	while (SDL_PollEvent(&e))
//	{
//		// do nothing
//	}
//
//	Log(LOG_INFO) << "Update prepared, restarting.";
//	getGame()->setUpdateFlag(true);
//	getGame()->quit();
//#endif
}

/**
 * Updates the scale.
 * @param dX delta of X;
 * @param dY delta of Y;
 */
void MainMenuState::resize(int &dX, int &dY)
{
	dX = Options::baseXResolution;
	dY = Options::baseYResolution;
	Screen::updateScale(Options::geoscapeScale, Options::baseXGeoscape, Options::baseYGeoscape, true);
	dX = Options::baseXResolution - dX;
	dY = Options::baseYResolution - dY;
	State::resize(dX, dY);
}

}
