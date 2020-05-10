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

#include <random>
#include <cmath>
#include <string.h>
#include "rodent.h"

// initialize random numbers

std::mt19937_64 e2(2018);
std::uniform_int_distribution<U64> dist(std::llround(std::pow(2, 56)), std::llround(std::pow(2, 62)));

U64 POS::Random64() {
    return dist(e2);
}

bool POS::Is960() {

    if (Castle_W_RQ != A1 && mCFlags & W_QS)
        return true;
    if (Castle_W_K  != E1 && (mCFlags & W_KS || mCFlags & W_QS))
        return true;
    if (Castle_W_RK != H1 && mCFlags & W_KS)
        return true;

    if (Castle_B_RQ != A8 && mCFlags & B_QS)
        return true;
    if (Castle_B_K  != E8 && (mCFlags & B_KS || mCFlags & B_QS))
        return true;
    if (Castle_B_RK != H8 && mCFlags & B_KS)
        return true;

    return false;
}

void POS::Init960() { // static init function for chess960

    for (int sq = 0; sq < 64; sq++)
        msCastleMask[sq] = W_KS | W_QS | B_KS | B_QS;

    msCastleMask[Castle_W_RQ] &= W_KS |        B_KS | B_QS;
    msCastleMask[Castle_W_K]  &=               B_KS | B_QS;
    msCastleMask[Castle_W_RK] &=        W_QS | B_KS | B_QS;
    msCastleMask[Castle_B_RQ] &= W_KS | W_QS | B_KS       ;
    msCastleMask[Castle_B_K]  &= W_KS | W_QS              ;
    msCastleMask[Castle_B_RK] &= W_KS | W_QS        | B_QS;

    CastleMask_W_QS =      CastleMask [File(Castle_W_K)][File(Castle_W_RQ)];
    CastleMask_W_KS =      CastleMask [File(Castle_W_K)][File(Castle_W_RK)];
    CastleMask_B_QS = (U64)CastleMask [File(Castle_B_K)][File(Castle_B_RQ)] << 56;
    CastleMask_B_KS = (U64)CastleMask [File(Castle_B_K)][File(Castle_B_RK)] << 56;

    CastleFile_RQ = File(Castle_W_RQ);
    CastleFile_RK = File(Castle_W_RK);
}

void POS::Init() { // static init function

    // Init standard-chess
    Castle_W_RQ = A1;
    Castle_W_K  = E1;
    Castle_W_RK = H1;
    Castle_B_RQ = A8;
    Castle_B_K  = E8;
    Castle_B_RK = H8;
    Init960();

    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 64; j++)
            msZobPiece[i][j] = Random64();

    for (int i = 0; i < 16; i++)
        msZobCastle[i] = Random64();

    for (int i = 0; i < 8; i++)
        msZobEp[i] = Random64();
}
