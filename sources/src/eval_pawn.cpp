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
#include "eval.h"
#include <cstring>

void cEngine::ClearPawnHash() {

    ZEROARRAY(mPawnTT);
}

void cEngine::EvaluatePawnStruct(POS *p, eData *e) {

    // Try to retrieve score from pawn hashtable

    int addr = p->mPawnKey % PAWN_HASH_SIZE;

    if (mPawnTT[addr].key == p->mPawnKey) {

        // pawn hashtable contains delta of white and black score

        e->mgPawns[WC] = mPawnTT[addr].mg_white_pawns;
        e->egPawns[WC] = mPawnTT[addr].eg_white_pawns;
        e->mgPawns[BC] = mPawnTT[addr].mg_black_pawns;
        e->egPawns[BC] = mPawnTT[addr].eg_black_pawns;
        return;
    }

    // Clear values

    e->mgPawns[WC] = 0;
    e->mgPawns[BC] = 0;
    e->egPawns[WC] = 0;
    e->egPawns[BC] = 0;

    // Pawn structure

    EvaluatePawns(p, e, WC);
    EvaluatePawns(p, e, BC);

    // King's pawn shield
    // (also includes pawn chains eval)

    EvaluateKing(p, e, WC);
    EvaluateKing(p, e, BC);

    // Center binds (good) and wing binds (bad)
    // - important squares controlled by two pawns

    
    int tmp = 0;
    if (e->twoPawnsTake[WC] & SqBb(D5)) tmp += V(P_BIND);
    if (e->twoPawnsTake[WC] & SqBb(E5)) tmp += V(P_BIND);
    if (e->twoPawnsTake[WC] & SqBb(D6)) tmp += V(P_BIND);
    if (e->twoPawnsTake[WC] & SqBb(E6)) tmp += V(P_BIND);

    if (p->IsOnSq(WC, P, B3) && (e->twoPawnsTake[WC] & SqBb(B5))) tmp -= V(P_BADBIND);
    if (p->IsOnSq(WC, P, G3) && (e->twoPawnsTake[WC] & SqBb(G5))) tmp -= V(P_BADBIND);

    if (p->IsOnSq(WC, K, H1) && p->IsOnSq(BC, P, H2)) tmp += 75;  // enemy pawn as a shield
    if (p->IsOnSq(WC, K, A1) && p->IsOnSq(BC, P, A2)) tmp += 75;

    AddPawns(e, WC, tmp, 0);

    tmp = 0;
    if (e->twoPawnsTake[BC] & SqBb(D4)) tmp += V(P_BIND);
    if (e->twoPawnsTake[BC] & SqBb(E4)) tmp += V(P_BIND);
    if (e->twoPawnsTake[BC] & SqBb(D3)) tmp += V(P_BIND);
    if (e->twoPawnsTake[BC] & SqBb(E3)) tmp += V(P_BIND);

    if (p->IsOnSq(BC, P, B6) && (e->twoPawnsTake[BC] & SqBb(B4))) tmp -= V(P_BADBIND);
    if (p->IsOnSq(BC, P, G6) && (e->twoPawnsTake[BC] & SqBb(G4))) tmp -= V(P_BADBIND);

    if (p->IsOnSq(BC, K, H8) && p->IsOnSq(WC, P, H7)) tmp += 75;  // enemy pawn as a shield
    if (p->IsOnSq(BC, K, A8) && p->IsOnSq(WC, P, A7)) tmp += 75;

    AddPawns(e, BC, tmp, 0);

    // King on a wing without pawns

    U64 bb_all_pawns = p->Pawns(WC) | p->Pawns(BC);

    if (bb_all_pawns) {
        if (!(bb_all_pawns & Mask.kingSide)) {
            AddPawns(e, WC, pst_empty_ks[p->mKingSq[WC]], pst_empty_ks[p->mKingSq[WC]]);
            AddPawns(e, BC, pst_empty_ks[p->mKingSq[BC]], pst_empty_ks[p->mKingSq[BC]]);
        }

        if (!(bb_all_pawns & Mask.queenSide)) {
            AddPawns(e, WC, pst_empty_qs[p->mKingSq[WC]], pst_empty_qs[p->mKingSq[WC]]);
            AddPawns(e, BC, pst_empty_qs[p->mKingSq[BC]], pst_empty_qs[p->mKingSq[BC]]);
        }
    }

    // Evaluate number of pawn islands (based on Texel)
    
    const U64 w_pawns = p->Pawns(WC);
    const U64 w_pawn_files = BB.FillSouth(w_pawns) & 0xff;
    const int w_islands = PopCnt(((~w_pawn_files) >> 1) & w_pawn_files);

    const U64 b_pawns = p->Pawns(BC);
    const U64 b_pawn_files = BB.FillSouth(b_pawns) & 0xff;
    const int b_islands = PopCnt(((~b_pawn_files) >> 1) & b_pawn_files);
    e->mgPawns[WC] -= w_islands  * V(P_ISL);
    e->mgPawns[BC] -= b_islands * V(P_ISL);
    e->egPawns[WC] -= w_islands * V(P_ISL);
    e->egPawns[BC] -= b_islands * V(P_ISL);
    

    // Save stuff in pawn hashtable.
    // Note that we save delta between white and black scores.
    // It might become a problem if we decide to print detailed eval score.

    mPawnTT[addr].key = p->mPawnKey;
    mPawnTT[addr].mg_white_pawns = e->mgPawns[WC];
    mPawnTT[addr].mg_black_pawns = e->mgPawns[BC];
    mPawnTT[addr].eg_white_pawns = e->egPawns[WC];
    mPawnTT[addr].eg_black_pawns = e->egPawns[BC];

}

