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
#include <cstdio>
#include <cstdlib>
#include <cmath>

void cParam::Recalculate() {

    InitPst();
    InitMaterialTweaks();
    InitBackward();
    InitPassers();
}

void cParam::DefaultWeights() {  // tuned automatically

    // Switch off weakening parameters

    searchSkill = 10;
    // npsLimit = 0; // became part of SetSpeed
    useWeakening = true;
    // elo = 2800; // was set at programstart - keep that value
    // evalBlur = 0; // became part of SetSpeed
    useMobilityRebalancing = false;

    // Opening book

    // bookDepth = 256; // became part of SetSpeed
    bookFilter = 20;

    // Timing

    timePercentage = 100;

    // Piece values

    static const bool tunePieceValues = true;

    SetVal(P_MID,   90,  50, 150, tunePieceValues);
    SetVal(N_MID,  380, 200, 400, tunePieceValues);
    SetVal(B_MID,  390, 200, 400, tunePieceValues);
    SetVal(R_MID,  530, 400, 700, tunePieceValues);
    SetVal(Q_MID, 1160, 800, 1500, tunePieceValues);

    SetVal(P_END,  110,  50, 150, tunePieceValues);
    SetVal(N_END,  360, 200, 400, tunePieceValues);
    SetVal(B_END,  370, 200, 400, tunePieceValues);
    SetVal(R_END,  650, 400, 700, tunePieceValues);
    SetVal(Q_END, 1190, 800, 1500, tunePieceValues);

    // Tendency to keep own pieces

    for (int i = 0; i < 7; i++) {
        keep[i] = 0;
    }

    // Material adjustments

    static const bool tuneAdj = false;

    SetVal(B_PAIR_MG, 51, -100, 100, tuneAdj);
    SetVal(B_PAIR_EG, 51, -100, 100, tuneAdj);
    SetVal(N_PAIR, -1, -50, 50, tuneAdj);
    SetVal(R_PAIR, -11, -50, 50, tuneAdj);
    SetVal(ELEPH,  5, -50, 50, tuneAdj);   // queen loses that much with each enemy minor on the board
    SetVal(A_EXC, 10, -50, 50, tuneAdj);   // exchange advantage additional bonus (20 is worse)
    SetVal(N_CL, 6, -50, 50, tuneAdj);     // knight gains this much with each own pawn present on the board
    SetVal(R_OP, 0, -50, 50, tuneAdj);     // rook loses that much with each own pawn present on the board  

    // King attack values

    // "_ATT1" values are awarded for attacking squares not defended by enemy pawns
    // "_ATT2" values are awarded for attacking squares defended by enemy pawns
    // "_CHK"  values are awarded for threatening check to enemy king
    // "_CONTACT" values are awarded for contact checks threats
    //
    // All these values are NOT the actual bonuses; their sum is used as index
    // to a non-linear king safety table. Tune them with extreme caution.

    static const bool tuneAttack = true;

    SetVal(N_ATT1, 5,  0, 50, tuneAttack);
    SetVal(N_ATT2, 5,  0, 50, tuneAttack);
    SetVal(B_ATT1, 7,  0, 50, tuneAttack);
    SetVal(B_ATT2, 3,  0, 50, tuneAttack);
    SetVal(R_ATT1, 12, 0, 50, tuneAttack);
    SetVal(R_ATT2, 4,  0, 50, tuneAttack);
    SetVal(Q_ATT1, 15, 0, 50, tuneAttack);
    SetVal(Q_ATT2, 5,  0, 50, tuneAttack);

    SetVal(N_CHK,  12, 0, 50, tuneAttack);
    SetVal(B_CHK,  20, 0, 50, tuneAttack);
    SetVal(R_CHK,  18, 0, 50, tuneAttack);
    SetVal(Q_CHK,  12, 0, 50, tuneAttack);

    SetVal(Q_CONTACT, 36, 0, 50, tuneAttack);

    // Varia

    SetVal(W_MATERIAL, 100,  0, 200, false);
    SetVal(W_PRIM, 58, 0, 200, false);
    SetVal(W_SECO, 40, 0, 200, false);
    primaryPstStyle = 0;
    secondaryPstStyle = 1;

    // Attack and mobility weights that can be set independently for each side
    // - the core of personality mechanism

    SetVal(W_OWN_ATT, 110, 0, 500, false);
    SetVal(W_OPP_ATT, 110, 0, 500, false);
    SetVal(W_OWN_MOB,  50, 0, 500, false);
    SetVal(W_OPP_MOB,  50, 0, 500, false);
    SetVal(W_FLAT,     50, 0, 500, false);

    // Positional weights

	static const bool tuneWeights = false;

    SetVal(W_THREATS, 109, 0, 500, tuneWeights);
    SetVal(W_TROPISM,  25, -500, 500, tuneWeights); // 33 is almost the same
    SetVal(W_PASSERS, 102, 0, 500, tuneWeights);
    SetVal(W_MASS,     98, 0, 500, tuneWeights);
    SetVal(W_CHAINS,  100, 0, 500, tuneWeights);
    SetVal(W_OUTPOSTS, 73, 0, 500, tuneWeights);
    SetVal(W_LINES,   109, 0, 500, tuneWeights);
    SetVal(W_STRUCT,  113, 0, 500, tuneWeights);
    SetVal(W_SHIELD,  120, 0, 500, tuneWeights);
    SetVal(W_STORM,    95, 0, 500, tuneWeights);
    SetVal(W_SPACE,     0, 0, 500, tuneWeights);

    // Pawn structure parameters

    static const bool tuneStruct = false;

    SetVal(DB_MID,  -8, -50, 0, tuneStruct);  // doubled
    SetVal(DB_END, -21, -50, 0, tuneStruct);

    SetVal(ISO_MG,  -7, -50, 0, tuneStruct);  // isolated
    SetVal(ISO_EG,  -7, -50, 0, tuneStruct);
    SetVal(ISO_OF, -13, -50, 0, tuneStruct);  // additional midgame penalty for isolated pawn on an open file

    SetVal(BK_MID,  -2, -50, 0, tuneStruct);  // backward
    SetVal(BK_END,  -1, -50, 0, tuneStruct); 
    SetVal(BK_OPE, -10, -50, 0, tuneStruct);  // additional midgame penalty for backward pawn on an open file

    SetVal(P_BIND,   2, 0, 50, tuneStruct);   // two pawns control central square
    SetVal(P_BADBIND, 13, 0, 50, tuneStruct); // penalty for a "wing triangle" like a4-b3-c4

    SetVal(P_ISL, 5, 0, 50, tuneStruct);      // penalty for each pawn island

    // Pawn chain values

    static const bool tuneChain = true;

    SetVal(P_BIGCHAIN, 38, 0, 50, tuneChain);   // general penalty for a compact pawn chain pointing at our king
    SetVal(P_SMALLCHAIN, 27, 0, 50, tuneChain); // similar penalty for a chain that is not fully blocked by enemy pawns
    SetVal(P_CS1, 12, 0, 50, tuneChain);        // bonus for a pawn storm next to a fixed chain - like g5 in King's Indian
    SetVal(P_CS2, 4, 0, 50, tuneChain);         // as above, this time like g4 in King's Indian
	SetVal(P_CS_EDGE, 8, 0, 50, tuneChain);     // similarly, h5 in King's Indian
    SetVal(P_CSFAIL, 32, 0, 50, tuneChain);     // penalty for misplaying pawn strom next to a chain

    // Passed pawn bonuses per rank

    static const bool tunePassers = false;

    SetVal(PMG2,   2, 0, 300, tunePassers);
    SetVal(PMG3,   2, 0, 300, tunePassers);
    SetVal(PMG4,  11, 0, 300, tunePassers);
    SetVal(PMG5,  33, 0, 300, tunePassers);
    SetVal(PMG6,  71, 0, 300, tunePassers);
    SetVal(PMG7, 135, 0, 300, tunePassers);

    SetVal(PEG2,  12, 0, 300, tunePassers);
    SetVal(PEG3,  21, 0, 300, tunePassers);
    SetVal(PEG4,  48, 0, 300, tunePassers);
    SetVal(PEG5,  93, 0, 300, tunePassers);
    SetVal(PEG6, 161, 0, 300, tunePassers);
    SetVal(PEG7, 266, 0, 300, tunePassers);

    // Passed pawn value percentage modifiers

    SetVal(P_BL_MUL, 42, 0, 50, tunePassers);      // blocked passer
    SetVal(P_OURSTOP_MUL, 27, 0, 50, tunePassers); // side with a passer controls its stop square
    SetVal(P_OPPSTOP_MUL, 29, 0, 50, tunePassers); // side playing against a passer controls its stop square
    SetVal(P_DEFMUL, 6, 0, 50, tunePassers);       // passer defended by own pawn
    SetVal(P_STOPMUL, 6, 0, 50, tunePassers);      // passers' stop square defended by own pawn

												   // Candidate passer bonuses per rank

	static const bool tuneCandidates = false;

	SetVal(CMG2,  0, 0, 300, tuneCandidates);
	SetVal(CMG3,  0, 0, 300, tuneCandidates);
	SetVal(CMG4,  3, 0, 300, tuneCandidates);
	SetVal(CMG5, 11, 0, 300, tuneCandidates);
	SetVal(CMG6, 23, 0, 300, tuneCandidates);

	SetVal(CEG2,  4, 0, 300, tuneCandidates);
	SetVal(CEG3,  7, 0, 300, tuneCandidates);
	SetVal(CEG4, 16, 0, 300, tuneCandidates);
	SetVal(CEG5, 31, 0, 300, tuneCandidates);
	SetVal(CEG6, 53, 0, 300, tuneCandidates);

    // King's pawn shield

    static const bool tuneShield = false;

    SetVal(P_SH_NONE, -40, -50, 50, tuneShield);
    SetVal(P_SH_2,   2, -50, 50, tuneShield);
    SetVal(P_SH_3,  -6, -50, 50, tuneShield);
    SetVal(P_SH_4, -15, -50, 50, tuneShield);
    SetVal(P_SH_5, -23, -50, 50, tuneShield);
    SetVal(P_SH_6, -24, -50, 50, tuneShield);
    SetVal(P_SH_7, -35, -50, 50, tuneShield);

    // Pawn storm

    SetVal(P_ST_OPEN, -6, -50, 50, tuneShield);
    SetVal(P_ST_3, -16, -50, 50, tuneShield);
    SetVal(P_ST_4, -16, -50, 50, tuneShield);
    SetVal(P_ST_5, -3, -50, 50, tuneShield);

    // Knight parameters

    static const bool tuneKnight = true;

    SetVal(N_TRAP, -168, -300, 0, tuneKnight); // trapped knight
    SetVal(N_BLOCK, -17, -50, 0, tuneKnight);  // knight blocks c pawn in queen pawn openings
    SetVal(N_OWH_MG, -1, -50, 0, tuneKnight);  // knight can move only to own half of the board (removal failed)
	SetVal(N_OWH_EG,  0, -50, 0, tuneKnight);  // knight can move only to own half of the board
    SetVal(N_REACH_MG, 11, 0, 50, tuneKnight); // knight can reach an outpost square - midgame bonus
	SetVal(N_REACH_EG, 4, 0, 50, tuneKnight);  // knight can reach an outpost square - endgame bonus
    SetVal(N_SH_MG,  7,  0, 50, tuneKnight);   // pawn in front of a knight - midgame bonus
	SetVal(N_SH_EG, 7, 0, 50, tuneKnight);     // pawn in front of a knight - endgame bonus

    // Bishop parameters

//FIT: 0.055117

    static const bool tuneBishop = false;

    SetVal(B_FIANCH, 13, 0, 50, tuneBishop);   // general bonus for fianchettoed bishop
    SetVal(B_KING, 20, 0, 50, tuneBishop);     // fianchettoed bishop near own king
    SetVal(B_BADF, -27, -50, 0, tuneBishop);   // enemy pawns hamper fianchettoed bishop 
    SetVal(B_TRAP_A2, -138, -300, 0, tuneBishop);
    SetVal(B_TRAP_A3, -45, -300, 0, tuneBishop);
    SetVal(B_BLOCK, -45, -100, 0, tuneBishop); // blocked pawn at d2/e2 hampers bishop's development
    SetVal(B_BF_MG, -12, -50, 0, tuneBishop);  // fianchettoed bishop blocked by own pawn (ie. Bg2, Pf3)
    SetVal(B_BF_EG, -20, -50, 0, tuneBishop);
    SetVal(B_OWH_MG, -3, -50, 0, true);        // bishop can move only to own half of the board
	SetVal(B_OWH_EG, -7, -50, 0, true);        // bishop can move only to own half of the board
    SetVal(B_OWN_P, -4, -50, 0, false);        // own pawn on the square of own bishop's color
    SetVal(B_OPP_P, -1, -50, 0, false);        // enemy pawn on the square of own bishop's color
    SetVal(B_RETURN, 7, 0, 50, tuneBishop);    // bishop returning to initial position after castling
    SetVal(B_REACH_MG, 5, 0, 50, tuneBishop);  // bishop can reach an outpost square - midgame bonus
    SetVal(B_REACH_EG, 0, 0, 50, tuneBishop);  // bishop can reach an outpost square - endgame bonus
    SetVal(B_SH_MG, 5, 0, 50, tuneBishop);     // pawn in front of a bishop - midgame bonus
    SetVal(B_SH_EG, 5, 0, 50, tuneBishop);     // pawn in front of a bishop - endgame bonus

    // Rook parameters

    static const bool tuneRook = false;

    SetVal(RSR_MG, 16, 0, 50, tuneRook); // rook on the 7th rank
    SetVal(RSR_EG, 32, 0, 50, tuneRook);
    SetVal(RS2_MG, 20, 0, 50, tuneRook); // additional bonus for two rooks on 7th rank
    SetVal(RS2_EG, 31, 0, 50, tuneRook);
    SetVal(ROF_MG, 30, 0, 50, tuneRook); // rook on open file
    SetVal(ROF_EG,  2, 0, 50, tuneRook);
    SetVal(RGH_MG, 15, 0, 50, tuneRook); // rook on half-open file with undefended enemy pawn
    SetVal(RGH_EG, 20, 0, 50, tuneRook);
    SetVal(RBH_MG,  0, 0, 50, tuneRook); // rook on half-open file with defended enemy pawn
    SetVal(RBH_EG,  0, 0, 50, tuneRook);
    SetVal(ROQ_MG,  9, 0, 50, tuneRook); // rook and queen on the same file, open or closed
    SetVal(ROQ_EG, 18, 0, 50, tuneRook);
    SetVal(R_BLOCK_MG, -50, -100, 0, tuneRook);
    SetVal(R_BLOCK_EG, -20, -100, 0, tuneRook);

    // Queen parameters

	static const bool tuneQueen = false;

    SetVal(QSR_MG, 0, 0, 50, tuneQueen);       // queen on the 7th rank
    SetVal(QSR_EG, 2, 0, 50, tuneQueen);

    // King parameters

    static const bool tuneKing = true;

    SetVal(K_NO_LUFT_MG, -11, -50,  0, tuneKing); // king cannot move upwards
	SetVal(K_NO_LUFT_EG, -11, -50,  0, tuneKing);
    SetVal(K_CASTLE_KS,   32,   0, 50, tuneKing);
	SetVal(K_CASTLE_QS,   21,   0, 50, tuneKing);

    // Mobility

    SetVal(P_MOB_MG, 2, 0, 50, true);
    SetVal(P_MOB_EG, 2, 0, 50, true);

    drawScore = 0;
    shut_up = false;       // true suppresses displaying info currmove etc.

    Recalculate();         // some values need to be calculated anew after the parameter change

    // History limit to prunings and reductions

    hist_perc = 175;
    histLimit = 24576;

    // user definded default values
    ReadPersonality("default.txt");
}

