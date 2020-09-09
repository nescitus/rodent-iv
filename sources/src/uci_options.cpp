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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <iomanip>
#include <sstream>
#include "stringfunctions.h"

#define PERSALIAS_ALEN       32     // max length for a personality alias
#define PERSALIAS_PLEN       256    // max length for an alias path
#define PERSALIAS_MAXALIASES 100    // max number of aliases
struct {
    char alias[PERSALIAS_MAXALIASES][PERSALIAS_ALEN];
    char path[PERSALIAS_MAXALIASES][PERSALIAS_PLEN];
    int count;
} pers_aliases;

#define PERSSETALIAS_LEN     32     // max length for a personality-set alias
struct {
    char alias[10][PERSSETALIAS_LEN]; // use one-digit-number for sets, so max-value fixed
    int count = 0;
    int used = 0;
} pers_sets;

void PrintSingleOption(int ind) {
    printfUciOut("option name %s type spin default %d min %d max %d\n",
            paramNames[ind], Par.values[ind], Par.min_val[ind], Par.max_val[ind]);
}

// workaround for giving infos ("type label") to the GUI
void PrintUciOptionInfo(const char* name, const char* info) {
    printfUciOut("option name %s type combo default %s var %s\n", name, info, info);
}

void PrintUciOptions() {

#ifdef DEBUG
    using namespace std;

    const string months("Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec");
    const string time(__TIME__); // From compiler, format is " 1:02:03"
    string month, day, year;
    stringstream ss, date(__DATE__); // From compiler, format is "Sep 21 2008"

    date >> month >> day >> year;
    ss << setfill('0') << year << setw(2) << (1 + months.find(month) / 4) << setw(2) << day;
    ss << setw(2) << atoi(time.substr(0,2).c_str()) << setw(2) << time.substr(3,2) << setw(2) << time.substr(6,2);

    // be sure testing the right version
    PrintUciOptionInfo("Build Time", ss.str().c_str());
#endif

	printfUciOut("option name Clear Hash type button\n");
    printfUciOut("option name Hash type spin default 16 min 1 max %d\n", max_tt_size_mb);

    if (LogFileWStr != L"")
        printfUciOut("option name LogFile type string default %s\n", WStr2Str(LogFileWStr).c_str());
#if defined(DEBUG) || defined(ANDROID)
    else
        printfUciOut("option name LogFile type string default <empty>\n");
#endif

#ifdef USE_THREADS
	if (Glob.threadOverride == 0)
    printfUciOut("option name Threads type spin default %d min 1 max %d\n", Glob.numberOfThreads, MAX_THREADS);
#endif
    printfUciOut("option name MultiPV type spin default %d min 1 max %d\n", Glob.multiPv, MAX_PV);
	printfUciOut("option name TimeBuffer type spin default %d min 0 max 1000\n", Glob.timeBuffer);

    if (Glob.eloSlider) {
        printfUciOut("option name PrintPv type check default %s\n", Glob.printPv ? "true" : "false");
        printfUciOut("option name Taunting type check default %s\n", Glob.useTaunting ? "true" : "false");
        printfUciOut("option name UCI_LimitStrength type check default %s\n", Par.useWeakening ? "true" : "false");
        printfUciOut("option name UCI_Elo type spin default %d min 800 max 2800\n", Par.elo);
    }

    if (Glob.usedGUI == Arena)
        PrintUciOptionInfo("Chess960 mode", "autodetection");
    else
        printfUciOut("option name UCI_Chess960 type check default false\n");

    // additional "var take" not really needed - so not shown as option
    // Arena needs "o-o", all other "move"
#ifdef DEBUG
    if (Glob.CastleNotation == OOO)
        printfUciOut("option name CastleNotation type combo default O-O var move var O-O\n");
    else if (Glob.CastleNotation == TakeRook)
        printfUciOut("option name CastleNotation type combo default take var move var O-O\n");
    else
        printfUciOut("option name CastleNotation type combo default move var move var O-O\n");
#endif

    if (Glob.usePersonalityFiles) {
        if (pers_aliases.count == 0 || Glob.showPersonalityFile)
            printfUciOut("option name PersonalityFile type string default default.txt\n");
        if (Glob.useUciPersonalitySet) {
            if (pers_sets.count != 0 && pers_sets.used < pers_sets.count) {
                printfUciOut("option name PersonalitySet type combo default %s", pers_sets.alias[pers_sets.used]);
                for (int i = 0; i < pers_sets.count; i++)
                    printfUciAdd(" var %s", pers_sets.alias[i]);
                printfUciAdd("\n");
            }
        }
        if (pers_aliases.count != 0) {
            std::string varStr = "";
            for (int i = 0; i < pers_aliases.count; i++)
                varStr += " var " + (std::string)pers_aliases.alias[i];
            // `---` in case we want PersonalityFile
            printfUciOut("option name Personality type combo default ---%s\n", varStr.c_str());
            #ifdef ANDROID
                // Feature not needed in windows version
                printfUciOut("option name PersonalityB type combo default ---%s\n", varStr.c_str());
            #endif
        }
    } else {

        printfUciOut("option name PawnValue type spin default %d min 0 max 1200\n", V(P_MID));
        printfUciOut("option name KnightValue type spin default %d min 0 max 1200\n", V(N_MID));
        printfUciOut("option name BishopValue type spin default %d min 0 max 1200\n", V(B_MID));
        printfUciOut("option name RookValue type spin default %d min 0 max 1200\n", V(R_MID));
        printfUciOut("option name QueenValue type spin default %d min 0 max 1200\n", V(Q_MID));

        printfUciOut("option name KeepPawn type spin default %d min 0 max 500\n", Par.keep[P]);
        printfUciOut("option name KeepKnight type spin default %d min 0 max 500\n", Par.keep[N]);
        printfUciOut("option name KeepBishop type spin default %d min 0 max 500\n", Par.keep[B]);
        printfUciOut("option name KeepRook type spin default %d min 0 max 500\n", Par.keep[R]);
        printfUciOut("option name KeepQueen type spin default %d min 0 max 500\n", Par.keep[Q]);

        PrintSingleOption(B_PAIR_MG);
        PrintSingleOption(B_PAIR_EG);
        PrintSingleOption(N_PAIR);
        PrintSingleOption(R_PAIR);
        printfUciOut("option name ExchangeImbalance type spin default %d min -200 max 200\n", V(A_EXC));
        printfUciOut("option name KnightLikesClosed type spin default %d min 0 max 10\n", V(N_CL));

        PrintSingleOption(W_MATERIAL);
        printfUciOut("option name PrimaryPstStyle type spin default %d min 0 max 4\n", Par.primaryPstStyle);
        printfUciOut("option name SecondaryPstStyle type spin default %d min 0 max 4\n", Par.secondaryPstStyle);
        // printfUciOut("option name PiecePlacement type spin default %d min 0 max 500\n", V(W_PRIM));
        PrintSingleOption(W_OWN_ATT);
        PrintSingleOption(W_OPP_ATT);
        PrintSingleOption(W_OWN_MOB);
        PrintSingleOption(W_OPP_MOB);
        PrintSingleOption(W_TROPISM);
        PrintSingleOption(W_PRIM);
        PrintSingleOption(W_SECO);
        PrintSingleOption(W_THREATS);

        PrintSingleOption(W_PASSERS);
        PrintSingleOption(W_STRUCT);
        PrintSingleOption(W_SHIELD);
        PrintSingleOption(W_STORM);
        PrintSingleOption(W_OUTPOSTS);
        PrintSingleOption(W_LINES);

        printfUciOut("option name FianchKing type spin default %d min 0 max 100\n", V(B_KING));

        printfUciOut("option name Contempt type spin default %d min -500 max 500\n", Par.drawScore);

        if (!Glob.eloSlider) {
            printfUciOut("option name EvalBlur type spin default %d min 0 max 5000000\n", Par.evalBlur);
            printfUciOut("option name NpsLimit type spin default %d min 0 max 5000000\n", Par.npsLimit);
        }

        printfUciOut("option name SlowMover type spin default %d min 10 max 500\n", Par.timePercentage);
        printfUciOut("option name Selectivity type spin default %d min 10 max 500\n", Par.hist_perc);
        printfUciOut("option name SearchSkill type spin default %d min 0 max 10\n", Par.searchSkill);
    }
	printfUciOut("option name Verbose type check default %s\n", Glob.isNoisy ? "true" : "false");
    printfUciOut("option name Ponder type check default %s\n", Par.use_ponder ? "true" : "false");
    printfUciOut("option name UseBook type check default %s\n", Par.useBook ? "true" : "false");
    printfUciOut("option name VerboseBook type check default %s\n", Par.verboseBook ? "true" : "false");
    printfUciOut("option name MobilityRebalancing type check default %s\n", Par.useMobilityRebalancing ? "true" : "false");

    if (!Glob.useBooksFromPers || !Glob.usePersonalityFiles) {
        printfUciOut("option name BookFilter type spin default %d min 0 max 100\n", Par.bookFilter);
        printfUciOut("option name GuideBookFile type string default %s\n", GuideBook.bookName);
        printfUciOut("option name MainBookFile type string default %s\n", MainBook.bookName);
    }
}

