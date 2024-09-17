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
#include "Logger.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace OpenXcom
{

/**
 * Generates a timestamp of the current time.
 * @return String in Y-M-D_H-M-S format.
 */
std::string now()
{
	const int MAX_LEN = 25, MAX_RESULT = 80;
	char result[MAX_RESULT] = {0};
#ifdef _WIN32
	char date[MAX_LEN], time[MAX_LEN];
	if (GetDateFormatA(LOCALE_INVARIANT, 0, 0, "yyyy'-'MM'-'dd", date, MAX_LEN) == 0)
		return "0000-00-00";
	if (GetTimeFormatA(LOCALE_INVARIANT, TIME_FORCE24HOURFORMAT, 0, "HH'-'mm'-'ss", time, MAX_LEN) == 0)
		return "00-00-00";
	sprintf_s(result, MAX_RESULT-1, "%s_%s", date, time);
#else
	char buffer[MAX_LEN];
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, MAX_LEN, "%Y-%m-%d_%H-%M-%S", timeinfo);
	sprintf(result, "%s", buffer);
#endif
	return result;
}

void log(int level, const std::ostringstream& baremsgstream)
{
	std::ostringstream msgstream;
	msgstream << "[" << now() << "]"
			  << "\t"
			  << "[" << Logger::toString(level) << "]"
			  << "\t"
			  << baremsgstream.str() << std::endl;
	std::string msg = msgstream.str();

	//int effectiveLevel = Logger::reportingLevel();
	//if (effectiveLevel >= LOG_DEBUG)
	//{
	//	fwrite(msg.c_str(), msg.size(), 1, stderr);
	//	fflush(stderr);
	//}
	//if (logBuffer.size() > LOG_BUFFER_LIMIT)
	//{ // drop earliest message so as to not eat all memory
	//	logBuffer.pop_front();
	//}
	//if (logFileName.empty() || effectiveLevel == LOG_UNCENSORED)
	//{ // no log file; accumulate.
	//	logBuffer.push_back(std::make_pair(level, msg));
	//	return;
	//}
	//// attempt to flush the buffer
	//bool failed = false;
	//while (!logBuffer.empty())
	//{
	//	if (effectiveLevel >= logBuffer.front().first)
	//	{
	//		if (!logToFile(logFileName, logBuffer.front().second))
	//		{
	//			std::string err = "Failed to append to '" + logFileName + "': " + SDL_GetError();
	//			logBuffer.push_back(std::make_pair(LOG_ERROR, err));
	//			failed = true;
	//			break;
	//		}
	//	}
	//	logBuffer.pop_front();
	//}
	//// retain the current message if write fails.
	//if (failed || !logToFile(logFileName, msg))
	//{
	//	logBuffer.push_back(std::make_pair(level, msg));
	//}

#ifdef _WIN32
	// output to visual studio
	OutputDebugStringA(msg.c_str());
#endif
}

} // namespace OpenXcom
