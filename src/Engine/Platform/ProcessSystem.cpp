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
#include "ProcessSystem.h"

namespace OpenXcom
{

PlatformProcessSystem::PlatformProcessSystem() : _isRunning(true)
{
}

PlatformProcessSystem::~PlatformProcessSystem()
{
}

void PlatformProcessSystem::update()
{
	platformSpecificUpdate();
}

void PlatformProcessSystem::exit(int exitCode)
{
	platformSpecificExit(exitCode);
}

} // namespace OpenXcom


/*
*
*/

//	std::string errorDlg;

/**
 * Determines the available Linux error dialogs.
 */
//void getErrorDialog()
//{
//#ifndef _WIN32
//	if (system(NULL))
//	{
//		if (getenv("KDE_SESSION_UID") && system("which kdialog > /dev/null 2>&1") == 0)
//			errorDlg = "kdialog --error ";
//		else if (system("which zenity > /dev/null 2>&1") == 0)
//			errorDlg = "zenity --no-wrap --error --text=";
//		else if (system("which kdialog > /dev/null 2>&1") == 0)
//			errorDlg = "kdialog --error ";
//		else if (system("which gdialog > /dev/null 2>&1") == 0)
//			errorDlg = "gdialog --msgbox ";
//		else if (system("which xdialog > /dev/null 2>&1") == 0)
//			errorDlg = "xdialog --msgbox ";
//	}
//#endif
//}

/**
 * Displays a message box with an error message.
 * @param error Error message.
 */
//void showError(const std::string& error)
//{
//#ifdef _WIN32
//	assert(!"Not implemented");
//	// auto titleW = pathToWindows("OpenXcom Error", false);
//	// auto errorW = pathToWindows(error, false);
//	// MessageBoxW(NULL, errorW.c_str(), titleW.c_str(), MB_ICONERROR | MB_OK);
//#else
//	if (errorDlg.empty())
//	{
//		std::cerr << error << std::endl;
//	}
//	else
//	{
//		std::string nError = '"' + error + '"';
//		Unicode::replace(nError, "\n", "\\n");
//		std::string cmd = errorDlg + nError;
//		if (system(cmd.c_str()) != 0)
//			std::cerr << error << std::endl;
//	}
//#endif
//	Log(LOG_FATAL) << error;
//}
