/*
Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2020-2020 Pawel Koziol
Copyright (C) 2020-2020 Bernhard C. Maerz

Rodent is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Rodent is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/

#include "stringfunctions.h"
#include <cctype>

std::string CStr2Str(const char* CStr) {
    return (std::string)CStr;
}
std::string WCStr2Str(const wchar_t* WCStr) {
    std::wstring WStr = (std::wstring)WCStr;
    return std::string(WStr.begin(), WStr.end());
}
std::string WStr2Str(const std::wstring &WStr) {
    return std::string(WStr.begin(), WStr.end());
}

std::wstring CStr2WStr(const char* CStr) {
    std::string Str = (std::string)CStr;
    return std::wstring(Str.begin(), Str.end());
}
std::wstring WCStr2WStr(const wchar_t* WCStr) {
    return (std::wstring)WCStr;
}
std::wstring Str2WStr(const std::string &Str) {
    return std::wstring(Str.begin(), Str.end());
}

#ifdef STR_OVERLOAD

std::string Str(const std::string &Str) {
    return Str;
}
std::string Str(const char* CStr) {
    return CStr2Str(CStr);
}
std::string Str(const wchar_t* WCStr) {
    return WCStr2Str(WCStr);
}
std::string Str(const std::wstring &WStr) {
    return WStr2Str(WStr);
}

std::wstring WStr(const std::wstring &WStr) {
    return WStr;
}
std::wstring WStr(const char* CStr) {
    return CStr2WStr(CStr);
}
std::wstring WStr(const wchar_t* WCStr) {
    return WCStr2WStr(WCStr);
}
std::wstring WStr(const std::string &Str) {
    return Str2WStr(Str);
}

#endif

std::string str_tolower(const std::string &inStr) {
    std::string textStr;

    textStr = inStr;

    for (int i = textStr.length()-1 ; i >= 0 ; i--)
        textStr[i] = std::tolower(textStr[i]);

   return textStr;
}

/*
void TestStrFunctions() {
    std::string testStr;
    std::wstring testWStr;

    printf("\nString conversion samples in TestStrFunctions():\n");

    printf("\n");
    
    testStr = "direct";
    printf("testStr=\"%s\"\n", testStr.c_str());
    testWStr = L"direct";
    printf("testWStr=L\"%ls\"\n", testWStr.c_str());

    printf("\n");
    
    testStr = CStr2Str("using CStr2Str");
    printf("testStr=\"%s\"\n", testStr.c_str());
    testStr = WCStr2Str(L"using WCStr2Str");
    printf("testStr=\"%s\"\n", testStr.c_str());
    testWStr = L"using WStr2Str"; testStr = WStr2Str(testWStr);
    printf("testStr=\"%s\"\n", testStr.c_str());

    testWStr = WCStr2WStr(L"using WCStr2WStr");
    printf("testWStr=L\"%ls\"\n", testWStr.c_str());
    testWStr = CStr2WStr("using CStr2WStr");
    printf("testWStr=L\"%ls\"\n", testWStr.c_str());
    testStr = "using Str2WStr"; testWStr = Str2WStr(testStr);
    printf("testWStr=L\"%ls\"\n", testWStr.c_str());
    
#ifdef STR_OVERLOAD

    printf("\n");

    testStr = Str("using Str=>CStr2Str");
    printf("testStr=\"%s\"\n", testStr.c_str());
    testStr = Str(L"using Str=>WCStr2Str");
    printf("testStr=\"%s\"\n", testStr.c_str());
    testWStr = L"using Str=>WStr2Str"; testStr = Str(testWStr);
    printf("testStr=\"%s\"\n", testStr.c_str());

    testWStr = WStr(L"using WStr=>WCStr2WStr");
    printf("testWStr=L\"%ls\"\n", testWStr.c_str());
    testWStr = WStr("using WStr=>CStr2WStr");
    printf("testWStr=L\"%ls\"\n", testWStr.c_str());
    testStr = "using WStr=>Str2WStr"; testWStr = WStr(testStr);
    printf("testWStr=L\"%ls\"\n", testWStr.c_str());

    printf("\n");

    // Nice thing, avoiding "#if defined(_WIN32) || defined(_WIN64)"
    #define WinPath L"c:\\temp"
    #define linuxPath "/tmp"

    printf("WinPath with Str=\"%s\"\n",   Str(WinPath).c_str());
    printf("LinuxPath with Str=\"%s\"\n", Str(linuxPath).c_str());
    printf("WinPath with WStr=\"%ls\"\n",   WStr(WinPath).c_str());
    printf("LinuxPath  with WStr=\"%ls\"\n", WStr(linuxPath).c_str());

#endif

    printf("\n");
}
*/
