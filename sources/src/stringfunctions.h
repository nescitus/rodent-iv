
#include <string>

// In my opinion:
// - overload do things automatically and make it easier
//   for example both would work: Str(L"C:\\temp\\") and Str("/tmp")
// - but maybe can make it harder finding errors

// #define STR_OVERLOAD


std::string CStr2Str(const char* CStr);
std::string WCStr2Str(const wchar_t* WCStr);
std::string WStr2Str(const std::wstring &WStr);

std::wstring CStr2WStr(const char* CStr);
std::wstring WCStr2WStr(const wchar_t* WCStr);
std::wstring Str2WStr(const std::string &Str);

#ifdef STR_OVERLOAD
std::string Str(const std::string &Str);
std::string Str(const char* CStr);
std::string Str(const wchar_t* WCStr);
std::string Str(const std::wstring &WStr);
std::wstring WStr(const std::wstring &WStr);
std::wstring WStr(const char* CStr);
std::wstring WStr(const wchar_t* WCStr);
std::wstring WStr(const std::string &Str);
#endif

// Use this also?
// #define Str2CStr(x) ( (x).c_str() )
// #define WStr2CStr(x) ( WStr2Str(x).c_str())

// void TestStrFunctions();

std::string str_tolower(const std::string &inStr);
