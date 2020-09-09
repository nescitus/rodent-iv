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

int POS::msCastleMask[64];

const int tp_value[7] = { 100, 325, 325, 500, 1000,  0,   0 };
const int ph_value[7] = {   0,   1,   1,   2,    4,  0,   0 }; // any change requires modification in draw.cpp

U64 POS::msZobPiece[12][64];
U64 POS::msZobCastle[16];
U64 POS::msZobEp[8];

eSquare POS::Castle_W_RQ;
eSquare POS::Castle_W_K;
eSquare POS::Castle_W_RK;
eSquare POS::Castle_B_RQ;
eSquare POS::Castle_B_K;
eSquare POS::Castle_B_RK;

unsigned char CastleFile_RQ;
unsigned char CastleFile_RK;

const unsigned char POS::CastleMask [8][8] = {
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // K-A
	{0x0C,0x00,0x78,0x74,0x6C,0x5C,0x3C,0x7C}, // K-B
	{0x0A,0x08,0x00,0x70,0x68,0x58,0x38,0x78}, // K-C
	{0x06,0x04,0x00,0x00,0x60,0x50,0x30,0x70}, // K-D
	{0x0E,0x0C,0x08,0x04,0x00,0x40,0x20,0x60}, // K-E
	{0x1E,0x1C,0x18,0x14,0x0C,0x00,0x00,0x40}, // K-F
	{0x3E,0x3C,0x38,0x34,0x2C,0x1C,0x00,0x20}, // K-G
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}  // K-H
};
U64 POS::CastleMask_W_KS;
U64 POS::CastleMask_W_QS;
U64 POS::CastleMask_B_KS;
U64 POS::CastleMask_B_QS;

#ifndef NO_THREADS
    int tDepth[MAX_THREADS];
#endif
int cEngine::msMoveTime;
int cEngine::msMoveNodes;
int cEngine::msSearchDepth;
int cEngine::msStartTime;

std::wstring RodentHomeDirWStr;
std::wstring LogFileWStr;
bool SkipBeginningOfLog;