void cEngine::EvaluateKing(POS *p, eData *e, eColor sd) {

    // const int qCastle[2] = { B1, B8 };
    // const int kCastle[2] = { G1, G8 };
    U64 bb_king_file, bb_next_file;
    int shield = 0;
    int storm = 0;
    int sq = kRoot[p->KingSq(sd)];

    // Evaluate shielding and storming pawns on each file.

    bb_king_file = BB.FillNorth(SqBb(sq)) | BB.FillSouth(SqBb(sq));
    EvaluateKingFile(p, sd, bb_king_file, &shield, &storm);

    bb_next_file = ShiftEast(bb_king_file);
    if (bb_next_file) EvaluateKingFile(p, sd, bb_next_file, &shield, &storm);

    bb_next_file = ShiftWest(bb_king_file);
    if (bb_next_file) EvaluateKingFile(p, sd, bb_next_file, &shield, &storm);

    AddPawns(e, sd, ((V(W_SHIELD) * shield) / 100) + ((V(W_STORM) * storm) / 100), 0);
    AddPawns(e, sd, EvaluateChains(p, sd), 0);
}

void cEngine::EvaluateKingFile(POS *p, eColor sd, U64 bb_file, int *shield, int *storm) {

    int shelter = EvaluateFileShelter(bb_file &  p->Pawns(sd), sd);
    if (p->Kings(sd) & bb_file) shelter = ((shelter * 120) / 100);
    if (bb_file & bbCentralFile) shelter /= 2;
    *shield += shelter;
    *storm += EvaluateFileStorm(p, bb_file & p->Pawns(~sd), sd);
}

int cEngine::EvaluateFileShelter(U64 bb_own_pawns, eColor sd) {

    if (!bb_own_pawns) return V(P_SH_NONE);
    if (bb_own_pawns & bbRelRank[sd][RANK_2]) return V(P_SH_2);
    if (bb_own_pawns & bbRelRank[sd][RANK_3]) return V(P_SH_3);
    if (bb_own_pawns & bbRelRank[sd][RANK_4]) return V(P_SH_4);
    if (bb_own_pawns & bbRelRank[sd][RANK_5]) return V(P_SH_5);
    if (bb_own_pawns & bbRelRank[sd][RANK_6]) return V(P_SH_6);
    if (bb_own_pawns & bbRelRank[sd][RANK_7]) return V(P_SH_7);
    return 0;
}

