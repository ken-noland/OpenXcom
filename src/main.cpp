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
#include <simplerttr.h>

#if defined(_DEBUG) && defined(_MSC_VER)
// The following is used to capture memory allocations to find out where
// memory leaks are coming from. Since there are statics that allocate
// memory, and since this is the first file passed to the linker, this
// is a good way to find out where the memory is being allocated and not
// deallocated.
#include  <crtdbg.h>

class DbgBreakAlloc
{
public:
	DbgBreakAlloc()
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF /*| _CRTDBG_CHECK_EVERY_16_DF*/);
		_crtBreakAlloc = 470921;
	}
};

DbgBreakAlloc brk;
#endif

#include "Engine/Engine.h"
#include "Game/Game.h"

using namespace OpenXcom;

namespace OpenXcom
{

int run(const std::vector<std::string>& args)
{
	// Create the engine
	std::unique_ptr<Engine> engine = std::make_unique<Engine>(args);

	// Create the game
	std::unique_ptr<Game> game = std::make_unique<Game>(*engine);

	// Run the game
	int ret = game->run();

	game.reset();
	engine.reset();

	return ret;
}

} // namespace OpenXcom


#ifdef _MSC_VER
#include <Windows.h>


// some string helpers
//   trim from start (in place)
inline void ltrim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
				return !std::isspace(ch) && ch != 0;
			}));
}

// trim from end (in place)
inline void rtrim(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
				return !std::isspace(ch) && ch != 0;
			}).base(),
			s.end());
}

//  trim from both ends (in place)
inline void trim(std::string& s)
{
	rtrim(s);
	ltrim(s);
}

// trim from start (copying)
inline std::string ltrim_copy(std::string s)
{
	ltrim(s);
	return s;
}

// trim from end (copying)
inline std::string rtrim_copy(std::string s)
{
	rtrim(s);
	return s;
}

// trim from both ends (copying)
inline std::string trim_copy(std::string s)
{
	trim(s);
	return s;
}

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
	for (int i = 1; i < argc; i++) {
		args.push_back(trim_copy(WideStringToString(argv[i])));
	}

	LocalFree(argv);

	return args;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#if defined(_DEBUG)
    // Initialize CRT memory leak checking
	_CrtMemState initialState, finalState, diffState;
	_CrtMemCheckpoint(&initialState); // Take a snapshot of memory state at start of main
#endif

	int ret = 0;

	// using a scope operator here to ensure that the args are cleaned up before the memory check
	{
		std::vector<std::string> args = CommandLineToArgvA();
#else

std::vector<std::string> CommandLineToArgvA(int argc, char* argv[])
{
	// Create a vector and populate it with the arguments
	std::vector<std::string> args;
	for (int i = 1; i < argc; i++)
	{
		args.push_back(std::string(argv[i]));
	}

	return args;
}

// If you can't tell what the main() is for you should have your
// programming license revoked...
int main(int argc, char *argv[])
{
	int ret = 0;

	// using a scope operator here to ensure that the args are cleaned up before the memory check
	{
		std::vector<std::string> args = CommandLineToArgvA(argc, argv);

#endif
		ret = OpenXcom::run(args);

		SimpleRTTR::shutdown();
	}

#if defined(_DEBUG) && defined(_MSC_VER)
	// Take a snapshot of memory state at the end of main
	_CrtMemCheckpoint(&finalState);

	// Compare the memory state at the beginning and end of main
	if (_CrtMemDifference(&diffState, &initialState, &finalState))
	{
		_CrtMemDumpStatistics(&diffState); // Dump only the leaks that occurred after main
	}
#endif
	return ret;
}

#ifdef __MORPHOS__
const char Version[] = "$VER: OpenXCom " OPENXCOM_VERSION_SHORT " (" __AMIGADATE__  ")";
#endif
