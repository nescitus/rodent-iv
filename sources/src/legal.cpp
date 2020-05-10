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

bool POS::Legal(int move) const {

    eColor sd = mSide;
    int fsq = Fsq(move);
    int tsq = Tsq(move);
    int ftp = TpOnSq(fsq);
    int ttp = TpOnSq(tsq);
	int pos;

    if ((ftp == NO_TP || Cl(mPc[fsq]) != sd) ||
        (ttp != NO_TP && Cl(mPc[tsq]) == sd && MoveType(move) != CASTLE))
        return false;

    switch (MoveType(move)) {
        case NORMAL:
            break;
        case CASTLE:
            if (sd == WC) {
                if (fsq != Castle_W_K)
                    return false;
                if (tsq == G1) {
                    if ((mCFlags & W_KS) && !(Filled() & CastleMask_W_KS))
                        if (!Attacked(Castle_W_K, BC)) {
                            for (pos = Castle_W_K + 1 ; pos < G1 ; pos++)
                                if (Attacked(pos, BC))
                                    return false;
                            return true;
                        }
                } else {
                    if ((mCFlags & W_QS) && !(Filled() & CastleMask_W_QS))
                        if (!Attacked(Castle_W_K, BC)) {
                            for (pos = Castle_W_K - 1 ; pos > C1 ; pos--)
                                if (Attacked(pos, BC))
                                    return false;
                            return true;
                        }
                }
            } else {
                if (fsq != Castle_B_K)
                    return false;
                if (tsq == G8) {
                    if ((mCFlags & B_KS) && !(Filled() & CastleMask_B_KS))
                        if (!Attacked(Castle_B_K, WC)) {
                            for (pos = Castle_B_K + 1 ; pos < G8 ; pos++)
                                if (Attacked(pos, WC))
                                    return false;
                            return true;
                        }
                } else {
                    if ((mCFlags & B_QS) && !(Filled() & CastleMask_B_QS))
                        if (!Attacked(Castle_B_K, WC)) {
                            for (pos = Castle_B_K - 1 ; pos > C8 ; pos--)
                                if (Attacked(pos, WC))
                                    return false;
                            return true;
                        }
                }
            }
            return false;
        case EP_CAP:
            if (ftp == P && tsq == mEpSq)
                return true;
            return false;
        case EP_SET:
            if (ftp == P && ttp == NO_TP && mPc[tsq ^ 8] == NO_PC)
                if ((tsq > fsq && sd == WC) ||
                    (tsq < fsq && sd == BC))
                    return true;
            return false;
    }

    if (ftp == P) {
        if (sd == WC) {
            if (Rank(fsq) == RANK_7 && !IsProm(move))
                return false;
            if (tsq - fsq == 8)
                if (ttp == NO_TP)
                    return true;
            if ((tsq - fsq == 7 && File(fsq) != FILE_A) ||
                (tsq - fsq == 9 && File(fsq) != FILE_H))
                if (ttp != NO_TP)
                    return true;
        } else {
            if (Rank(fsq) == RANK_2 && !IsProm(move))
                return false;
            if (tsq - fsq == -8)
                if (ttp == NO_TP)
                    return true;
            if ((tsq - fsq == -9 && File(fsq) != FILE_A) ||
                (tsq - fsq == -7 && File(fsq) != FILE_H))
                if (ttp != NO_TP)
                    return true;
        }
        return false;
    }

    if (IsProm(move))
        return false;

    return (AttacksFrom(fsq) & SqBb(tsq)) != 0;
}

bool POS::Unambiguous(int move) const {

#ifndef DEBUG
    // When OOO- or TakeRook-mode all is fine, but maybe we still like to know in debug-build
    if (Glob.CastleNotation == OOO || Glob.CastleNotation == TakeRook)
        return true;
#endif

    int fsq = Fsq(move);
    int tsq = Tsq(move);
    std::string moveTypeStr;

    if (Legal((NORMAL << 12) | (tsq << 6) | fsq) &&
        Legal((CASTLE << 12) | (tsq << 6) | fsq)) {

        if (MoveType(move) == CASTLE)
            moveTypeStr = "castling";
        else
            moveTypeStr = "noncastling";

        if (Glob.CastleNotation == KingMove) {
            // "Chess from Jeroen Carolus" uses this as default
            printfUciOut("info string ambiguous move - like to do %s\n", moveTypeStr.c_str());
            return false;
        } else
            printf_debug("ambiguous move in standard notation - do %s\n", moveTypeStr.c_str());
    }

    return true;
}
