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
#include <cstring>
#include <cmath> // round

//#define DEBUG_EVAL_HASH

int cEngine::Evaluate(POS *p, eData *e) {

    // Try retrieving score from per-thread eval hashtable

    int addr = p->mHashKey % EVAL_HASH_SIZE;
#ifdef DEBUG_EVAL_HASH
    bool isHashEntry = false;
#endif
    int hashScore = 0;

    if (mEvalTT[addr].key == p->mHashKey) {
#ifdef DEBUG_EVAL_HASH
        isHashEntry = true;
#endif
        hashScore = mEvalTT[addr].score;
#ifndef DEBUG_EVAL_HASH
        return p->mSide == WC ? hashScore : -hashScore;
#endif
    }

    // Clear eval data

    e->Init(p);

    // Run all the evaluation subroutines

    EvaluateMaterial(p, e, WC);
    EvaluateMaterial(p, e, BC);
    EvaluatePieces(p, e, WC);
    EvaluatePieces(p, e, BC);  
    EvaluatePawnStruct(p, e);
    EvaluatePassers(p, e, WC);
    EvaluatePassers(p, e, BC);
    EvaluateUnstoppable(e, p);
    EvaluateThreats(p, e, WC);
    EvaluateThreats(p, e, BC);
    
    Add(e, p->mSide, 14, 7); // tempo bonus

    // Evaluate patterns

    EvaluateKnightPatterns(p, e);
    EvaluateBishopPatterns(p, e);
    EvaluateKingPatterns(p, e);
    EvaluateCentralPatterns(p, e);

    EvaluateKingAttack(p, e, WC);
    EvaluateKingAttack(p, e, BC);

    // Add pawn score (which might come from hash)

    e->mg[WC] += e->mgPawns[WC];
    e->mg[BC] += e->mgPawns[BC];
    e->eg[WC] += e->egPawns[WC];
    e->eg[BC] += e->egPawns[BC];

    // Add asymmetric bonus for keeping certain type of pieces

    e->mg[Par.programSide] += Par.keep[Q] * p->mCnt[Par.programSide][Q];
    e->mg[Par.programSide] += Par.keep[R] * p->mCnt[Par.programSide][R];
    e->mg[Par.programSide] += Par.keep[B] * p->mCnt[Par.programSide][B];
    e->mg[Par.programSide] += Par.keep[N] * p->mCnt[Par.programSide][N];
    e->mg[Par.programSide] += Par.keep[P] * p->mCnt[Par.programSide][P];

    // Bishop pins

    EvalPin(p, e, WC, D8, E7, F6, G5, H4);
    EvalPin(p, e, WC, E8, D7, C6, B5, A4);
    EvalPin(p, e, BC, D1, E2, F3, G4, H5);
    EvalPin(p, e, BC, E1, D2, C3, B4, A5);

    // Interpolate between midgame and endgame scores

    int score = Interpolate(p, e);

    // Piece/square table magic interpolation

    int mgPh = Min(p->mPhase, 24);
    int egPh = 24 - mgPh;

    int mgHypothesis = e->mgPrimaryPst[WC] - e->mgPrimaryPst[BC];
    int egHypothesis = e->egPrimaryPst[WC] - e->egPrimaryPst[BC];
    int primaryHypothesis = ((mgHypothesis * mgPh) + (egHypothesis * egPh)) / 24;

    mgHypothesis = e->mgSecondaryPst[WC] - e->mgSecondaryPst[BC];
    egHypothesis = e->egSecondaryPst[WC] - e->egSecondaryPst[BC];
    int secondaryHypothesis = ((mgHypothesis * mgPh) + (egHypothesis * egPh)) / 24;

    int scoreDelta = (50 * (primaryHypothesis - secondaryHypothesis)) / 100;
    int shift = sqrt(std::abs(scoreDelta));
    shift = Min(shift, 40);

    int primaryWeight = V(W_PRIM);
    int secondaryWeight = V(W_SECO);

    if (scoreDelta > 0) {
        primaryWeight += shift;
        secondaryWeight -= shift;
    }
    if (scoreDelta < 0) {
        primaryWeight -= shift;
        secondaryWeight += shift;
    }

    score += Percent(primaryHypothesis, primaryWeight);
    score += Percent(secondaryHypothesis, secondaryWeight);

    // Mobility rebalancing

    if (Par.useMobilityRebalancing) {

        mgHypothesis = e->mgDynMob[WC] - e->mgDynMob[BC];
        egHypothesis = e->egDynMob[WC] - e->egDynMob[BC];
        int dynHypothesis = ((mgHypothesis * mgPh) + (egHypothesis * egPh)) / 24;

        mgHypothesis = e->mgFlatMob[WC] - e->mgFlatMob[BC];
        egHypothesis = e->egFlatMob[WC] - e->egFlatMob[BC];
        int flatHypothesis = ((mgHypothesis * mgPh) + (egHypothesis * egPh)) / 24;

        scoreDelta = (50 * (dynHypothesis - flatHypothesis)) / 100;
        shift = sqrt(std::abs(scoreDelta));
        shift = Min(shift, 40);

        int primaryWhiteWeight = Par.sideMobility[WC];
        int primaryBlackWeight = Par.sideMobility[BC];
        secondaryWeight = V(W_FLAT);

        if (scoreDelta > 0) {
            primaryWhiteWeight += shift;
            primaryBlackWeight += shift;
            secondaryWeight -= shift;
        }
        if (scoreDelta < 0) {
            primaryWhiteWeight -= shift;
            primaryBlackWeight -= shift;
            secondaryWeight += shift;
        }

        mgHypothesis = (e->mgDynMob[WC] * primaryWhiteWeight) - (e->mgDynMob[BC] * primaryBlackWeight);
        egHypothesis = (e->egDynMob[WC] * primaryWhiteWeight) - (e->egDynMob[BC] * primaryBlackWeight);
        mgHypothesis /= 100;
        egHypothesis /= 100;

        dynHypothesis = ((mgHypothesis * mgPh) + (egHypothesis * egPh)) / 24;

        flatHypothesis *= secondaryWeight;
        flatHypothesis /= 100;

        score += flatHypothesis;
        score += dynHypothesis;

    }

    // Exchange imbalance evaluation

    int minorBalance = p->MinorCount(WC) - p->MinorCount(BC);
    int majorBalance = p->mCnt[WC][R] - p->mCnt[BC][R] + 2 * p->mCnt[WC][Q] - 2 * p->mCnt[BC][Q];

    if (minorBalance == -1 && majorBalance == 1)
        score += V(A_EXC);

    if (minorBalance == 1 && majorBalance == -1)
        score -= V(A_EXC);

    // Weakening: add pseudo-random value to eval score

    if (Par.evalBlur) {
        int randomMod = (Par.evalBlur / 2) - ((p->mHashKey ^ Glob.game_key) % Par.evalBlur);
        score += randomMod;
    }

    // Special case code for KBN vs K checkmate

    score += CheckmateHelper(p);

    // Decrease score for drawish endgames

    int draw_factor = 64;
    if (score > 0) draw_factor = GetDrawFactor(p, WC);
    if (score < 0) draw_factor = GetDrawFactor(p, BC);
    score = (score * draw_factor) / 64;

    // Ensure that returned value doesn't exceed mate score

    score = Clip(score, MAX_EVAL);

    // Save eval score in the evaluation hash table

#ifdef DEBUG_EVAL_HASH
    if (isHashEntry && hashScore != score)
        printf("x");
#endif

    mEvalTT[addr].key = p->mHashKey;
    mEvalTT[addr].score = score;

    // Return score relative to the side to move

    return p->mSide == WC ? score : -score;
}