void cParam::InitPassers() {

    passed_bonus_mg[WC][0] = 0;                passed_bonus_mg[BC][7] = 0;
    passed_bonus_mg[WC][1] = values[PMG2];     passed_bonus_mg[BC][6] = values[PMG2];
    passed_bonus_mg[WC][2] = values[PMG3];     passed_bonus_mg[BC][5] = values[PMG3];
    passed_bonus_mg[WC][3] = values[PMG4];     passed_bonus_mg[BC][4] = values[PMG4];
    passed_bonus_mg[WC][4] = values[PMG5];     passed_bonus_mg[BC][3] = values[PMG5];
    passed_bonus_mg[WC][5] = values[PMG6];     passed_bonus_mg[BC][2] = values[PMG6];
    passed_bonus_mg[WC][6] = values[PMG7];     passed_bonus_mg[BC][1] = values[PMG7];
    passed_bonus_mg[WC][7] = 0;                passed_bonus_mg[BC][0] = 0;

    passed_bonus_eg[WC][0] = 0;                passed_bonus_eg[BC][7] = 0;
    passed_bonus_eg[WC][1] = values[PEG2];     passed_bonus_eg[BC][6] = values[PEG2];
    passed_bonus_eg[WC][2] = values[PEG3];     passed_bonus_eg[BC][5] = values[PEG3];
    passed_bonus_eg[WC][3] = values[PEG4];     passed_bonus_eg[BC][4] = values[PEG4];
    passed_bonus_eg[WC][4] = values[PEG5];     passed_bonus_eg[BC][3] = values[PEG5];
    passed_bonus_eg[WC][5] = values[PEG6];     passed_bonus_eg[BC][2] = values[PEG6];
    passed_bonus_eg[WC][6] = values[PEG7];     passed_bonus_eg[BC][1] = values[PEG7];
    passed_bonus_eg[WC][7] = 0;                passed_bonus_eg[BC][0] = 0;

	cand_bonus_mg[WC][0] = 0;                cand_bonus_mg[BC][7] = 0;
	cand_bonus_mg[WC][1] = values[CMG2];     cand_bonus_mg[BC][6] = values[CMG2];
	cand_bonus_mg[WC][2] = values[CMG3];     cand_bonus_mg[BC][5] = values[CMG3];
	cand_bonus_mg[WC][3] = values[CMG4];     cand_bonus_mg[BC][4] = values[CMG4];
	cand_bonus_mg[WC][4] = values[CMG5];     cand_bonus_mg[BC][3] = values[CMG5];
	cand_bonus_mg[WC][5] = values[CMG6];     cand_bonus_mg[BC][2] = values[CMG6];
	cand_bonus_mg[WC][6] = 0;                cand_bonus_mg[BC][1] = 0;
	cand_bonus_mg[WC][7] = 0;                cand_bonus_mg[BC][0] = 0;

	cand_bonus_eg[WC][0] = 0;                cand_bonus_eg[BC][7] = 0;
	cand_bonus_eg[WC][1] = values[CEG2];     cand_bonus_eg[BC][6] = values[CEG2];
	cand_bonus_eg[WC][2] = values[CEG3];     cand_bonus_eg[BC][5] = values[CEG3];
	cand_bonus_eg[WC][3] = values[CEG4];     cand_bonus_eg[BC][4] = values[CEG4];
	cand_bonus_eg[WC][4] = values[CEG5];     cand_bonus_eg[BC][3] = values[CEG5];
	cand_bonus_eg[WC][5] = values[CEG6];     cand_bonus_eg[BC][2] = values[CEG6];
	cand_bonus_eg[WC][6] = 0;                cand_bonus_eg[BC][1] = 0;
	cand_bonus_eg[WC][7] = 0;                cand_bonus_eg[BC][0] = 0;
}

