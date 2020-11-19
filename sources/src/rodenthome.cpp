/*
Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
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

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else    
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#include "rodent.h"

// --------------------------------------------------------------------------------
// GetEnv
// --------------------------------------------------------------------------------

std::string GetEnv(const char *envName) {
    std::string retStr;

    // Default value (we don't need difference between 'empty' and 'unset')
    retStr = "";

#if defined(_WIN32) || defined(_WIN64)
    // Simple variant is enough - without autoincrease the limit
    const DWORD buff_size = 255;
    
    wchar_t buff[buff_size];
    const DWORD var_size = GetEnvironmentVariableW(Str2WStr(envName).c_str(), buff, buff_size);

    if (var_size && var_size <= buff_size)
        retStr = WCStr2Str(buff);
#else
    char *envValue;
    envValue = getenv(envName);
    if (envValue)
        retStr = envValue;
#endif
    
    return retStr;
}

// --------------------------------------------------------------------------------
// MkDir
// --------------------------------------------------------------------------------

bool MkDir(const char *DirName) {
    bool result;
    
#if defined(_WIN32) || defined(_WIN64)
    result = CreateDirectoryW(Str2WStr(DirName).c_str(), NULL);
#else
    result = (-1 != mkdir(DirName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH));
#endif    

    if (!result) {
        printf_debug("Error creating directory '%s'\n", DirName);
    } else
        printf_debug("Directory successfully created '%s'\n", DirName);
    
    return result;
}

// --------------------------------------------------------------------------------
// DirOrFileExists
// --------------------------------------------------------------------------------

bool DirOrFileExists(const char* path) {

#if defined(_WIN32) || defined(_WIN64)
    DWORD ftyp;
    
    ftyp = GetFileAttributesA(path);
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false; //something is wrong with your path!
    // if need any "DirExists", use:
    //     return (ftyp & FILE_ATTRIBUTE_DIRECTORY);
	
#else
    struct stat info;

    if (stat( path, &info ))
        return false; //something is wrong with your path!
    // if need any "DirExists", use:
    //     return ( info.st_mode & S_IFDIR );
	
#endif
    
    return true;
}

// --------------------------------------------------------------------------------
// PathEndSlash
// --------------------------------------------------------------------------------
// to ensure that path ends with a slash

void PathEndSlash(std::wstring &pathWStr) {

    std::wstring slashWStr;

    if (pathWStr == L"")
        return; // what???

#ifdef __CYGWIN__
    // cygwin allows win- and linux-style, so add the right slash:
    // But if you don't use cygwin environment, you must use win-style,
    // to make chdir working - but file handling also work with linux-style ;-)
    slashWStr = L"\\";
    if (pathWStr.substr(0,1) == L"/")
        slashWStr = L"/";
#elif defined(_WIN32) || defined(_WIN64)
    slashWStr = L"\\";
#else
    slashWStr = L"/";
#endif

    if (pathWStr.substr(pathWStr.length()-1,1) != slashWStr)
        pathWStr += slashWStr; 
}

#if defined (ANDROID)
void CheckRodentHome(std::wstring checkDirWStr) {

    if (RodentHomeDirWStr != L"")
        return;

    if (DirOrFileExists(WStr2Str(checkDirWStr + L"personalities/basic.ini").c_str()))
        RodentHomeDirWStr = checkDirWStr;
}
#endif

// --------------------------------------------------------------------------------
// SetRodentHomeDir
// --------------------------------------------------------------------------------

void SetRodentHomeDir() {

    RodentHomeDirWStr = L"";
#ifndef DEBUG
    SkipBeginningOfLog = true;
#else
    SkipBeginningOfLog = false;
#endif

    // Taken from "ChDir", added GetEnv

#if defined(_WIN32) || defined(_WIN64)
    RodentHomeDirWStr = Str2WStr(GetEnv("RODENT4HOME"));
    if (RodentHomeDirWStr != L"") {
        PathEndSlash(RodentHomeDirWStr);
        printf_debug("override for home path: '%s'\n", WStr2Str(RodentHomeDirWStr).c_str());
    } else  {
        wchar_t exe_path[1024];

        // getting the current executable location ...
        GetModuleFileNameW(NULL, exe_path, sizeof(exe_path)/sizeof(exe_path[0]));
        *(wcsrchr(exe_path, '\\') + 1) = L'\0';

        RodentHomeDirWStr = WCStr2WStr(exe_path);
    }

#elif defined (__APPLE__)
    // #error something should be done here, look for _NSGetExecutablePath(path, &size)
    // RodentHomeDirWStr = ...
    
#elif defined (ANDROID)
    RodentHomeDirWStr = L"";

    CheckRodentHome(L"/sdcard/Rodent4/");
    CheckRodentHome(L"/sdcard/Android/data/ccc.chess.engines/files/");

    if (RodentHomeDirWStr == L"")
        RodentHomeDirWStr = L"/sdcard/Rodent4/";

    // only if it doesn't exists, create it
    // (filling with default files is no longer needed)
    // Also no longer create directories
    // CreateRodentHome(WStr2Str(RodentHomeDirWStr).c_str());

#else
    RodentHomeDirWStr = Str2WStr(GetEnv("RODENT4HOME"));
    if (RodentHomeDirWStr != L"") {
        PathEndSlash(RodentHomeDirWStr);
        printf_debug("override for home path: '%s'\n", WStr2Str(RodentHomeDirWStr).c_str());
    } else  {
        char exe_path[1024];
        // getting the current executable location ...
        readlink("/proc/self/exe", exe_path, sizeof(exe_path));
        *(strrchr(exe_path, '/') + 1) = '\0';

        RodentHomeDirWStr = CStr2WStr(exe_path);
    }

#endif

#ifndef DEBUG
    LogFileWStr = L"";
#else
    LogFileWStr = RodentHomeDirWStr + L"rodent.log";
#endif
    printf_debug("LogFile='%s'\n", WStr2Str(LogFileWStr).c_str());

    if (!DirOrFileExists(WStr2Str(RodentHomeDirWStr + L"personalities/basic.ini").c_str()))
        printfUciOut("info string no 'basic.ini' - check installation, please\n");
#ifdef DEBUG
    else
        printfUciOut("info string use '%s'\n", WStr2Str(RodentHomeDirWStr).c_str());
#endif

    printf_debug("RodentHome: '%s'\n", WStr2Str(RodentHomeDirWStr).c_str());

#if defined(DEBUG) && defined(ANDROID)
    // only for information (for GUIs supporting oex):
    char binfile[1024];
    ssize_t len = readlink("/proc/self/exe", binfile, sizeof(binfile)-1);
	if (len != -1) {
        binfile[len] = '\0';
        printf_debug("binfile '%s'\n", binfile);
	}
#endif
}

// --------------------------------------------------------------------------------
// CreateRodentHome
// --------------------------------------------------------------------------------

// only needed for Android, but working also on win and Linux (good for testing)

void CreateRodentHome(const char *RodentDir) {
    std::string FileNameStr;
    std::string DirNameStr;
//    FILE *f;
    
    // If directory already exists - we don't like do (overwrite) anything
    if (DirOrFileExists(RodentDir))
        return;

    if (!MkDir(RodentDir))
        return;

    // Test it again, if it had worked
    if (!DirOrFileExists(RodentDir))
        return;

    // All is fine, now

#if defined(_WIN32) || defined(_WIN64)
    DirNameStr = (std::string)RodentDir + WCStr2Str(_BOOKSPATH);
#else
    DirNameStr = (std::string)RodentDir + _BOOKSPATH;
#endif
    if (!MkDir(DirNameStr.c_str()))
        return;

#if defined(_WIN32) || defined(_WIN64)
    DirNameStr = (std::string)RodentDir + WCStr2Str(_PERSONALITIESPATH);
#else
    DirNameStr = (std::string)RodentDir + _PERSONALITIESPATH;
#endif
    if (!MkDir(DirNameStr.c_str()))
        return;

/* template
    //  --------------------------------------------------------------------------------
    //  personalities/dummy.txt
    //  --------------------------------------------------------------------------------
    FileNameStr = DirNameStr + "dummy.txt";
    if (DirOrFileExists(FileNameStr.c_str())) {
    // to be sure not overwriting anything
        printf_debug("File alredy exists '%s'\n", FileNameStr.c_str());
    } else {
        printf_debug("Create file '%s'\n", FileNameStr.c_str());
        f = fopen(FileNameStr.c_str(), "w");
        if (f) {
            fprintf(f, 
#include "DefaultPersonalities/dummy.txt"
            );

            fclose(f);
        }
    }
*/