void cEngine::ClearAll() {

    ClearPawnHash();
    ClearEvalHash();
    ClearHist();
}

void cEngine::ClearEvalHash() {

    ZEROARRAY(mEvalTT);
}

void cEngine::EvaluateMaterial(POS *p, eData *e, eColor sd) {

    eColor op = ~sd;

    int tmp = Par.np_table[p->mCnt[sd][P]] * p->mCnt[sd][N]    // knights lose value as pawns disappear
            - Par.rp_table[p->mCnt[sd][P]] * p->mCnt[sd][R];   // rooks gain value as pawns disappear

    if (p->mCnt[sd][N] > 1) tmp += V(N_PAIR);                  // knight pair
    if (p->mCnt[sd][R] > 1) tmp += V(R_PAIR);                  // rook pair
    if (p->mCnt[sd][B] > 1) Add(e, sd, V(B_PAIR_MG), V(B_PAIR_EG));  // bishop pair

    // "elephantiasis correction" for queen, idea by H.G.Mueller (nb. rookVsQueen doesn't help)

    if (p->mCnt[sd][Q])
        tmp -= V(ELEPH) * (p->mCnt[op][N] + p->mCnt[op][B]);

    Add(e, sd, tmp);
}

void cEngine::EvaluatePieces(POS *p, eData *e, eColor sd) {

    U64 pieces, occ, attack, control, possibleOutpost, contact, file;
    U64 b; // bitboard of evaluated square
    int sq, cnt, ownPawnCount, oppPawnCount;
    int rooksOn7th = 0;
    int mobMg = 0;
    int mobEg = 0;
    int flatMobMg = 0;
    int flatMobEg = 0;
    int tropism = 0;
    int linesMg = 0;
    int linesEg = 0;
    int outpostMg = 0;
	int outpostEg = 0;

    // Init king attack zone

    eColor op = ~sd;
    int kingSq = p->KingSq(op);
    U64 kingZone = BB.KingAttacks(kRoot[kingSq]);

	// Factor in minor pieces as king defenders
    
	int defenders[5] = { -5, 0, 3, 6, 9 };
	U64 minors = p->Knights(~sd) | p->Bishops(~sd);
	cnt = Min(PopCnt(minors & kingZone), 4);
	Add(e, ~sd, defenders[cnt], 0);

    // Init helper bitboards

    U64 nChecks = BB.KnightAttacks(kingSq) & ~p->mClBb[sd] & ~e->pawnTakes[op];
    U64 bChecks = BB.BishAttacks(p->Filled(), kingSq) & ~p->mClBb[sd] & ~e->pawnTakes[op];
    U64 rChecks = BB.RookAttacks(p->Filled(), kingSq) & ~p->mClBb[sd] & ~e->pawnTakes[op];
    U64 qChecks = rChecks & bChecks;

    // Squares excluded from mobility calculations

    U64 excluded = p->Pawns(sd) | e->pawnTakes[op];

    // Knight eval

    pieces = p->Knights(sd);
    while (pieces) {
        sq = PopFirstBit(&pieces);                         // get square
        AddPst(e, sd, N, (eSquare)sq);

        // knight tropism to enemy king (based on Hakapeliitta)

        tropism += Dist.knightTropism[sq][kingSq];

        control = BB.KnightAttacks(sq) & ~p->mClBb[sd];    // get control bitboard
        if (!(control & ~e->pawnTakes[op] & Mask.away[sd]))// we do not attack enemy half of the board
            Add(e, sd, V(N_OWH_MG), V(N_OWH_EG) );         // (surprisingly, seems beneficial for values like -1,0)
        e->allAttacks[sd] |= BB.KnightAttacks(sq);
        e->SetKnightAttacks(control, sd); // TODO: this or BB.KnightAttacks() ??
        if (control & nChecks) e->att[sd] += V(N_CHK);     // check threats

        possibleOutpost = control & ~e->pawnTakes[op];     // reachable outposts
        possibleOutpost &= ~e->pawnCanTake[op];
        possibleOutpost &= Mask.outpostMap[sd];
        if (possibleOutpost) {
            Add(e, sd, V(N_REACH_MG), V(N_REACH_EG));
        }

        attack = BB.KnightAttacks(sd) & kingZone;
        if (attack) {                                      // king attack
            e->att[sd] += V(N_ATT1) * PopCnt(attack & ~e->pawnTakes[op]);
            e->att[sd] += V(N_ATT2) * PopCnt(attack & e->pawnTakes[op]);
        }

        cnt = PopCnt(control & ~e->pawnTakes[op]);         // get mobility count
        mobMg += n_mob_mg_decreasing[cnt];
        mobEg += n_mob_eg_decreasing[cnt];
        flatMobMg += n_flat_mg[cnt];
        flatMobEg += n_flat_eg[cnt];

		EvaluateShielded(p, e, sd, sq, V(N_SH_MG), V(N_SH_EG), &outpostMg, &outpostEg);   // knight shielded by a pawn
        EvaluateOutpost(p, e, sd, N, sq, &outpostMg, &outpostEg);    // outpost
    }

    // Bishop eval

    pieces = p->Bishops(sd);
    while (pieces) {
        sq = PopFirstBit(&pieces);                         // get square
        b = SqBb(sq);
        AddPst(e, sd, B, (eSquare)sq);

        // bishop tropism  to enemy king (based on Hakapeliitta)

        tropism += Dist.bishopTropism[sq][kingSq];

        control = BB.BishAttacks(p->Filled(), sq);         // get control bitboard                     
        e->SetBishopAttacks(control, sd);                  // update attack map
        if (!(control & Mask.away[sd]))
             Add(e, sd, V(B_OWH_MG), V(B_OWH_EG) );        // we do not attack enemy half of the board
		if (control & bChecks) {
			e->att[sd] += V(B_CHK);                        // check threats
		}

        occ = p->Filled() ^ p->Queens(sd);
        attack = BB.BishAttacks(occ, sq) & kingZone;       // get king attack bitboard

        if (attack) {                                      // evaluate king attacks
            e->att[sd] += V(B_ATT1) * PopCnt(attack & ~e->pawnTakes[op]);
            e->att[sd] += V(B_ATT2) * PopCnt(attack & e->pawnTakes[op]);
        }

        cnt = PopCnt(control &~excluded);                  // get mobility count
        mobMg += b_mob_mg_decreasing[cnt];
        mobEg += b_mob_eg_decreasing[cnt];
        flatMobMg += b_flat_mg[cnt];
        flatMobEg += b_flat_eg[cnt];

        possibleOutpost = control & ~e->pawnTakes[op];     // reachable outposts
        possibleOutpost &= ~e->pawnCanTake[op];
        possibleOutpost &= Mask.outpostMap[sd];
        if (possibleOutpost) {
            Add(e, sd, V(B_REACH_MG), V(B_REACH_EG));
        }

		EvaluateShielded(p, e, sd, sq, V(B_SH_MG), V(B_SH_EG), &outpostMg, &outpostEg);  // bishop shielded by a pawn
        EvaluateOutpost(p, e, sd, B, sq, &outpostMg, &outpostEg);              // outpost

        // Bishops side by side

        if (ShiftNorth(SqBb(sq)) & p->Bishops(sd))
            Add(e, sd, 4);
        if (ShiftEast(SqBb(sq)) & p->Bishops(sd))
            Add(e, sd, 4);

        // Pawns on the same square color as our bishop;
        // central pawns are weighted higher

        if (bbWhiteSq & b) {
            ownPawnCount = PopCnt(bbWhiteSq & p->Pawns(sd)) - 4;
            ownPawnCount += PopCnt(bbWhiteSq & p->Pawns(sd) & Mask.center); // TODO: higher weight?
            oppPawnCount = PopCnt(bbWhiteSq & p->Pawns(op)) - 4;
        } else {
            ownPawnCount = PopCnt(bbBlackSq & p->Pawns(sd)) - 4;
            ownPawnCount += PopCnt(bbBlackSq & p->Pawns(sd) & Mask.center);
            oppPawnCount = PopCnt(bbBlackSq & p->Pawns(op)) - 4;
        }

        Add(e, sd, V(B_OWN_P) * ownPawnCount
                 + V(B_OPP_P) * oppPawnCount);
    }

    // Rook eval

    pieces = p->Rooks(sd);
    while (pieces) {
        sq = PopFirstBit(&pieces);                         // get square
        b = SqBb(sq);                                      // set square bitboard
        AddPst(e, sd, R, (eSquare)sq);

        // rook tropism to enemy king (based on Hakkapeliitta)

        tropism += Dist.rookTropism[sq][kingSq];

        control = BB.RookAttacks(p->Filled(), sq);         // get control bitboard                     
        e->SetRookAttacks(control, sd);                    // update attack map

        if (control & ~p->mClBb[sd] & rChecks) {                                
            e->att[sd] += V(R_CHK);                        // check threat bonus
        }

        occ = p->Filled() ^ p->StraightMovers(sd);
        attack = BB.RookAttacks(occ, sq) & kingZone;       // get king attack bitboard

        if (attack) {                                      // evaluate king attacks
            e->att[sd] += V(R_ATT1) * PopCnt(attack & ~e->pawnTakes[op]);
            e->att[sd] += V(R_ATT2) * PopCnt(attack & e->pawnTakes[op]);
        }

        cnt = PopCnt(control & ~excluded);                 // get mobility count
        mobMg += r_mob_mg_decreasing[cnt];
        mobEg += r_mob_eg_decreasing[cnt];
        flatMobMg += r_flat_mg[cnt];
        flatMobEg += r_flat_eg[cnt];

        // FILE EVALUATION:

        file = BB.FillNorth(b) | BB.FillSouth(b);          // get file

        if (file & p->Queens(op)) {                        // enemy queen on rook's file
            linesMg += V(ROQ_MG);
            linesEg += V(ROQ_EG);
        }

        if (!(file & p->Pawns(sd))) {                      // no own pawns on that file
            if (!(file & p->Pawns(op))) {                  // open file
                linesMg += V(ROF_MG);
                linesEg += V(ROF_EG);
            } else {                                            // half-open file...
                if (file & (p->Pawns(op) & e->pawnTakes[op])) { // ...with defended enemy pawn
                    linesMg += V(RBH_MG);
                    linesEg += V(RBH_EG);
                } else {                                   // ...with undefended enemy pawn
                    linesMg += V(RGH_MG);
                    linesEg += V(RGH_EG);
                }
            }
        }

        // Rook on the 7th rank attacking pawns or cutting off enemy king

        if (Mask.IsOnRank7(b,sd)) {                        // rook on 7th rank
            if (p->Pawns(op) & bbRelRank[sd][RANK_7]     // attacking enemy pawns
            ||  p->Kings(op) & bbRelRank[sd][RANK_8]) {  // or cutting off enemy king
                linesMg += V(RSR_MG);
                linesEg += V(RSR_EG);
                rooksOn7th++;
            }
        }
    }

    // Queen eval

    pieces = p->Queens(sd);
    while (pieces) {
        sq = PopFirstBit(&pieces);                         // get square
        b = SqBb(sq);                                      // set square bitboard
        AddPst(e, sd, Q, (eSquare)sq);

        // queen tropism to enemy king (based on Hakapeliitta)

        tropism += Dist.queenTropism[sq][kingSq];

        control = BB.QueenAttacks(p->Filled(), sq);        // get control bitboard
        e->SetQueenAttacks(control, sd);                   // update attack map
        
        if (control & qChecks) {                           // check threat bonus
            e->att[sd] += V(Q_CHK);

            contact = control & BB.KingAttacks(kingSq);    // queen contact checks
            while (contact) {
                int contactSq = PopFirstBit(&contact);     // find potential contact check square
                if (p->Swap(sq, contactSq) >= 0) {         // if check doesn't lose material, evaluate
                    e->att[sd] += V(Q_CONTACT);
                    break;
                }
            }
        }

        attack  = BB.BishAttacks(p->Filled() ^ p->DiagMovers(sd), sq);
        attack |= BB.RookAttacks(p->Filled() ^ p->StraightMovers(sd), sq);
        attack &= kingZone;

        if (attack) {                                      // evaluate king attacks
            e->att[sd] += V(Q_ATT1) * PopCnt(attack & ~e->pawnTakes[op]);
            e->att[sd] += V(Q_ATT2) * PopCnt(attack &  e->pawnTakes[op]);
        }

        cnt = PopCnt(control & ~excluded);                 // get mobility count
        mobMg += q_mob_mg_decreasing[cnt];
        mobEg += q_mob_eg_decreasing[cnt];
        flatMobMg += q_flat_mg[cnt];
        flatMobEg += q_flat_eg[cnt];

        if (Mask.IsOnRank7(b,sd)) {                        // queen on 7th rank
            if (p->Pawns(op) & bbRelRank[sd][RANK_7]     // attacking enemy pawns
            ||  p->Kings(op) & bbRelRank[sd][RANK_8]) {  // or cutting off enemy king
                linesMg += V(QSR_MG);
                linesEg += V(QSR_EG);
            }
        }

    } // end of queen eval

    // Composite factors

    if (rooksOn7th > 1) {  // two rooks on 7th rank
        linesMg += V(RS2_MG);
        linesEg += V(RS2_EG);
    }

    // Weighting eval parameters

    if (Par.useMobilityRebalancing) {
        e->mgDynMob[sd] = mobMg;
        e->egDynMob[sd] = mobEg;
        e->mgFlatMob[sd] = flatMobMg;
        e->egFlatMob[sd] = flatMobEg;
    } else {
        Add(e, sd, Percent(Par.sideMobility[sd],mobMg), Percent(Par.sideMobility[sd],mobEg));
        Add(e, sd, Percent(V(W_FLAT), flatMobMg), Percent(V(W_FLAT), flatMobEg));
    }

    Add(e, sd, Percent(V(W_TROPISM),tropism), 0);
    Add(e, sd, Percent(V(W_LINES), linesMg), Percent(V(W_LINES), linesEg));
    Add(e, sd, Percent(V(W_OUTPOSTS), outpostMg), Percent(V(W_OUTPOSTS), outpostEg));
}