void cParam::InitBackward() {

    // add file-dependent component to backward pawns penalty
    // (assuming backward pawns on central files are bigger liability)

    backward_malus_mg[FILE_A] = values[BK_MID] + 3;
    backward_malus_mg[FILE_B] = values[BK_MID] + 1;
    backward_malus_mg[FILE_C] = values[BK_MID] - 1;
    backward_malus_mg[FILE_D] = values[BK_MID] - 3;
    backward_malus_mg[FILE_E] = values[BK_MID] - 3;
    backward_malus_mg[FILE_F] = values[BK_MID] - 1;
    backward_malus_mg[FILE_G] = values[BK_MID] + 1;
    backward_malus_mg[FILE_H] = values[BK_MID] + 3;
}

void cParam::InitPst() {

    mg_mat[P] = ((values[P_MID] * Par.values[W_MATERIAL]) / 100);
    eg_mat[P] = ((values[P_END] * Par.values[W_MATERIAL]) / 100);
    mg_mat[N] = ((values[N_MID] * Par.values[W_MATERIAL]) / 100);
    eg_mat[N] = ((values[N_END] * Par.values[W_MATERIAL]) / 100);
    mg_mat[B] = ((values[B_MID] * Par.values[W_MATERIAL]) / 100);
    eg_mat[B] = ((values[B_END] * Par.values[W_MATERIAL]) / 100);
    mg_mat[R] = ((values[R_MID] * Par.values[W_MATERIAL]) / 100);
    eg_mat[R] = ((values[R_END] * Par.values[W_MATERIAL]) / 100);
    mg_mat[Q] = ((values[Q_MID] * Par.values[W_MATERIAL]) / 100);
    eg_mat[Q] = ((values[Q_END] * Par.values[W_MATERIAL]) / 100);
    mg_mat[K] = 0;
    eg_mat[K] = 0;

    for (int sq = 0; sq < 64; sq++) {
        for (eColor sd = WC; sd < 2; ++sd) {

            mgPrimaryPstData[sd][P][REL_SQ(sq, sd)] = pstPawnMg [primaryPstStyle][sq];
            egPrimaryPstData[sd][P][REL_SQ(sq, sd)] = pstPawnEg[primaryPstStyle][sq];
            mgPrimaryPstData[sd][N][REL_SQ(sq, sd)] = pstKnightMg[primaryPstStyle][sq];
            egPrimaryPstData[sd][N][REL_SQ(sq, sd)] = pstKnightEg[primaryPstStyle][sq];
            mgPrimaryPstData[sd][B][REL_SQ(sq, sd)] = pstBishopMg[primaryPstStyle][sq];
            egPrimaryPstData[sd][B][REL_SQ(sq, sd)] = pstBishopEg[primaryPstStyle][sq];
            mgPrimaryPstData[sd][R][REL_SQ(sq, sd)] = pstRookMg[primaryPstStyle][sq];
            egPrimaryPstData[sd][R][REL_SQ(sq, sd)] = pstRookEg[primaryPstStyle][sq];
            mgPrimaryPstData[sd][Q][REL_SQ(sq, sd)] = pstQueenMg[primaryPstStyle][sq];
            egPrimaryPstData[sd][Q][REL_SQ(sq, sd)] = pstQueenEg[primaryPstStyle][sq];
            mgPrimaryPstData[sd][K][REL_SQ(sq, sd)] = pstKingMg[primaryPstStyle][sq];
            egPrimaryPstData[sd][K][REL_SQ(sq, sd)] = pstKingEg[primaryPstStyle][sq];

            mgSecondaryPstData[sd][P][REL_SQ(sq, sd)] = pstPawnMg[secondaryPstStyle][sq];
            egSecondaryPstData[sd][P][REL_SQ(sq, sd)] = pstPawnEg[secondaryPstStyle][sq];
            mgSecondaryPstData[sd][N][REL_SQ(sq, sd)] = pstKnightMg[secondaryPstStyle][sq];
            egSecondaryPstData[sd][N][REL_SQ(sq, sd)] = pstKnightEg[secondaryPstStyle][sq];
            mgSecondaryPstData[sd][B][REL_SQ(sq, sd)] = pstBishopMg[secondaryPstStyle][sq];
            egSecondaryPstData[sd][B][REL_SQ(sq, sd)] = pstBishopEg[secondaryPstStyle][sq];
            mgSecondaryPstData[sd][R][REL_SQ(sq, sd)] = pstRookMg[secondaryPstStyle][sq];
            egSecondaryPstData[sd][R][REL_SQ(sq, sd)] = pstRookEg[secondaryPstStyle][sq];
            mgSecondaryPstData[sd][Q][REL_SQ(sq, sd)] = pstQueenMg[secondaryPstStyle][sq];
            egSecondaryPstData[sd][Q][REL_SQ(sq, sd)] = pstQueenEg[secondaryPstStyle][sq];
            mgSecondaryPstData[sd][K][REL_SQ(sq, sd)] = pstKingMg[secondaryPstStyle][sq];
            egSecondaryPstData[sd][K][REL_SQ(sq, sd)] = pstKingEg[secondaryPstStyle][sq];

            sp_pst[sd][N][REL_SQ(sq, sd)] = pstKnightOutpost[sq];
            sp_pst[sd][B][REL_SQ(sq, sd)] = pstBishopOutpost[sq];
            sp_pst[sd][DEF_MG][REL_SQ(sq, sd)] = pstDefendedPawnMg[sq];
            sp_pst[sd][PHA_MG][REL_SQ(sq, sd)] = pstPhalanxPawnMg[sq];
            sp_pst[sd][DEF_EG][REL_SQ(sq, sd)] = pstDefendedPawnEg[sq];
            sp_pst[sd][PHA_EG][REL_SQ(sq, sd)] = pstPhalanxPawnEg[sq];
        }
    }
}

