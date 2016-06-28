#pragma once

#include <windows.h>
#include <tchar.h>

#include <stdarg.h>
#include <cstdio>

#define Override virtual

#define LogMaxSize 500
inline bool DebugLogLn(const char* format, ...)
{
#ifdef _DEBUG
	va_list args;
	va_start(args, format);

	static char logBuf[LogMaxSize +1];
	_vsnprintf_s(logBuf, LogMaxSize, LogMaxSize -1, format, args);

	va_end(args);

	static char logBuf2[LogMaxSize +3];
	_snprintf_s(logBuf2, sizeof(logBuf2), sizeof(logBuf2) -1, "%s\r\n", logBuf);

	OutputDebugStringA(logBuf2);
#endif
	return true;
}

#define  SafeDelete(p)	{if(p){delete p; p =NULL;}}


#include <string>
#include <vector>
using namespace std;

inline int split(const string& str, vector<string>& ret_, const string& sep)
{
	if (str.empty())
	{
		return 0;
	}

	string tmp;
	string::size_type pos_begin = str.find_first_not_of(sep);
	string::size_type comma_pos = 0;

	while (pos_begin != string::npos)
	{
		comma_pos = str.find(sep, pos_begin);
		if (comma_pos != string::npos)
		{
			tmp = str.substr(pos_begin, comma_pos - pos_begin);
			pos_begin = comma_pos + sep.length();
		}
		else
		{
			tmp = str.substr(pos_begin);
			pos_begin = comma_pos;
		}

		if (!tmp.empty())
		{
			ret_.push_back(tmp);
			tmp.clear();
		}
	}
	return 0;
}