void cEngine::EvaluateKingAttack(POS *p, eData *e, eColor sd) {

    eColor op = eColor(sd ^ 1);
    eSquare sq = (eSquare) p->KingSq(sd);
    AddPst(e, sd, K, sq);

    // attack on enemy king - attacked and undefended squares in king zone

    U64 attacks  = e->pawnTakes[sd] | e->GetNbrqAttacks(sd);
    U64 defences = e->pawnTakes[op] | e->GetNbrqAttacks(op);
    int kingSq = p->KingSq(op);
    U64 kingZone = BB.KingAttacks(kRoot[kingSq]);
    attacks &= kingZone;
    e->att[sd] += 5 * PopCnt(attacks & ~defences);

    // attack on enemy king - final calculation

    if (e->att[sd] > 399) e->att[sd] = 399;
    if (p->mCnt[sd][Q] == 0) e->att[sd] = 0;
    Add(e, sd, Percent(Par.danger[e->att[sd]], Par.sideAttack[sd]));
}

void cEngine::EvalPin(POS * p, eData * e, eColor c, eSquare pinned0, eSquare pinned1, eSquare knight, eSquare bish0, eSquare bish1) {

    if (p->IsOnSq(~c, N, knight)) {
        if (p->IsOnSq(c, B, bish0)
            || ((p->Empty() & (SqBb(bish0))) && p->IsOnSq(c, B, bish1))) {
            if (p->IsOnSq(~c, K, pinned1)
            || p->IsOnSq(~c, Q, pinned1))
                Add(e, c, 10, 5);
            else if (p->Empty() & SqBb(pinned1)) {
                if (p->IsOnSq(~c, K, pinned0)
                || p->IsOnSq(~c, Q, pinned0))
                    Add(e, c, 10, 5);
            }
        }
    }
}

