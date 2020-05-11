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

void cEngine::EvaluateBishopPatterns(POS *p, eData *e) {

    if (p->Bishops(WC) & Mask.wb_special) {

        // white bishop trapped

        if (p->IsOnSq(WC, B, A6) && p->IsOnSq(BC, P, B5)) Add(e, WC, Par.values[B_TRAP_A3]);
        if (p->IsOnSq(WC, B, A7) && p->IsOnSq(BC, P, B6)) Add(e, WC, Par.values[B_TRAP_A2]);
        if (p->IsOnSq(WC, B, B8) && p->IsOnSq(BC, P, C7)) Add(e, WC, Par.values[B_TRAP_A2]);
        if (p->IsOnSq(WC, B, H6) && p->IsOnSq(BC, P, G5)) Add(e, WC, Par.values[B_TRAP_A3]);
        if (p->IsOnSq(WC, B, H7) && p->IsOnSq(BC, P, G6)) Add(e, WC, Par.values[B_TRAP_A2]);
        if (p->IsOnSq(WC, B, G8) && p->IsOnSq(BC, P, F7)) Add(e, WC, Par.values[B_TRAP_A2]);

        // white bishop blocked on its initial square by own pawn
        // or returning to protect castled king

        EvalBishopOnInitial(p, e, WC, C1, D2, D3, SqBb(B1) | SqBb(A1) | SqBb(A2));
        EvalBishopOnInitial(p, e, WC, F1, E2, E3, SqBb(G1) | SqBb(H1) | SqBb(H2));

        // white bishop fianchettoed

        EvalFianchetto(p, e, WC, B2, B3, A2, C2, C3, D4, E5, C5, Mask.qsCastle[WC]);
        EvalFianchetto(p, e, WC, G2, G3, H2, F2, F3, E4, D5, F5, Mask.ksCastle[WC]);

        // FRC opening pattern: blocked bishop in the corner

        if (p->IsOnSq(WC, B, H1) && p->IsOnSq(WC, P, G2)) {
            Add(e, WC, -20, -40);
        }

        if (p->IsOnSq(WC, B, A1) && p->IsOnSq(WC, P, B2)) {
            Add(e, WC, -20, -40);
        }
    }

    if (p->Bishops(BC) & Mask.bb_special) {

        // black bishop trapped

        if (p->IsOnSq(BC, B, A3) && p->IsOnSq(WC, P, B4)) Add(e, BC, Par.values[B_TRAP_A3]);
        if (p->IsOnSq(BC, B, A2) && p->IsOnSq(WC, P, B3)) Add(e, BC, Par.values[B_TRAP_A2]);
        if (p->IsOnSq(BC, B, B1) && p->IsOnSq(WC, P, C2)) Add(e, BC, Par.values[B_TRAP_A2]);
        if (p->IsOnSq(BC, B, H3) && p->IsOnSq(WC, P, G4)) Add(e, BC, Par.values[B_TRAP_A3]);
        if (p->IsOnSq(BC, B, H2) && p->IsOnSq(WC, P, G3)) Add(e, BC, Par.values[B_TRAP_A2]);
        if (p->IsOnSq(BC, B, G1) && p->IsOnSq(WC, P, F2)) Add(e, BC, Par.values[B_TRAP_A2]);

        // black bishop blocked on its initial square by own pawn
        // or returning to protect castled king

        EvalBishopOnInitial(p, e, BC, C8, D7, D6, SqBb(B8) | SqBb(A8) | SqBb(A7));
        EvalBishopOnInitial(p, e, BC, F8, E7, E6, SqBb(G8) | SqBb(H8) | SqBb(H7));

        // black bishop fianchettoed

        EvalFianchetto(p, e, BC, B7, B6, A7, C7, C6, D5, E4, C4, Mask.qsCastle[BC]);
        EvalFianchetto(p, e, BC, G7, G6, H7, F7, F6, E5, D4, F4, Mask.ksCastle[BC]);

        // FRC opening pattern: blocked bishop in the corner

        if (p->IsOnSq(BC, B, H8) && p->IsOnSq(BC, P, G7)) {
            Add(e, BC, -20, -40);
        }

        if (p->IsOnSq(BC, B, A8) && p->IsOnSq(BC, P, B7)) {
            Add(e, BC, -20, -40);
        }
    }

}

void cEngine::EvalFianchetto(POS *p, eData *e, eColor side, eSquare bsq, eSquare psq, eSquare s1, eSquare s2, 
     eSquare obl, eSquare b1, eSquare b2, eSquare b3, U64 kingMask) {

    if (p->IsOnSq(side, B, bsq)) {
       
        // fianchetto: bishop behind defended pawn

        if (p->IsOnSq(side, P, psq) && (p->IsAnyPawn(side, s1, s2))) 
            Add(e, side, Par.values[B_FIANCH]);

        // bishop protecting king

        if (p->Kings(side) & kingMask)
            Add(e, side, Par.values[B_KING], 0);

        // bishop blocked by own pawn

        if (p->IsOnSq(side, P, obl))
            Add(e, side, Par.values[B_BF_MG], Par.values[B_BF_EG]);

        // bishop blocked by defended enemy pawn

        if (p->IsOnSq(~side, P, b1) && (p->IsAnyPawn(~side, b2, b3))) 
            Add(e, side, Par.values[B_BADF]);
    }
}

void cEngine::EvalBishopOnInitial(POS *p, eData *e, eColor side, eSquare bSq, eSquare pSq, eSquare blockSq, U64 king) {

    if (p->IsOnSq(side, B, bSq)) {

        // bishop hampered by blocked central pawn

        if (p->IsOnSq(side, P, pSq) && (SqBb(blockSq) & p->Filled()))
            Add(e, side, Par.values[B_BLOCK], 0);

        // bishop returned to protect castled king's position

        if (p->Kings(side) & king)
            Add(e, side, Par.values[B_RETURN], 0);
    }
}