void cParam::InitMaterialTweaks() {

    // Init tables for adjusting piece values
    // according to the number of own pawns

    for (int i = 0; i < 9; i++) {
        np_table[i] = adj[i] * values[N_CL];
        rp_table[i] = adj[i] * values[R_OP];
    }
}

void cParam::InitKingAttackTable() {

    for (int t = 0, i = 1; i < 511; ++i) {
        t = (int)Min(1280.0, Min((((double)27 * 0.001) * i * i), t + double(8)));
        danger[i] = (t * 100) / 256; // rescale to centipawns
    }
}

void cParam::SetSpeed(int elo_in) {
    npsLimit = 0;
    evalBlur = 0;
    bookDepth = 256;

    if (useWeakening && elo_in<2800) {
        printf_debug("set ELO to %d\n", elo_in);
        npsLimit = EloToSpeed(elo_in);
        evalBlur = EloToBlur(elo_in);
        bookDepth = SpeedToBookDepth(npsLimit);
    } else
        printf_debug("set ELO to maximum\n");
}

int cParam::EloToSpeed(int elo_in) {

    // This formula abuses Michael Byrne's code from CraftySkill.
    // He used  it to calculate max nodes per elo. By  dividing,
    // I derive speed that yields similar result in standard blitz.
    // Formula has a little bit of built-in randomness.

    const int lower_elo = elo_in - 25;
    const int upper_elo = elo_in + 25;
    int use_rating = rand() % (upper_elo - lower_elo + 1) + lower_elo;
    int search_nodes = (int)(pow(1.0069555500567, (((use_rating) / 1200) - 1)
                             + (use_rating - 1200)) * 128);

    return (search_nodes / 7) + (elo_in / 60);
}