void cEngine::EvaluateShielded(POS *p, eData *e, eColor sd, int sq, int v1, int v2, int *outpost_mg, int *outpost_eg) {

    if (SqBb(sq) & Mask.home[sd]) {
        U64 stop = BB.ShiftFwd(SqBb(sq), sd);             // get square in front of a minor
		if (stop & (p->Pawns(sd) | p->Pawns(~sd))) {      // is it occupied by a pawn?
			*outpost_mg += v1;                            // add bonus for a pawn shielding a minor
			*outpost_eg += v2;
		}
    }
}

void cEngine::EvaluateOutpost(POS *p, eData *e, eColor sd, int pc, int sq, int *outpost_mg, int *outpost_eg) {

    int tmp = Par.sp_pst[sd][pc][sq];                      // get base outpost bonus
	int dst = Dist.metric[sd][p->KingSq(~sd)];             // factor in distance to enemy king
	if (dst > 0) tmp += dst / 2;

    if (tmp) {
        U64 b = SqBb(sq);
        int mul = 0;                                       // reset outpost multiplier
        if (b & ~e->pawnCanTake[~sd]) mul += 2;            // is piece in hole of enemy pawn structure?
        if (b & e->pawnTakes[sd]) mul += 1;                // is piece defended by own pawn?
        if (b & e->twoPawnsTake[sd]) mul += 1;             // is piece defended by two pawns?
        *outpost_mg += (tmp * mul) / 2;                    // add outpost bonus
		*outpost_eg += (tmp * mul) / 2;
    }
}

