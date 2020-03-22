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
#include <cstdio>
#include <cstring>

void POS::ClearPosition() {

    *this = {{0}};

    mKingSq[WC] = NO_SQ;
    mKingSq[BC] = NO_SQ;

    for (int sq = 0; sq < 64; sq++)
        mPc[sq] = NO_PC;

    mSide = WC;
    mEpSq = NO_SQ;
}

void POS::SetPosition(const char *epd) {

    static const char pc_char[] = "PpNnBbRrQqKk";

    ClearPosition();
    Glob.moves_from_start = 0;

    for (int i = 56; i >= 0; i -= 8) {
        int j = 0, pc_loop;
        while (j < 8) {
            if (*epd >= '1' && *epd <= '8')
                for (pc_loop = 0; pc_loop < *epd - '0'; pc_loop++) {
                    mPc[i + j] = NO_PC;
                    j++;
                }
            else {
                for (pc_loop = 0; pc_char[pc_loop] && pc_char[pc_loop] != *epd; pc_loop++)
                    ;

                if ( !pc_char[pc_loop] ) {
                    printfUciOut("info string FEN parsing error\n");
                    SetPosition(START_POS);
                    return;
                }

                mPc[i + j] = pc_loop;
                mClBb[Cl(pc_loop)] ^= SqBb(i + j);
                mTpBb[Tp(pc_loop)] ^= SqBb(i + j);

                if (Tp(pc_loop) == K)
                    mKingSq[Cl(pc_loop)] = i + j;

                mMgScore[Cl(pc_loop)] += Par.mg_mat[Tp(pc_loop)];
                mEgScore[Cl(pc_loop)] += Par.eg_mat[Tp(pc_loop)];
                mPhase += ph_value[Tp(pc_loop)];
                mCnt[Cl(pc_loop)][Tp(pc_loop)]++;
                j++;
            }
            epd++;
        }
        epd++;
    }
    if (*epd++ == 'w')
        mSide = WC;
    else
        mSide = BC;
    epd++;
    if (*epd == '-')
        epd++;
    else {
        eSquare Castle_R;

        Castle_W_K = (eSquare) KingSq(WC);
        Castle_B_K = (eSquare) KingSq(BC);

        for (int i = 4; i > 0; i -= 1) {

            if ((*epd >= 'A') && (*epd <= 'H')) {
                Castle_R = (eSquare) (0 + (*epd - 'A'));

                if (Castle_R > Castle_W_K) {
                    Castle_W_RK = Castle_R;
                    mCFlags |= W_KS;
                } else {
                    Castle_W_RQ = Castle_R;
                    mCFlags |= W_QS;
                }
                epd++;
            }

            if ((*epd >= 'a') && (*epd <= 'h')) {
                Castle_R = (eSquare) (56 + (*epd - 'a'));

                if (Castle_R > Castle_B_K) {
                    Castle_B_RK = Castle_R;
                    mCFlags |= B_KS;
                } else {
                    Castle_B_RQ = Castle_R;
                    mCFlags |= B_QS;
                }
                epd++;
            }

            if (*epd == 'K') {
                mCFlags |= W_KS;
                epd++;
                for (i=0 ; i < 8 ; i++)
                    if (Rooks(WC) & mClBb[WC] & SqBb(i))
                        Castle_W_RK = (eSquare) (i);
            }
            if (*epd == 'Q') {
                mCFlags |= W_QS;
                epd++;
                for (i=7 ; i >= 0 ; i--)
                    if (Rooks(WC) & mClBb[WC] & SqBb(i))
                        Castle_W_RQ = (eSquare) (i);
            }
            if (*epd == 'k') {
                mCFlags |= B_KS;
                epd++;
                for (i=0 ; i < 8 ; i++)
                    if (Rooks(BC) & mClBb[BC] & SqBb(56 + i))
                        Castle_B_RK = (eSquare) (56 + i);
            }
            if (*epd == 'q') {
                mCFlags |= B_QS;
                epd++;
                for (i=7 ; i >= 0 ; i--)
                    if (Rooks(BC) & mClBb[BC] & SqBb(56 + i))
                        Castle_B_RQ = (eSquare) (56 + i);
            }
        }

        // "MoveToStr" did not know which side to move, so correct them also for the unneeded color
        // Only need Castle_W_RQ and Castle_W_RK, but make proper things:

        if (!(mCFlags & W_KS))
            Castle_W_RK = (eSquare) ( 0 + File(Castle_B_RK));
        if (!(mCFlags & W_QS))
            Castle_W_RQ = (eSquare) ( 0 + File(Castle_B_RQ));
        if (!(mCFlags & B_KS))
            Castle_B_RK = (eSquare) (56 + File(Castle_W_RK));
        if (!(mCFlags & B_QS))
            Castle_B_RQ = (eSquare) (56 + File(Castle_W_RQ));

        // and for completeness:
        if (!(mCFlags & (W_KS | W_QS)))
            Castle_W_K = (eSquare) ( 0 + File(Castle_B_K));
        if (!(mCFlags & (B_KS | B_QS)))
            Castle_B_K = (eSquare) (56 + File(Castle_W_K));

        Init960();
    }
    epd++;
    if (*epd == '-')
        mEpSq = NO_SQ;
    else {
        mEpSq = Sq(*epd - 'a', *(epd + 1) - '1');
        if (!(BB.PawnAttacks(~mSide, mEpSq) & Pawns(mSide)))
            mEpSq = NO_SQ;
    }
    InitHashKey();
    InitPawnKey();
}