int cEngine::EvaluateFileStorm(POS * p, U64 bb_opp_pawns, eColor sd) {

    if (!bb_opp_pawns) return V(P_ST_OPEN);
    if (bb_opp_pawns & bbRelRank[sd][RANK_3]) return V(P_ST_3);
    if (bb_opp_pawns & bbRelRank[sd][RANK_4]) return V(P_ST_4);
    if (bb_opp_pawns & bbRelRank[sd][RANK_5]) return V(P_ST_5);
    return 0;
}

#define SQ(sq) RelSqBb(sq,sd)
#define opPawns p->Pawns(op)
#define sdPawns p->Pawns(sd)
#define OWN_PAWN(sq) (p->Pawns(sd) & RelSqBb(sq,sd))
#define OPP_PAWN(sq) (p->Pawns(op) & RelSqBb(sq,sd))
#define CONTAINS(bb, s1, s2) ((bb) & SQ(s1)) && ((bb) & SQ(s2))

// @brief EvaluateChains() gives a penalty to side being at the receiving end of the pawn chain

int cEngine::EvaluateChains(POS *p, eColor sd) {

    int mg_result = 0;
    int sq = p->mKingSq[sd];
    eColor op = ~sd;

    // basic pointy chain

    if (SqBb(sq) & Mask.ksCastle[sd]) {

        if (OPP_PAWN(E4)) {
            if (CONTAINS(opPawns, D5, C6)) // c6-d5-e4 triad
                mg_result -= (CONTAINS(sdPawns, D4, E3)) ? V(P_BIGCHAIN) : V(P_SMALLCHAIN);

            if (CONTAINS(opPawns, D5, F3)) // d5-e4-f3 triad
                mg_result -= (OWN_PAWN(E3)) ? V(P_BIGCHAIN) : V(P_SMALLCHAIN);
        }

        if (OPP_PAWN(E5)) {
            if (CONTAINS(opPawns, F4, D6)) { // d6-e5-f4 triad
                // h5
				if (OPP_PAWN(H5)) mg_result -= V(P_CS_EDGE);

                // storm of a "g" pawn in the King's Indian
                if (OPP_PAWN(G5)) {
                    mg_result -= V(P_CS1);
                    if (OPP_PAWN(H4)) return V(P_CSFAIL); // opponent did us a favour, rendering his chain immobile
                }
                if (OPP_PAWN(G4)) mg_result -= V(P_CS2);

                mg_result -= (CONTAINS(sdPawns, E4, D5)) ? V(P_BIGCHAIN) : V(P_SMALLCHAIN);
            }

            if (CONTAINS(opPawns, G3, F4)) // e5-f4-g3 triad
                mg_result -= (OWN_PAWN(F3)) ? V(P_BIGCHAIN) : V(P_SMALLCHAIN);
        }
    }

    if (SqBb(sq) & Mask.qsCastle[sd]) {

        // basic pointy chain

        if (OPP_PAWN(D4)) {
            if (CONTAINS(opPawns, E5, F6))
                mg_result -= (CONTAINS(sdPawns, E4, D3)) ? V(P_BIGCHAIN) : V(P_SMALLCHAIN);

            if (CONTAINS(opPawns, F5, C3))
                mg_result -= (SQ(D3) & sdPawns) ? V(P_BIGCHAIN) : V(P_SMALLCHAIN);
        }

        if (OPP_PAWN(D5)) {
            if (CONTAINS(opPawns, C4, E6)) {
				if (OPP_PAWN(A5)) mg_result -= V(P_CS_EDGE);
                // storm of a "b" pawn
                if (OPP_PAWN(B5)) {
                    mg_result -= V(P_CS1);
                    if (OPP_PAWN(A4)) return V(P_CSFAIL); // opponent did us a favour, rendering his chain immobile
                }
                if (OPP_PAWN(B4)) mg_result -= V(P_CS2);

                mg_result -= (CONTAINS(sdPawns, E4, D5)) ? V(P_BIGCHAIN) : V(P_SMALLCHAIN);
            }

            if (CONTAINS(opPawns, B3, C4))
                mg_result -= (OWN_PAWN(C3)) ? V(P_BIGCHAIN) : V(P_SMALLCHAIN);
        }
    }

    return (mg_result * V(W_CHAINS)) / 100;
}
