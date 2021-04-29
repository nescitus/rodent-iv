/*
Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
Copyright (C) 2011-2019 Pawel Koziol
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

#include "rodent.h"
#include "book.h"
#include <cstdlib>
#include <string>

cGlobals Glob;

#ifdef USE_THREADS
    #include <list>
    std::list<cEngine> Engines(1);
#else
    cEngine EngineSingle(0);
#endif
cBitBoard BB;
cParam Par;
cMask Mask;
cDistance Dist;
sBook GuideBook;
sBook MainBook;

void PrintVersion() {
    std::string OutStr;

    OutStr = "id name Rodent IV 0.33";

#if defined(DEBUG)

	int bits = sizeof(void*) * 8; // CHAR_BIT
	if (bits == 32)
        OutStr += " 32-bit";
	else if (bits == 64)
        OutStr += " 64-bit";

#if defined(__arm__)
    OutStr += "/arm";
#elif defined(__aarch64__)
    OutStr += "/aarch64";
#elif defined(__i386__)
    OutStr += "/x86";
#elif defined(__x86_64__)
    OutStr += "/x86_64";
#endif

#if defined(__clang__)
    // "__clang_version__" too long
    OutStr += "/CLANG " + std::to_string(__clang_major__);
    OutStr += "." + std::to_string(__clang_minor__);
    OutStr += "." + std::to_string(__clang_patchlevel__);
#elif defined(__MINGW32__)
    OutStr += "/MINGW " __VERSION__;
#elif defined(__GNUC__)
    OutStr += "/GCC " __VERSION__;
#elif defined(_MSC_VER)
    OutStr += "/MSVS";
    #if   _MSC_VER == 1900
        OutStr += "2015";
    #elif _MSC_VER >= 1910
        OutStr += "2017";
    #endif
#endif

#if (defined(_MSC_VER) && defined(USE_MM_POPCNT)) || (defined(__GNUC__) && defined(__POPCNT__))
        OutStr += "/POPCNT";
#elif defined(__GNUC__) && defined(__SSSE3__) // we are using custom SSSE3 popcount implementation
        OutStr += "/SSSE3";
#endif

#if defined(NO_THREADS)
    OutStr += "/NOSMP";
#else
    OutStr += "/SMP";
#endif

    OutStr += "/DEBUG";
#endif

    // Maybe too much info - can be shortened later
    // But currently it's not bad to have infos
    printfUciOut("%s\n",OutStr.c_str());
}

int main() {

	SetRodentHomeDir();
	Glob.threadOverride = 0;
	ReadThreadNumber("threads.ini");

    // catching memory leaks using MS Visual Studio
#if defined(_MSC_VER) && !defined(NDEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    srand(GetMS());
    BB.Init();
    cEngine::InitSearch();
    POS::Init();
    Glob.Init();
	Par.elo = 2800;
	Par.SetSpeed(Par.elo); // no longer part of DefaultWeights
    // Par.DefaultWeights(); will be done later
    Par.InitKingAttackTable();
	Par.use_ponder = false;
    Mask.Init();
    Dist.Init();

    Par.chess960 = false;
	Par.useBook = true;
	Par.verboseBook = false;

    //PrintVersion();

if (Glob.isNoisy) {
#if defined(_WIN32) || defined(_WIN64)
    printfUciOut("info string opening books path is '%ls' (%s)\n", _BOOKSPATH, ChDir(_BOOKSPATH) ? "exists" : "doesn't exist");
    printfUciOut("info string personalities path is '%ls' (%s)\n", _PERSONALITIESPATH, ChDir(_PERSONALITIESPATH) ? "exists" : "doesn't exist");
#else
    printfUciOut("info string opening books path is '%s' (%s)\n", _BOOKSPATH, ChDir(_BOOKSPATH) ? "exists" : "doesn't exist");
    printfUciOut("info string personalities path is '%s' (%s)\n", _PERSONALITIESPATH, ChDir(_PERSONALITIESPATH) ? "exists" : "doesn't exist");
#endif
}

    PrintOverrides(); // print books and pers paths overrides (26/08/17: linux only)

    GuideBook.SetBookName("guide.bin");
    MainBook.SetBookName("rodent.bin");
    ReadPersonality("basic.ini");

    // To make also "setoption name Personality ..." useable in "default.txt"
    // We need set to default values AFTER reading basic.ini
    Par.DefaultWeights();

    CheckGUI();

    UciLoop();
}

void cGlobals::Init() {

	isNoisy = false;
    isTesting = false;
    isBenching = false;
    isTuning = false;
    useTaunting = false;
    printPv = true;
    isReadingPersonality = false;
    usePersonalityFiles = true;
    useBooksFromPers = true;
    showPersonalityFile = false;
    numberOfThreads = 1;
	if (Glob.threadOverride)
		numberOfThreads = Glob.threadOverride;
	timeBuffer = 10; // blitz under Arena would require something like 200, but it's user's job
	game_key = 0;

    // Clearing  and  setting threads  may  be  necessary
    // if we need a compile using a bigger default number
    // of threads for testing purposes

#ifdef USE_THREADS
    if (numberOfThreads > 1) { //-V547 get rid of PVS Studio warning
        Engines.clear();
        for (int i = 0; i < numberOfThreads; i++)
            Engines.emplace_back(i);
    }
#endif

    shouldClear = false;
    isConsole = true;
    eloSlider = true;
	multiPv = 1;
    CastleNotation = KingMove;
    useUciPersonalitySet = false;
    personalityW = "";
    personalityB = "";
}

bool cGlobals::CanReadBook() {
    return (useBooksFromPers == isReadingPersonality || !usePersonalityFiles);
}