/* 
--------------------------------------------------------------------------------
activate later with updated files (and maybe better code)!!!
--------------------------------------------------------------------------------

#define CreatePersonalityPRE(WriteFileName) FileNameStr = DirNameStr + WriteFileName; if (DirOrFileExists(FileNameStr.c_str())) { printf_debug("File alredy exists '%s'\n", FileNameStr.c_str()); } else { printf_debug("Create file '%s'\n", FileNameStr.c_str()); f = fopen(FileNameStr.c_str(), "w"); if (f) { fprintf(f, 
#define CreatePersonalityPOST ); fclose(f); }}

CreatePersonalityPRE ("basic.ini")
#include "DefaultPersonalities/basic.ini"
CreatePersonalityPOST

CreatePersonalityPRE ("_init.txt")
#include "DefaultPersonalities/_init.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("alekhine.txt")
#include "DefaultPersonalities/alekhine.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("amanda.txt")
#include "DefaultPersonalities/amanda.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("ampere.txt")
#include "DefaultPersonalities/ampere.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("anand.txt")
#include "DefaultPersonalities/anand.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("anderssen.txt")
#include "DefaultPersonalities/anderssen.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("botvinnik.txt")
#include "DefaultPersonalities/botvinnik.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("cloe.txt")
#include "DefaultPersonalities/cloe.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("deborah.txt")
#include "DefaultPersonalities/deborah.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("defender.txt")
#include "DefaultPersonalities/defender.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("fischer.txt")
#include "DefaultPersonalities/fischer.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("grumpy.txt")
#include "DefaultPersonalities/grumpy.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("karpov.txt")
#include "DefaultPersonalities/karpov.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("kasparov.txt")
#include "DefaultPersonalities/kasparov.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("kortchnoi.txt")
#include "DefaultPersonalities/kortchnoi.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("larsen.txt")
#include "DefaultPersonalities/larsen.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("lasker.txt")
#include "DefaultPersonalities/lasker.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("marshall.txt")
#include "DefaultPersonalities/marshall.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("morphy.txt")
#include "DefaultPersonalities/morphy.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("nimzowitsch.txt")
#include "DefaultPersonalities/nimzowitsch.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("partisan.txt")
#include "DefaultPersonalities/partisan.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("pawnsacker.txt")
#include "DefaultPersonalities/pawnsacker.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("pedrita.txt")
#include "DefaultPersonalities/pedrita.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("petrosian.txt")
#include "DefaultPersonalities/petrosian.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("preston.txt")
#include "DefaultPersonalities/preston.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("reti.txt")
#include "DefaultPersonalities/reti.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("rubinstein.txt")
#include "DefaultPersonalities/rubinstein.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("simple.txt")
#include "DefaultPersonalities/simple.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("spassky.txt")
#include "DefaultPersonalities/spassky.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("spitfire.txt")
#include "DefaultPersonalities/spitfire.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("steinitz.txt")
#include "DefaultPersonalities/steinitz.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("strangler.txt")
#include "DefaultPersonalities/strangler.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("tal.txt")
#include "DefaultPersonalities/tal.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("tarrasch.txt")
#include "DefaultPersonalities/tarrasch.txt"
CreatePersonalityPOST

CreatePersonalityPRE ("test.txt")
#include "DefaultPersonalities/test.txt"
CreatePersonalityPOST

*/
}
