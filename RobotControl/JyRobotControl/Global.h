#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdarg.h>
#include <cstdio>
#include <string>
#include <vector>

#ifndef LogMaxSize
    #define LogMaxSize 500
#endif

inline bool DebugLogLn_(const char* format, ...)
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

#ifndef SafeDelete

#define  SafeDelete(p)	{if(p){delete p; p =nullptr;}}
#define  SafeDeleteAtomic(t, p)	{if((t)p){delete (t)p; p =nullptr;}}

#endif // !1

typedef std::vector<BYTE> vecByte;

inline int split_(const std::string& str, std::vector<std::string>& ret_, const std::string& sep)
{
	if (str.empty())
	{
		return 0;
	}

    std::string tmp;
    std::string::size_type pos_begin = str.find_first_not_of(sep);
    std::string::size_type comma_pos = 0;

    while (pos_begin != std::string::npos)
	{
		comma_pos = str.find(sep, pos_begin);
        if (comma_pos != std::string::npos)
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