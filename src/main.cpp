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
#include <sstream>
#include <exception>
#include <cassert>
#include "version.h"
#include "Engine/Exception.h"
#include "Engine/Logger.h"
#include "Engine/CrossPlatform.h"
#include "Engine/Game.h"
#include "Engine/Options.h"
#include "Engine/FileMap.h"
#include "Menu/StartState.h"

using namespace OpenXcom;

// Crash handling routines
#ifdef _MSC_VER
#include <windows.h>
LONG WINAPI crashLogger(PEXCEPTION_POINTERS exception)
{
	CrossPlatform::crashDump(exception, "");
	return EXCEPTION_CONTINUE_SEARCH;
}
#else
#include <signal.h>
void signalLogger(int sig)
{
	CrossPlatform::crashDump(&sig, "");
	exit(EXIT_FAILURE);
}

#endif

void exceptionLogger()
{
	static bool logged = false;
	std::string error;
	try
	{
		if (!logged)
		{
			logged = true;
			throw;
		}
	}
	catch (const std::exception &e)
	{
		error = e.what();
	}
	catch (...)
	{
		error = "Unknown exception";
	}
	CrossPlatform::crashDump(0, error);
	exit(EXIT_FAILURE);
}

#ifdef _MSC_VER
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <io.h>
#include <fcntl.h>

std::string WideStringToString(const std::wstring& wstr)
{
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string str(bufferSize, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], bufferSize, nullptr, nullptr);
	return str;
}

std::vector<std::string> CommandLineToArgvA()
{
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argv == NULL) {
		exit(EXIT_FAILURE);
	}

	std::vector<std::string> args;
	for (int i = 0; i < argc; i++) {
		args.push_back(WideStringToString(argv[i]));
	}

	LocalFree(argv);

	return args;
}

void AttachParentConsole()
{
	// Allocate a console for this application
	if (!AttachConsole(ATTACH_PARENT_PROCESS))
	{
		AllocConsole();
	}
	// Set the screen buffer size
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	csbi.dwSize.Y = 500;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), csbi.dwSize);

	// Redirect unbuffered STDOUT to the console
	HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrtOut = _open_osfhandle((intptr_t)hConsoleOutput, _O_TEXT);
	FILE* fpOut = _fdopen(hCrtOut, "w");
	freopen_s(&fpOut, "CONOUT$", "w", stdout);
	setvbuf(stdout, NULL, _IONBF, 0);

	// Redirect unbuffered STDIN to the console
	HANDLE hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
	int hCrtIn = _open_osfhandle((intptr_t)hConsoleInput, _O_TEXT);
	FILE* fpIn = _fdopen(hCrtIn, "r");
	freopen_s(&fpIn, "CONIN$", "r", stdin);
	setvbuf(stdin, NULL, _IONBF, 0);

	// Redirect unbuffered STDERR to the console
	HANDLE hConsoleError = GetStdHandle(STD_ERROR_HANDLE);
	int hCrtErr = _open_osfhandle((intptr_t)hConsoleError, _O_TEXT);
	FILE* fpErr = _fdopen(hCrtErr, "w");
	freopen_s(&fpErr, "CONOUT$", "w", stderr);
	setvbuf(stderr, NULL, _IONBF, 0);

	// Synchronize C++ streams with C streams
	std::ios::sync_with_stdio(true);

	// Clear the error state for each of the C++ standard streams
	std::wcout.clear();
	std::cout.clear();
	std::wcerr.clear();
	std::cerr.clear();
	std::wcin.clear();
	std::cin.clear();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	AttachParentConsole();
	std::vector<std::string> args = CommandLineToArgvA();
#else

// If you can't tell what the main() is for you should have your
// programming license revoked...
int main(int argc, char *argv[])
{
	std::vector<std::string> args = CommandLineToArgvA();

#endif

#ifndef DUMP_CORE
#ifdef _MSC_VER
	// Uncomment to check memory leaks in VS
	SetUnhandledExceptionFilter(crashLogger);
#ifdef __MINGW32__
	// MinGW can use SJLJ or Dwarf exceptions, because of this SEH can't catch it.
	std::set_terminate(exceptionLogger);
#endif
	// Uncomment to debug crash handler
	// AddVectoredContinueHandler(1, crashLogger);
#else
	signal(SIGSEGV, signalLogger);
	std::set_terminate(exceptionLogger);
#endif
#endif

	//CrossPlatform::getErrorDialog();
	//CrossPlatform::processArgs(argc, argv);
// OPTIONSHACK
	// if (!Options::init())
	//	return EXIT_SUCCESS;
	std::ostringstream title;
	title << "OpenXcom " << OPENXCOM_VERSION_SHORT << OPENXCOM_VERSION_GIT;
// OPTIONSHACK
	//Options::baseXResolution = Options::displayWidth;
	//Options::baseYResolution = Options::displayHeight;

	// Load the options
	Options options;
	if(!options.load(args)) // options can be changed by the command line
	{
		return EXIT_FAILURE;
	}

	// some options (like -version or -help) don't need to run the game
	if (options.get<&GameOptions::_shouldRun>() == false)
	{
		return EXIT_SUCCESS;
	}

	// With the options loaded, we can now create the game
	Game* game = new Game(title.str(), &options);

	game->setState(new StartState);
	game->run();

	delete game;

	//KN NOTE: Auto-updating is disabled for now.
	//bool startUpdate = game->getUpdateFlag();

	//// Comment those two for faster exit.
	//delete game;
	//FileMap::clear(true, false); // make valgrind happy

	//if (startUpdate)
	//{
	//	CrossPlatform::startUpdateProcess();
	//}

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return EXIT_SUCCESS;
}

#ifdef __MORPHOS__
const char Version[] = "$VER: OpenXCom " OPENXCOM_VERSION_SHORT " (" __AMIGADATE__  ")";
#endif
