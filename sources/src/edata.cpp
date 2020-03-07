#include "rodent.h"

void eData::Init(POS * p) {

    for (eColor sd = WC; sd < 2; ++sd) {

        mgPrimaryPst[sd] = 0;
        mgSecondaryPst[sd] = 0;
        egPrimaryPst[sd] = 0;
        egSecondaryPst[sd] = 0;
        mg[sd] = p->mMgScore[sd];
        eg[sd] = p->mEgScore[sd];
        att[sd] = 0;
        nbrAttacks[sd] = 0ULL;
        qAttacks[sd] = 0ULL;
    }

    // Init helper bitboards (pawn info)

    pawnTakes[WC] = GetWPControl(p->Pawns(WC));
    pawnTakes[BC] = GetBPControl(p->Pawns(BC));
    pawnCanTake[WC] = BB.FillNorth(pawnTakes[WC]);
    pawnCanTake[BC] = BB.FillSouth(pawnTakes[BC]);
    twoPawnsTake[WC] = GetDoubleWPControl(p->Pawns(WC));
    twoPawnsTake[BC] = GetDoubleBPControl(p->Pawns(BC));

    // Partial initialization of attack maps

    allAttacks[WC] = pawnTakes[WC] | BB.KingAttacks(p->KingSq(WC));
    allAttacks[BC] = pawnTakes[BC] | BB.KingAttacks(p->KingSq(BC));
}

void eData::SetKnightAttacks(U64 control, int sd) {
    nbrAttacks[sd] |= control;
}

void eData::SetBishopAttacks(U64 control, int sd) {
    nbrAttacks[sd] |= control;
    allAttacks[sd] |= control;
}

void eData::SetRookAttacks(U64 control, int sd) {
    nbrAttacks[sd] |= control;
    allAttacks[sd] |= control;
}

void eData::SetQueenAttacks(U64 control, int sd) {
    allAttacks[sd] |= control;
    qAttacks[sd] |= control;
}

U64 eData::GetNbrqAttacks(int sd) {
    return nbrAttacks[sd] | qAttacks[sd];
}

U64 eData::GetNbrAttacks(int sd) {
    return nbrAttacks[sd];
}