void cEngine::EvaluatePawns(POS *p, eData *e, eColor sd) {

    U64 pawns, frontSpan, isPhalanx, isDefended;
    int sq, isUnopposed;
    eColor op = ~sd;
    int massMg = 0;
    int massEg = 0;

    pawns = p->Pawns(sd);
    while (pawns) {

        // Get square

        sq = PopFirstBit(&pawns);

        // Set data and flags

        frontSpan = BB.GetFrontSpan(SqBb(sq), sd);
        isUnopposed = ((frontSpan & p->Pawns(op)) == 0);
        isPhalanx = (ShiftSideways(SqBb(sq)) & p->Pawns(sd));
        isDefended = (SqBb(sq) & e->pawnTakes[sd]);

        // Candidate passers

        if (isUnopposed) {
            if (isPhalanx || isDefended) {
                if (PopCnt((Mask.passed[sd][sq] & p->Pawns(op))) == 1)
                    AddPawns(e, sd, Par.cand_bonus_mg[sd][Rank(sq)], Par.cand_bonus_eg[sd][Rank(sq)]);
            }
        }

        // Doubled pawn

        if (frontSpan & p->Pawns(sd))
            AddPawns(e, sd, V(DB_MID), V(DB_END));

        // Supported pawn

        if (isPhalanx) {
            massMg += Par.sp_pst[sd][PHA_MG][sq];
            massEg += Par.sp_pst[sd][PHA_EG][sq];
        } else if (isDefended) {
            massMg += Par.sp_pst[sd][DEF_MG][sq];
            massEg += Par.sp_pst[sd][DEF_EG][sq];
        }

        // Isolated and weak pawn

        if (!(Mask.adjacent[File(sq)] & p->Pawns(sd)))
            AddPawns(e, sd, V(ISO_MG) + V(ISO_OF) * isUnopposed, V(ISO_EG));
        else if (!(Mask.supported[sd][sq] & p->Pawns(sd)))
            AddPawns(e, sd, Par.backward_malus_mg[File(sq)] +V(BK_OPE) * isUnopposed, V(BK_END));
    }

    AddPawns(e, sd, Percent(massMg,V(W_MASS)), Percent(massEg,V(W_MASS)) );
}