void cEngine::EvaluateKnightPatterns(POS *p, eData *e) {

    // trapped knight

    if (p->IsOnSq(WC, N, A7) && p->IsPawnComplex(BC, A6, B7)) Add(e, WC, Par.values[N_TRAP]);
    if (p->IsOnSq(WC, N, H7) && p->IsPawnComplex(BC, H6, G7)) Add(e, WC, Par.values[N_TRAP]);
    if (p->IsOnSq(BC, N, A2) && p->IsPawnComplex(WC, A3, B2)) Add(e, BC, Par.values[N_TRAP]);
    if (p->IsOnSq(BC, N, H2) && p->IsPawnComplex(WC, H3, G2)) Add(e, BC, Par.values[N_TRAP]);
}

void cEngine::EvaluateKingPatterns(POS *p, eData *e) {

    U64 kingMask, rookMask;

    if (p->Kings(WC) & RANK_1_BB) {

        // White castled king that cannot escape upwards

        if (p->IsOnSq(WC, K, H1) && p->IsPawnComplex(WC, H2, G2))
            Add(e, WC, Par.values[K_NO_LUFT_MG], Par.values[K_NO_LUFT_EG]);

        if (p->IsOnSq(WC, K, G1) && p->IsPawnComplex(WC, H2, G2, F2))
            Add(e, WC, Par.values[K_NO_LUFT_MG], Par.values[K_NO_LUFT_EG]);

        if (p->IsOnSq(WC, K, A1) && p->IsPawnComplex(WC, A2, B2))
            Add(e, WC, Par.values[K_NO_LUFT_MG], Par.values[K_NO_LUFT_EG]);

        if (p->IsOnSq(WC, K, B1) && p->IsPawnComplex(WC, A2, B2, C2))
            Add(e, WC, Par.values[K_NO_LUFT_MG], Par.values[K_NO_LUFT_EG]);

        // White rook blocked by uncastled king

        kingMask = SqBb(F1) | SqBb(G1);
        rookMask = SqBb(G1) | SqBb(H1) | SqBb(H2);

        if ((p->Kings(WC) & kingMask)
        && (p->Rooks(WC) & rookMask)) Add(e, WC, Par.values[R_BLOCK_MG], Par.values[R_BLOCK_EG]);

        kingMask = SqBb(B1) | SqBb(C1);
        rookMask = SqBb(A1) | SqBb(B1) | SqBb(A2);

        if ((p->Kings(WC) & kingMask)
        && (p->Rooks(WC) & rookMask)) Add(e, WC, Par.values[R_BLOCK_MG], Par.values[R_BLOCK_EG]);

        // White castling rights

        if ((p->mCFlags & W_KS)) Add(e, WC, Par.values[K_CASTLE_KS], 0);
        else if ((p->mCFlags & W_QS)) Add(e, WC, Par.values[K_CASTLE_QS], 0);
    }

    if (p->Kings(BC) & RANK_8_BB) {

        // Black castled king that cannot escape upwards

        if (p->IsOnSq(BC, K, H8) && p->IsPawnComplex(BC, H7, G7))
            Add(e, BC, Par.values[K_NO_LUFT_MG], Par.values[K_NO_LUFT_EG]);

        if (p->IsOnSq(BC, K, G8) && p->IsPawnComplex(BC, H7, G7, F7))
            Add(e, BC, Par.values[K_NO_LUFT_MG], Par.values[K_NO_LUFT_EG]);

        if (p->IsOnSq(BC, K, A8) && p->IsPawnComplex(BC, A7, B7))
            Add(e, BC, Par.values[K_NO_LUFT_MG], Par.values[K_NO_LUFT_EG]);

        if (p->IsOnSq(BC, K, B8) && p->IsPawnComplex(BC, A7, B7, C7))
            Add(e, BC, Par.values[K_NO_LUFT_MG], Par.values[K_NO_LUFT_EG]);

        // Black rook blocked by uncastled king

        kingMask = SqBb(F8) | SqBb(G8);
        rookMask = SqBb(G8) | SqBb(H8) | SqBb(H7);

        if ((p->Kings(BC) & kingMask)
        && (p->Rooks(BC) & rookMask)) Add(e, BC, Par.values[R_BLOCK_MG], Par.values[R_BLOCK_EG]);

        kingMask = SqBb(B8) | SqBb(C8);
        rookMask = SqBb(B8) | SqBb(A8) | SqBb(A7);

        if ((p->Kings(BC) & kingMask)
        && (p->Rooks(BC) & rookMask)) Add(e, BC, Par.values[R_BLOCK_MG], Par.values[R_BLOCK_EG]);

        // Black castling rights

        if ((p->mCFlags & B_KS)) Add(e, BC, Par.values[K_CASTLE_KS], 0);
        else if ((p->mCFlags & B_QS)) Add(e, BC, Par.values[K_CASTLE_QS], 0);
    }
}

void cEngine::EvaluateCentralPatterns(POS *p, eData *e) {

    // Knight blocking c pawn

    if (p->IsPawnComplex(WC, C2, D4) && p->IsOnSq(WC, N, C3)) {
        if ((p->Pawns(WC) & SqBb(E4)) == 0) Add(e, WC, Par.values[N_BLOCK], 0);
    }
    if (p->IsPawnComplex(BC, C7, D5) && p->IsOnSq(BC, N, C6)) {
        if ((p->Pawns(BC) & SqBb(E5)) == 0) Add(e, BC, Par.values[N_BLOCK], 0);
    }
}