static void valuebool(bool& param, char *val) {

    for (int i = 0; val[i]; i++)
        val[i] = tolower(val[i]);

    if (strcmp(val, "true")  == 0) param = true;
    if (strcmp(val, "false") == 0) param = false;
}

// @brief set a value that is a part of Par.values[]

static void setvalue(int ind, int val, bool isTable) {

    Par.values[ind] = val;
    if (isTable) Par.InitPst();
    Glob.shouldClear = true;
}

static char *pseudotrimstring(char *in_str) {

    for (int last = (int)strlen(in_str)-1; last >= 0 && in_str[last] == ' '; last--)
        in_str[last] = '\0';

    while (*in_str == ' ') in_str++;

    return in_str;
}

void ParseSetoption(const char *ptr) {

    char *name, *value;

    char *npos = (char *)strstr(ptr, " name ");  // len(" name ") == 6, len(" value ") == 7
    char *vpos = (char *)strstr(ptr, " value "); // sorry for an ugly "unconst"

    if ( !npos ) return; // if no 'name'

    if ( vpos ) {
        *vpos = '\0';
        value = pseudotrimstring(vpos + 7);
    }
    else value = npos; // fake, just to prevent possible crash if misusing

    name = pseudotrimstring(npos + 6);

    for (int i = 0; name[i]; i++)   // make `name` lowercase
        name[i] = tolower(name[i]); // we can't lowercase `value` 'coz paths are case-sensitive on linux

    printf_debug("setoption name: '%s' value: '%s'\n", name, value );

    if (strcmp(name, "hash") == 0)                                           {
        Trans.AllocTrans(atoi(value));
#ifdef USE_THREADS
    } else if (strcmp(name, "threads") == 0 && Glob.threadOverride == 0)     {
        Glob.numberOfThreads = (atoi(value));
        if (Glob.numberOfThreads > MAX_THREADS) Glob.numberOfThreads = MAX_THREADS;

        if (Glob.numberOfThreads != (int)Engines.size()) {
            Engines.clear();
            for (int i = 0; i < Glob.numberOfThreads; i++)
                Engines.emplace_back(i);
        }
#endif
    } else if (strcmp(name, "clear hash") == 0)                              {
        Trans.Clear();
    } else if (strcmp(name, "multipv") == 0)                                 {
        Glob.multiPv = atoi(value);
        if (Glob.multiPv < 1)
            Glob.multiPv = 1;
        else if (Glob.multiPv > MAX_PV)
            Glob.multiPv = MAX_PV;
    } else if (strcmp(name, "timebuffer") == 0)                              {
        Glob.timeBuffer = atoi(value);
    } else if (strcmp(name, "pawnvaluemg") == 0)                             {
        setvalue(P_MID, atoi(value), true);
    } else if (strcmp(name, "pawnvalueeg") == 0)                             {
        setvalue(P_END, atoi(value), true);
    } else if (strcmp(name, "knightvaluemg") == 0)                           {
        setvalue(N_MID, atoi(value), true);
    } else if (strcmp(name, "knightvalueeg") == 0)                           {
        setvalue(N_END, atoi(value), true);
    } else if (strcmp(name, "bishopvaluemg") == 0)                           {
        setvalue(B_MID, atoi(value), true);
    } else if (strcmp(name, "bishopvalueeg") == 0)                           {
        setvalue(B_END, atoi(value), true);
    } else if (strcmp(name, "rookvaluemg") == 0)                             {
        setvalue(R_MID, atoi(value), true);
    } else if (strcmp(name, "rookvalueeg") == 0)                             {
        setvalue(R_END, atoi(value), true);
    } else if (strcmp(name, "queenvaluemg") == 0)                            {
        setvalue(Q_MID, atoi(value), true);
    } else if (strcmp(name, "queenvalueeg") == 0)                            {
        setvalue(Q_END, atoi(value), true);
    } else if (strcmp(name, "keeppawn") == 0)                                {
        Par.keep[P] = atoi(value);
        Glob.shouldClear = true;
    } else if (strcmp(name, "keepknight") == 0)                              {
        Par.keep[N] = atoi(value);
        Glob.shouldClear = true;
    } else if (strcmp(name, "keepbishop") == 0)                              {
        Par.keep[B] = atoi(value);
        Glob.shouldClear = true;
    } else if (strcmp(name, "keeprook") == 0)                                {
        Par.keep[R] = atoi(value);
        Glob.shouldClear = true;
    } else if (strcmp(name, "keepqueen") == 0)                               {
        Par.keep[Q] = atoi(value);
        Glob.shouldClear = true;
    } else if (strcmp(name, "bishoppairmg") == 0)                            {
        setvalue(B_PAIR_MG, atoi(value), false);
    } else if (strcmp(name, "bishoppaireg") == 0)                            {
        setvalue(B_PAIR_EG, atoi(value), false);
    } else if (strcmp(name, "knightpair") == 0)                              {
        setvalue(N_PAIR, atoi(value), false);
    } else if (strcmp(name, "rookpair") == 0)                                {
        setvalue(R_PAIR, atoi(value), false);
    } else if (strcmp(name, "exchangeimbalance") == 0)                       {
        Par.values[A_EXC] = atoi(value);
        Par.InitMaterialTweaks();
        Glob.shouldClear = true;
    } else if (strcmp(name, "minorvsqueen") == 0)                            {
        setvalue(ELEPH, atoi(value), false);
    } else if (strcmp(name, "knightlikesclosed") == 0)                       {
        Par.values[N_CL] = atoi(value);
        Par.InitMaterialTweaks();
        Glob.shouldClear = true;
    } else if (strcmp(name, "rooklikesopen") == 0)                           {
        Par.values[R_OP] = atoi(value);
        Par.InitMaterialTweaks();
        Glob.shouldClear = true;
    } else if (strcmp(name, "material") == 0)                                {
        setvalue(W_MATERIAL, atoi(value), true);
    //} else if (strcmp(name, "pieceplacement") == 0)                          {
    //    setvalue(W_MATERIAL, atoi(value), true);
    } else if (strcmp(name, "ownattack") == 0)                               {
        setvalue(W_OWN_ATT, atoi(value), false);
    } else if (strcmp(name, "oppattack") == 0)                               {
        setvalue(W_OPP_ATT, atoi(value), false);
    } else if (strcmp(name, "ownmobility") == 0)                             {
        setvalue(W_OWN_MOB, atoi(value), false);
    } else if (strcmp(name, "oppmobility") == 0)                             {
        setvalue(W_OPP_MOB, atoi(value), false);
    } else if (strcmp(name, "flatmobility") == 0)                            {
        setvalue(W_FLAT, atoi(value), false);
    } else if (strcmp(name, "kingtropism") == 0)                             {
        setvalue(W_TROPISM, atoi(value), false);
    } else if (strcmp(name, "primarypstweight") == 0)                        {
        setvalue(W_PRIM, atoi(value), false);
    } else if (strcmp(name, "secondarypstweight") == 0)                      {
        setvalue(W_SECO, atoi(value), false);
    } else if (strcmp(name, "piecepressure") == 0)                           {
        setvalue(W_THREATS, atoi(value), false);
    } else if (strcmp(name, "passedpawns") == 0)                             {
        setvalue(W_PASSERS, atoi(value), false);
    } else if (strcmp(name, "pawnstructure") == 0)                           {
        setvalue(W_STRUCT, atoi(value), false);
    } else if (strcmp(name, "pawnmass") == 0)                                {
        setvalue(W_MASS, atoi(value), false);
    } else if (strcmp(name, "pawnchains") == 0)                              {
        setvalue(W_CHAINS, atoi(value), false);
    } else if (strcmp(name, "pawnshield") == 0)                              {
        setvalue(W_SHIELD, atoi(value), false);
    } else if (strcmp(name, "pawnstorm") == 0)                               {
        setvalue(W_STORM, atoi(value), false);
    } else if (strcmp(name, "outposts") == 0)                                {
        setvalue(W_OUTPOSTS, atoi(value), false);
    } else if (strcmp(name, "space") == 0)                                   {
        setvalue(W_SPACE, atoi(value), false);
    } else if (strcmp(name, "lines") == 0)                                   {
        setvalue(W_LINES, atoi(value), false);
    } else if (strcmp(name, "fianchbase") == 0)                              {
        setvalue(B_FIANCH, atoi(value),false);
    } else if (strcmp(name, "fianchking") == 0)                              {
        setvalue(B_KING, atoi(value), false);
    } else if (strcmp(name, "returningb") == 0)                              {
        setvalue(B_RETURN, atoi(value), false);
    } else if (strcmp(name, "doubledpawnmg") == 0)                           {
        setvalue(DB_MID, atoi(value), false);
    } else if (strcmp(name, "doubledpawneg") == 0)                           {
        setvalue(DB_END, atoi(value), false);
    } else if (strcmp(name, "isolatedpawnmg") == 0)                          {
        setvalue(ISO_MG, atoi(value), false);
    } else if (strcmp(name, "isolatedpawneg") == 0)                          {
        setvalue(ISO_EG, atoi(value), false);
    } else if (strcmp(name, "isolatedonopenmg") == 0)                        {
        setvalue(ISO_OF, atoi(value), false);
    } else if (strcmp(name, "backwardpawnmg") == 0)                          {
        Par.values[BK_MID] = atoi(value);
        Par.InitBackward();
        Glob.shouldClear = true;
    } else if (strcmp(name, "backwardpawneg") == 0)                          {
        setvalue(BK_END, atoi(value), false);
    } else if (strcmp(name, "backwardonopenmg") == 0)                          {
        setvalue(BK_OPE, atoi(value), false);
    } else if (strcmp(name, "primarypststyle") == 0)                         {
        Par.primaryPstStyle = atoi(value);
        Par.InitPst();
        Glob.shouldClear = true;
    } else if (strcmp(name, "secondarypststyle") == 0)                       {
        Par.secondaryPstStyle = atoi(value);
        Par.InitPst();
        Glob.shouldClear = true;
    } else if (strcmp(name, "blockedcpawn") == 0) {
        Par.values[N_BLOCK] = atoi(value);
    // Here starts a block of non-eval options

    } else if (strcmp(name, "logfile") == 0)                           {
        LogFileWStr = CStr2WStr(value);
        if (LogFileWStr == L"<empty>")
            LogFileWStr = L"";
        else {
            if (!isabsolute(WStr2Str(LogFileWStr).c_str()))
                LogFileWStr = RodentHomeDirWStr + LogFileWStr;
            SkipBeginningOfLog = false;
            printf_debug("LogFile='%s'\n", WStr2Str(LogFileWStr).c_str());
            ReadPersonality("basic.ini"); // only for checking "CLEAR_LOG" - needed!
        }
    } else if (strcmp(name, "bookfilter") == 0)                              {
        Par.bookFilter = atoi(value);
    } else if (strcmp(name, "guidebookfile") == 0)                           {
        if (Glob.CanReadBook() ) GuideBook.SetBookName(value);
    } else if (strcmp(name, "mainbookfile") == 0)                            {
        if (Glob.CanReadBook() ) MainBook.SetBookName(value);
    } else if (strcmp(name, "contempt") == 0)                                {
        Par.drawScore = atoi(value);
        Glob.shouldClear = true;
    } else if (strcmp(name, "evalblur") == 0)                                {
        if (!Glob.isReadingPersonality) {
            Par.evalBlur = atoi(value);
            Glob.shouldClear = true;
        } else
            printf_debug("skipping when reading personalities setoption name: '%s'\n", name);
    } else if (strcmp(name, "npslimit") == 0)                                {
        if (!Glob.isReadingPersonality)
            Par.npsLimit = atoi(value);
        else
            printf_debug("skipping when reading personalities setoption name: '%s'\n", name);
    } else if (strcmp(name, "uci_elo") == 0)                                 {
        Par.elo = atoi(value);
        Par.SetSpeed(Par.elo);
    } else if (strcmp(name, "printpv") == 0)                                 {
        valuebool(Glob.printPv, value);
    } else if (strcmp(name, "taunting") == 0)                                {
        valuebool(Glob.useTaunting, value);
    } else if (strcmp(name, "verbose") == 0)                                 {
        valuebool(Glob.isNoisy, value);
    } else if (strcmp(name, "uci_limitstrength") == 0)                       {
        valuebool(Par.useWeakening, value);
        Par.SetSpeed(Par.elo);
    } else if (strcmp(name, "ponder") == 0)                                  {
        valuebool(Par.use_ponder, value);
    } else if (strcmp(name, "castlenotation") == 0)                          {
        if (strcmp(value, "O-O") == 0 || strcmp(value, "o-o") == 0)
            Glob.CastleNotation = OOO;
        else if (strcmp(value, "take") == 0)
            Glob.CastleNotation = TakeRook;
        else
            Glob.CastleNotation = KingMove;
    } else if (strcmp(name, "uci_chess960") == 0)                            {
        valuebool(Par.chess960, value);
    } else if (strcmp(name, "usebook") == 0)                                 {
        valuebool(Par.useBook, value);
    } else if (strcmp(name, "verbosebook") == 0)                             {
        valuebool(Par.verboseBook, value);
    } else if (strcmp(name, "mobilityrebalancing") == 0)                     {
    valuebool(Par.useMobilityRebalancing, value);
    } else if (strcmp(name, "searchskill") == 0)                             {
        Par.searchSkill = atoi(value);
        Glob.shouldClear = true;
    } else if (strcmp(name, "slowmover") == 0)                               {
        Par.timePercentage = atoi(value);
    } else if (strcmp(name, "selectivity") == 0)                             {
        Par.hist_perc = atoi(value);
        Par.histLimit = -MAX_HIST + ((MAX_HIST * Par.hist_perc) / 100);
        Glob.shouldClear = true;
    } else if (strcmp(name, "personalityfile") == 0)                         {
        ReadPersonality(value);
    } else if (strcmp(name, "personalityset") == 0)                          {
        for (int i = 0; i < pers_sets.count; i++)
            if (strcmp(pers_sets.alias[i], value) == 0) {
                ChangePersonalitySet(i);
                break;
            }
    } else if (strcmp(name, "personality") == 0 )                            {
        Glob.personalityW = "";
        for (int i = 0; i < pers_aliases.count; i++)
            if (strcmp(pers_aliases.alias[i], value) == 0) {
                Glob.personalityW = pers_aliases.path[i];
                if (Glob.personalityB == "")
                    ReadPersonality(Glob.personalityW.c_str());
                break;
            }
    } else if (strcmp(name, "personalityb") == 0 )                           {
        Glob.personalityB = "";
        for (int i = 0; i < pers_aliases.count; i++)
            if (strcmp(pers_aliases.alias[i], value) == 0) {
                Glob.personalityB = pers_aliases.path[i];
                break;
            }
        if (Glob.personalityB == "")
            ReadPersonality(Glob.personalityW.c_str());
    }
}