void cEngine::EvaluatePassers(POS *p, eData *e, eColor sd) {

    U64 pieces, b, bb_stop;
    int sq, mgTemp, egTemp, mul;
    eColor op = ~sd;
    int mgTotal = 0;
    int egTotal = 0;

    pieces = p->Pawns(sd);
    while (pieces) {
        sq = PopFirstBit(&pieces);
        AddPst(e, sd, P, (eSquare)sq);
        b = SqBb(sq);
        bb_stop = BB.ShiftFwd(b, sd);

        // pawn mobility

        if (!(bb_stop & p->Filled())) {
			Add(e, sd, V(P_MOB_MG), V(P_MOB_EG));          // pawn mobility bonus
			if (b & Mask.center) Add(e, sd, 2, 0);         // additional central pawn mobility bonus

			// TODO: pawn can attack enemy piece in one move
        }

        // passed pawns

        if (!(Mask.passed[sd][sq] & p->Pawns(op))) {
            mul = 100;

            if (b & e->pawnTakes[sd]) mul += V(P_DEFMUL);
            if (bb_stop & e->pawnTakes[sd]) mul += V(P_STOPMUL);

            if (bb_stop & p->Filled()) mul -= V(P_BL_MUL); // blocked passers score less

            else if ((bb_stop & e->allAttacks[sd])         // our control of stop square
                 && (bb_stop & ~e->allAttacks[op])) mul += V(P_OURSTOP_MUL);
            
            else if ((bb_stop & e->allAttacks[op])         // opp control of stop square
                 && (bb_stop & ~e->allAttacks[sd])) mul -= V(P_OPPSTOP_MUL);

            // In the midgame, we use just a bonus from the table;
            // in the endgame, passed pawn attracts both kings.

            mgTemp = Par.passed_bonus_mg[sd][Rank(sq)];
            egTemp = Par.passed_bonus_eg[sd][Rank(sq)]
                   - ((Par.passed_bonus_eg[sd][Rank(sq)] * Dist.bonus[sq][p->mKingSq[op]]) / 30)
                   + ((Par.passed_bonus_eg[sd][Rank(sq)] * Dist.bonus[sq][p->mKingSq[sd]]) / 90);

            mgTotal += Percent(mgTemp, mul);
            egTotal += Percent(egTemp, mul);

            // Special case pattern - pawn on 7th rank
            // that has to be blocked by a major piece
            // restricts it badly, and gets a big bonus.

            if (Mask.IsOnRank7(b, sd)) {
                if (bb_stop & (p->Rooks(op) | p->Queens(op)))
                    Add(e, sd, 0, 100);
            }
        }
    }

    Add(e, sd, Percent(mgTotal, V(W_PASSERS)), Percent(egTotal, V(W_PASSERS)) );
}

