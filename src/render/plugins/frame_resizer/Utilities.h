#pragma once
#include <winnt.h>
#include <locale>
#include <comdef.h>
#include "tc_common_new/log.h"
#include "tc_common_new/string_util.h"

//wprintf(L"RETURN_ON_BAD_HR: hr=0x%08x, error is %ls\n", _hr_, err.ErrorMessage());

#define RETURN_ON_BAD_HR(expr) \
{ \
    HRESULT _hr_ = (expr); \
    if (FAILED(_hr_)) { \
    {\
        _com_error err(_hr_);\
        LOGE("HRESULT: {:x}, msg: {}", (uint32_t)_hr_, tc::StringUtil::ToUTF8(err.ErrorMessage())); \
    }\
        return _hr_; \
    } \
}

#define LOG_ON_BAD_HR(expr) \
{ \
    HRESULT _hr_ = (expr); \
    if (FAILED(_hr_)) { \
    {\
        _com_error err(_hr_);\
        wprintf(L"BAD HR: hr=0x%08x, error is %ls\n", _hr_, err.ErrorMessage());\
    }\
    } \
}

inline std::wstring s2ws(const std::string& str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
inline std::string ws2s(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string r(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &r[0], size_needed, NULL, NULL);
	return r;
}


// Create a string with last error message
inline std::string GetLastErrorStdStr()
{
	DWORD error = GetLastError();
	if (error)
	{
		LPVOID lpMsgBuf;
		DWORD bufLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, nullptr);
		if (bufLen)
		{
			LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
			std::string result(lpMsgStr, lpMsgStr + bufLen);

			LocalFree(lpMsgBuf);

			return result;
		}
	}
	return std::string();
}

// Create a string with last error message
inline std::wstring GetLastErrorStdWstr() {
	return s2ws(GetLastErrorStdStr());
}

inline INT64 MillisToHundredNanos(INT64 millis) {
	return millis * 10 * 1000;
}

//inline INT64 HundredNanosToMillis(INT64 hundredNanos) {
//	return round((double)hundredNanos / 10 / 1000);
//}