void ReadThreadNumber(const char * fileName) {
	FILE *threadFile = NULL;
	threadFile = fopen(fileName, "r");
    // printf_debug("reading threadFile '%s' (%s)\n", fileName, threadFile == NULL ? "failure" : "success");

	if (threadFile == NULL)
		return;

    char line[256];
    // int cnt = 0;
    char *pos;
	Glob.threadOverride = 0;

	while (fgets(line, sizeof(line), threadFile)) {

		while ((pos = strpbrk(line, "\r\n"))) *pos = '\0'; // clean the sh!t
		int cnt = atoi(line);
		Glob.threadOverride = cnt;
	}

	fclose(threadFile);

}

void ReadPersonality(const char *fileName) {

    // It is possible that user will attempt to load a personality of older Rodent version.
    // There is nothing wrong with that, except that there will be some parameters missing.
    // and there will be no way of telling whether previous personality used their default
    // value or not. For that reason we reset params now.

    // DefaultWeights will make user settings by calling this sub with "default.txt"
    if (strcmp(fileName,"default.txt")  // no init in init in init ...
        && strcmp(fileName,"basic.ini") // need init AFTER reading basic.ini (to use "setoption name Personality ..." there)
        && !Glob.isReadingPersonality)  // no more init, when using "setoption name Personality ..."
        Par.DefaultWeights(); // maybe better do this only if fileName exists???

    FILE *personalityFile = NULL;
    if (isabsolute(fileName)                    // if known locations don't exist we want to load only from absolute paths
        || ChDirEnv("RODENT4PERSONALITIES")     // try `RODENT4PERSONALITIES` env var first (26/08/17: linux only)
            || ChDir(_PERSONALITIESPATH))       // next built-in path
                personalityFile = fopen(fileName, "r");
    printf_debug("reading personality '%s' (%s)\n", fileName, personalityFile == NULL ? "failure" : "success");

    if (Glob.isNoisy)
        printfUciOut("info string reading personality '%s' (%s)\n", fileName, personalityFile == NULL ? "failure" : "success");

    // Exit if this personality file doesn't exist

    if (personalityFile == NULL)
        return;

    // Set flag in case we want to disable some options while reading personality from a file

    bool oldIsReadingPersonality = Glob.isReadingPersonality;
    Glob.isReadingPersonality = true;

    char line[256], token[180]; int cnt = 0; char *pos;
    int readPersonalitySet = -1; // we use 0 as first (default) personality-set

    while (fgets(line, sizeof(line), personalityFile)) {    // read options line by line

        while ((pos = strpbrk(line, "\r\n"))) *pos = '\0'; // clean the sh!t

        char *skipWS = line;
        while (*skipWS == ' ' || *skipWS == '\t')
            skipWS++;

        // Do we pick opening book within a personality?

        if (strstr(line, "PERSONALITY_BOOKS") == skipWS) Glob.useBooksFromPers = true; // DEFAULT
        if (strstr(line, "GENERAL_BOOKS") == skipWS)     Glob.useBooksFromPers = false;

        // How we go about weakening the engine?

        if (strstr(line, "ELO_SLIDER") == skipWS) Glob.eloSlider = true; // DEFAULT
        if (strstr(line, "NPS_BLUR") == skipWS)   Glob.eloSlider = false;

        // Which UCI options are exposed to the user?

        if (strstr(line, "HIDE_OPTIONS") == skipWS) Glob.usePersonalityFiles = true;
        if (strstr(line, "SHOW_OPTIONS") == skipWS) Glob.usePersonalityFiles = false; // DEFAULT
        if (strstr(line, "HIDE_PERSFILE") == skipWS) Glob.showPersonalityFile = false; // DEFAULT == true

        if (strstr(line, "GUI_PERSONALITYSET") == skipWS) Glob.useUciPersonalitySet = true;

        if (strstr(line, "CLEAR_LOG") == skipWS) {
            FILE *logFile;

            logFile = fopen(WStr2Str(LogFileWStr).c_str(), "r");
            if (logFile) {
                fclose(logFile);

                logFile = fopen(WStr2Str(LogFileWStr).c_str(), "w");
                if (logFile)
                    fclose(logFile);
                printf_debug("Log cleared\n");
            }
        }

        if (strstr(line, "SPLIT_LOG") == skipWS) {
            std::wstring::size_type pos;

            pos=LogFileWStr.find(L'.');
            if (pos != std::wstring::npos) {
                LogFileWStr = LogFileWStr.substr(0, pos);

                pos=LogFileWStr.find(L'_');
                if (pos != std::wstring::npos) {
                    LogFileWStr = LogFileWStr.substr(0, pos);
                }

                using namespace std;
                stringstream ss;
                ss << setfill('0') << setw(7) << GetMS() / 1000;

                LogFileWStr = LogFileWStr + L"_" + Str2WStr(ss.str()) + L".log";

                printf_debug("Log splitted='%s'\n", WStr2Str(LogFileWStr).c_str());
            }
        }

        // Aliases for personalities

        pos = strchr(line, '=');
        if (pos && *skipWS != ';' && *skipWS != '#' && *skipWS != '\'' && *skipWS != '/') {

            if (strstr(line, "USE_PERSONALITY_SET_NO") == skipWS) {
                pers_sets.used = *(pos+1) - '0';
                if (pers_sets.used < 0 || pers_sets.used > 9)
                    pers_sets.used = 0;

            } else if (strstr(line, "PERSONALITY_SET") == skipWS) {
                readPersonalitySet++;
                if (readPersonalitySet < 10) {
                    strncpy(pers_sets.alias[readPersonalitySet], pos+1, PERSSETALIAS_LEN-1); // -1 coz `strncpy` has a very unexpected glitch
                    pers_sets.count = readPersonalitySet+1;
                }

            } else if (pers_sets.used == readPersonalitySet) {
                if (!cnt) {
                    // add a fake alias to allow to use PersonalityFile, ReadPersonality will
                    // fail on it keeping PersonalityFile values
                    strcpy(pers_aliases.alias[cnt], "---");
                    strcpy(pers_aliases.path[cnt], "///");
                    cnt++;
                }

                *pos = '\0';
                strncpy(pers_aliases.alias[cnt], line, PERSALIAS_ALEN-1); // -1 coz `strncpy` has a very unexpected glitch
                strncpy(pers_aliases.path[cnt], pos+1, PERSALIAS_PLEN-1); // see the C11 language standard, note 308

                // test if filename exists:
                FILE *tempFile = NULL;
                tempFile = fopen(pers_aliases.path[cnt], "r");
                printf_debug("personality '%s' exists? (%s)\n", pers_aliases.path[cnt], tempFile == NULL ? "failure" : "success");

                if (tempFile) {
                    // only allow existing personalities
                    fclose(tempFile);
                    cnt++;
                }

                continue;
            }
        }

        // Personality files use the same syntax as UCI options parser (yes I have been lazy)

        const char *ptr = ParseToken(line, token);
        if (strcmp(token, "setoption") == 0)
            ParseSetoption(ptr);
    }

    if (cnt != 0) pers_aliases.count = cnt;
    fclose(personalityFile);
    Par.SpeedToBookDepth(Par.npsLimit);
    Glob.isReadingPersonality = oldIsReadingPersonality;
}