void cEngine::EvaluateUnstoppable(eData *e, POS *p) {

    U64 bb_pieces, bb_span;
    int w_dist = 8;
    int b_dist = 8;
    int sq, king_sq, pawn_sq, tempo, prom_dist;

    // White unstoppable passers

    if (p->PieceCount(BC) == 0) {
        king_sq = p->KingSq(BC);
        if (p->mSide == BC) tempo = 1; else tempo = 0;
        bb_pieces = p->Pawns(WC);
        while (bb_pieces) {
            sq = PopFirstBit(&bb_pieces);
            if (!(Mask.passed[WC][sq] & p->Pawns(BC))) {
                bb_span = BB.GetFrontSpan(SqBb(sq), WC);
                pawn_sq = ((WC - 1) & 56) + (sq & 7);
                prom_dist = Min(5, Dist.metric[sq][pawn_sq]);

                if (prom_dist < (Dist.metric[king_sq][pawn_sq] - tempo)) {
                    if (bb_span & p->Kings(WC)) prom_dist++;
                    w_dist = Min(w_dist, prom_dist);
                }
            }
        }
    }

    // Black unstoppable passers

    if (p->PieceCount(WC) == 0) {
        king_sq = p->KingSq(WC);
        if (p->mSide == WC) tempo = 1; else tempo = 0;
        bb_pieces = p->Pawns(BC);
        while (bb_pieces) {
            sq = PopFirstBit(&bb_pieces);
            if (!(Mask.passed[BC][sq] & p->Pawns(WC))) {
                bb_span = BB.GetFrontSpan(SqBb(sq), BC);
                pawn_sq = ((BC - 1) & 56) + (sq & 7);
                prom_dist = Min(5, Dist.metric[sq][pawn_sq]);

                if (prom_dist < (Dist.metric[king_sq][pawn_sq] - tempo)) {
                    if (bb_span & p->Kings(BC)) prom_dist++;
                    b_dist = Min(b_dist, prom_dist);
                }
            }
        }
    }

    if (w_dist < b_dist - 1) Add(e, WC, 0, 500);
    if (b_dist < w_dist - 1) Add(e, BC, 0, 500);
}

