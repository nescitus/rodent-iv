/*
Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
Copyright (C) 2011-2019 Pawel Koziol

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

        if (p->IsOnSq(WC, B, B2)) {
            if (p->IsOnSq(WC, P, C3)) Add(e, WC, Par.values[B_BF_MG], Par.values[B_BF_EG]);
            if (p->IsOnSq(WC, P, B3) && (p->IsAnyPawn(WC, A2, C2))) Add(e, WC, Par.values[B_FIANCH]);
            if (p->IsOnSq(BC, P, D4) && (p->IsAnyPawn(BC, E5, C5))) Add(e, WC, Par.values[B_BADF]);
            if (p->Kings(WC) & Mask.qsCastle[WC]) Add(e, WC, Par.values[B_KING], 0);
        }

        if (p->IsOnSq(WC, B, G2)) {
            if (p->IsOnSq(WC, P, F3)) Add(e, WC, Par.values[B_BF_MG], Par.values[B_BF_EG]);
            if (p->IsOnSq(WC, P, G3) && (p->IsAnyPawn(WC, H2, F2))) Add(e, WC, Par.values[B_FIANCH]);
            if (p->IsOnSq(BC, P, E4) && (p->IsAnyPawn(BC, D5, F5))) Add(e, WC, Par.values[B_BADF]);
            if (p->Kings(WC) & Mask.ksCastle[WC]) Add(e, WC, Par.values[B_KING], 0);
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

        if (p->IsOnSq(BC, B, B7)) {
            if (p->IsOnSq(BC, P, C6)) Add(e, BC, Par.values[B_BF_MG], Par.values[B_BF_EG]);
            if (p->IsOnSq(BC, P, B6) && (p->IsAnyPawn(BC, A7, C7))) Add(e, BC, Par.values[B_FIANCH]);
            if (p->IsOnSq(WC, P, D5) && (p->IsAnyPawn(WC, E4, C4))) Add(e, BC, Par.values[B_BADF]);
            if (p->Kings(BC) & Mask.qsCastle[BC]) Add(e, BC, Par.values[B_KING], 0);
        }

        if (p->IsOnSq(BC, B, G7)) {
            if (p->IsOnSq(BC, P, F6)) Add(e, BC, Par.values[B_BF_MG], Par.values[B_BF_EG]);
            if (p->IsOnSq(BC, P, G6) && (p->IsAnyPawn(BC, H7, F7))) Add(e, BC, Par.values[B_FIANCH]);
            if (p->IsOnSq(WC, P, E5) && (p->IsAnyPawn(WC, D4, F4))) Add(e, BC, Par.values[B_BADF]);
            if (p->Kings(BC) & Mask.ksCastle[BC]) Add(e, BC, Par.values[B_KING], 0);
        }
    }

}

void cEngine::EvalBishopOnInitial(POS *p, eData *e, eColor side, eSquare bSq, eSquare pSq, eSquare blockSq, U64 king) {

    if (p->IsOnSq(side, B, bSq)) {
        if (p->IsOnSq(side, P, pSq) && (SqBb(blockSq) & p->Filled()))
            Add(e, side, Par.values[B_BLOCK], 0);
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

    U64 king_mask, rook_mask;

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

        king_mask = SqBb(F1) | SqBb(G1);
        rook_mask = SqBb(G1) | SqBb(H1) | SqBb(H2);

        if ((p->Kings(WC) & king_mask)
        && (p->Rooks(WC) & rook_mask)) Add(e, WC, Par.values[R_BLOCK_MG], Par.values[R_BLOCK_EG]);

        king_mask = SqBb(B1) | SqBb(C1);
        rook_mask = SqBb(A1) | SqBb(B1) | SqBb(A2);

        if ((p->Kings(WC) & king_mask)
        && (p->Rooks(WC) & rook_mask)) Add(e, WC, Par.values[R_BLOCK_MG], Par.values[R_BLOCK_EG]);

        // White castling rights

        if (p->IsOnSq(WC, K, E1)) {
            if ((p->mCFlags & W_KS)) Add(e, WC, Par.values[K_CASTLE_KS], 0);
			else if ((p->mCFlags & W_QS)) Add(e, WC, Par.values[K_CASTLE_QS], 0);
        }
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

        king_mask = SqBb(F8) | SqBb(G8);
        rook_mask = SqBb(G8) | SqBb(H8) | SqBb(H7);

        if ((p->Kings(BC) & king_mask)
        && (p->Rooks(BC) & rook_mask)) Add(e, BC, Par.values[R_BLOCK_MG], Par.values[R_BLOCK_EG]);

        king_mask = SqBb(B8) | SqBb(C8);
        rook_mask = SqBb(B8) | SqBb(A8) | SqBb(A7);

        if ((p->Kings(BC) & king_mask)
        && (p->Rooks(BC) & rook_mask)) Add(e, BC, Par.values[R_BLOCK_MG], Par.values[R_BLOCK_EG]);

        // Black castling rights

        if (p->IsOnSq(BC, K, E8)) {
            if ((p->mCFlags & B_KS)) Add(e, BC, Par.values[K_CASTLE_KS], 0);
			else if ((p->mCFlags & B_QS)) Add(e, BC, Par.values[K_CASTLE_QS], 0);
        }
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