void ChangePersonalitySet(int persSetNo) {
    char line[256];
    bool needReread = false;
    FILE *basicFile = NULL;
    char fileName[] = "basic.ini";

    if (persSetNo < 0 || persSetNo > 9) {
        // allow only one-digit, so we can make binary-change (and don't need to rewrite whole file)
        printf_debug("USE_PERSONALITY_SET_NO allows only values from 0 to 9\n");
        return;
    }

    if (isabsolute(fileName)                    // if known locations don't exist we want to load only from absolute paths
        || ChDirEnv("RODENT4PERSONALITIES")     // try `RODENT4PERSONALITIES` env var first (26/08/17: linux only)
            || ChDir(_PERSONALITIESPATH))       // next built-in path
                basicFile = fopen(fileName, "r+b");
    // printf_debug("reading personality '%s' (%s)\n", fileName, basicFile == NULL ? "failure" : "success");

    if (!basicFile)
        return;

    while (fgets(line, sizeof(line), basicFile)) {    // read options line by line

        char *skipWS = line;
        while (*skipWS == ' ' || *skipWS == '\t')
            skipWS++;

        if (strstr(line, "USE_PERSONALITY_SET_NO=") == skipWS) {
            char c;

            // set position direct after '=' char:
            fseek (basicFile, ftell (basicFile) - strlen(line) + strlen("USE_PERSONALITY_SET_NO="), SEEK_SET);

            c = fgetc(basicFile); // to know if there are changes
            fseek (basicFile, ftell (basicFile) - 1, SEEK_SET);

            if (persSetNo != c-'0') {
                fprintf(basicFile,"%d", persSetNo);
                printf_debug("changed USE_PERSONALITY_SET_NO from '%c' to '%d'\n", c, persSetNo);
                needReread = true;
            } else
                printf_debug("USE_PERSONALITY_SET_NO already '%d'\n", persSetNo);

            break;
        }
    }

    fclose(basicFile);

    if (needReread)
        // Reread PersonalitySets (in case any GUI will be able to reread them without restart)
        ReadPersonality("basic.ini");
}