void cEngine::AddPst(eData *e, eColor c, int type, eSquare s) {

    e->mgPrimaryPst[c] += Par.mgPrimaryPstData[c][type][s];
    e->egPrimaryPst[c] += Par.egPrimaryPstData[c][type][s];
    e->mgSecondaryPst[c] += Par.mgSecondaryPstData[c][type][s];
    e->egSecondaryPst[c] += Par.egSecondaryPstData[c][type][s];
}

void cEngine::Add(eData *e, eColor sd, int mg_val, int eg_val) {

    e->mg[sd] += mg_val;
    e->eg[sd] += eg_val;
}

void cEngine::Add(eData *e, eColor sd, int val) {

    e->mg[sd] += val;
    e->eg[sd] += val;
}

void cEngine::AddPawns(eData *e, eColor sd, int mg_val, int eg_val) {

    e->mgPawns[sd] += mg_val;
    e->egPawns[sd] += eg_val;
}

int cEngine::Interpolate(POS *p, eData *e) {

    int mg_tot = e->mg[WC] - e->mg[BC];
    int eg_tot = e->eg[WC] - e->eg[BC];
    int mg_phase = Min(p->mPhase, 24);
    int eg_phase = 24 - mg_phase;

    return (mg_tot * mg_phase + eg_tot * eg_phase) / 24;
}

void cEngine::EvaluateThreats(POS *p, eData *e, eColor sd) {

    int pc, sq;
    int mg = 0;
    int eg = 0;
    eColor op = ~sd;

    U64 undefended = p->mClBb[op];
    U64 threatened = undefended & e->pawnTakes[sd];
    U64 defended = undefended & e->allAttacks[op];
    U64 hanging = undefended & ~e->pawnTakes[op];

    undefended &= ~e->allAttacks[sd];
    undefended &= ~e->allAttacks[op];

    hanging |= threatened;            // piece attacked by our pawn isn't well defended
    hanging &= e->allAttacks[sd];     // hanging piece has to be attacked

    defended &= e->GetNbrAttacks(sd); // N, B, R attacks (pieces attacked by pawns are scored as hanging)
    defended &= ~e->pawnTakes[sd];    // no defense against pawn attack

	const int attOnHangingMg[7]  = {  0, 15, 15, 17, 25,  0,  0 };
	const int attOnHangingEg[7]  = {  0, 23, 23, 25, 33,  0,  0 };
	const int attOnDefendedMg[7] = {  0,  8,  8, 10, 15,  0,  0 };
	const int attOnDefendedEg[7] = {  0, 12, 12, 14, 19,  0,  0 };
	const int floatingPieceMg[7] = {  0,  5,  5,  5,  5,  0,  0 };
	const int floatingPieceEg[7] = {  0,  9,  9,  9,  9,  0,  0 };

    // hanging pieces (attacked and undefended, based on DiscoCheck)

    while (hanging) {
        sq = PopFirstBit(&hanging);
        pc = p->TpOnSq(sq);
        mg += attOnHangingMg[pc];
        eg += attOnHangingEg[pc];
    }

    // defended pieces under attack

    while (defended) {
        sq = PopFirstBit(&defended);
        pc = p->TpOnSq(sq);
        mg += attOnDefendedMg[pc];
        eg += attOnDefendedEg[pc];
    }

    // unattacked and undefended

    while (undefended) {
		sq = PopFirstBit(&undefended);
		pc = p->TpOnSq(sq);
		mg += floatingPieceMg[pc];
		eg += floatingPieceEg[pc];
    }

    Add(e, sd, Percent(V(W_THREATS), mg), Percent(V(W_THREATS), eg));

    // space evaluation - takes into account two factors:
    // control of central squares by one player only
    // and control of squares behind own pawn chain.
    // It is scaled by the number of minor pieces, so
    // much more important in the opening. The idea comes
    // from Stockfish.

    U64 behind;

    if (sd == WC)
        behind = BB.FillSouth(p->Pawns(WC));
    else
        behind = BB.FillNorth(p->Pawns(BC));

    int minors = p->mCnt[sd][N] + p->mCnt[sd][B];
    U64 space = Mask.space[sd] & e->allAttacks[sd];
    space |= ~e->allAttacks[op];

    int spaceBonus = minors * PopCnt(space) / 2;

    space |= behind;
    spaceBonus += minors * PopCnt(space);
    Add(e, sd, Percent(spaceBonus, V(W_SPACE)), 0);

}