int cParam::EloToBlur(int elo_in) {

    // Weaker levels get their evaluation blurred

    if (elo_in < 1500) return (1500 - elo_in) / 3;
    return 0;
}

int cParam::SpeedToBookDepth(int nps) {

	if (nps == 0 || nps > 100000) return 256;

	return (int) (nps * 256) / 100000;
}

void cDistance::Init() {

    static const int diagToUpperLeft[64] = {
         0,  1,  2,  3,  4,  5,  6,  7,
         1,  2,  3,  4,  5,  6,  7,  8,
         2,  3,  4,  5,  6,  7,  8,  9,
         3,  4,  5,  6,  7,  8,  9, 10,
         4,  5,  6,  7,  8,  9, 10, 11,
         5,  6,  7,  8,  9, 10, 11, 12,
         6,  7,  8,  9, 10, 11, 12, 13,
         7,  8,  9, 10, 11, 12, 13, 14
    };

    static const int diagToUpperRight[64] = {
         7,  6,  5,  4,  3,  2,  1,  0,
         8,  7,  6,  5,  4,  3,  2,  1,
         9,  8,  7,  6,  5,  4,  3,  2,
        10,  9,  8,  7,  6,  5,  4,  3,
        11, 10,  9,  8,  7,  6,  5,  4,
        12, 11, 10,  9,  8,  7,  6,  5,
        13, 12, 11, 10,  9,  8,  7,  6,
        14, 13, 12, 11, 10,  9,  8,  7
    };

    int nBonusMg[15] = { 14, 22, 29, 28, 19, -1, -6, -10, -11, -12, -13, -14, -15, -16, -17 };
    int bBonusMg[15] = { 6, -12, 4, -16, -11, -17, -6, -14, -9, -17, -1, -17, -4, 3, 7 };
    int rBonusMg[15] = { 7, 22, 23, 22, 22, 16, -2, -5, -14, -10, -8, -15, -16, -17, -17 };
    int qBonusMg[15] = { 35, 49, 47, 44, 40, 14, 3, 0, -2, 1, 4, -3, -5, -6, -9 };

    // Init distance tables

    for (int sq1 = 0; sq1 < 64; ++sq1) {
        for (int sq2 = 0; sq2 < 64; ++sq2) {
            int rankDelta = Abs(Rank(sq1) - Rank(sq2));
            int fileDelta = Abs(File(sq1) - File(sq2));
            grid[sq1][sq2] = rankDelta + fileDelta;
            bonus[sq1][sq2] = 14 - (rankDelta + fileDelta);  // for Fruit-like king tropism evaluation
            metric[sq1][sq2] = Max(rankDelta, fileDelta);    // chebyshev distance for unstoppable passers

            // Init per-piece distance bonuses (Hakapeliitta formula)

            queenTropism[sq1][sq2] = qBonusMg[grid[sq1][sq2]] + 7 * bonus[sq1][sq2];
            rookTropism[sq1][sq2] = rBonusMg[grid[sq1][sq2]];
            knightTropism[sq1][sq2] = nBonusMg[grid[sq1][sq2]];
            bishopTropism[sq1][sq2] = bBonusMg[Abs(diagToUpperRight[sq1] - diagToUpperRight[sq2])];
            bishopTropism[sq1][sq2] += bBonusMg[Abs(diagToUpperLeft[sq1] - diagToUpperLeft[sq2])];
        }
    }
}

void cParam::SetVal(int slot, int val, int min, int max, bool tune) {

    values[slot] = val;
    min_val[slot] = min;
    max_val[slot] = max;
    tunable[slot] = tune;
    if (val < min || val > max)
        printf("%14s ERROR\n", paramNames[slot]);
}

void cParam::PrintValues(int startTune, int endTune) {

    int iter = 0;

    printf("Values \n\n");
    for (int i = startTune; i < endTune; ++i) {
        if (tunable[i] == true) {
            printf("%14s : %4d     ", paramNames[i], Par.values[i]);
            iter++;
            if (iter % 4 == 0) printf("\n");
        }
    }
    printf("\n\n");
}