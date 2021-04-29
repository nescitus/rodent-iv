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

// REGEX to count all the lines under MSVC 13: ^(?([^\r\n])\s)*[^\s+?/]+[^\n]*$
// 9289 lines

// bench 14, 64-bit release: 11133006 nodes searched in 11203, speed 993663 nps (Score: 2.306)
// bench 14, 64-bit debug  : ??

// bench 14, 32-bit release: ??
// bench 14, 32-bit debug  : ??

// TODO: pawns participation in king attack
// TODO: auxiliary pawn piece/square tables, depending on king's wing
// TODO: king attacks calculated when no queen present
// TODO: continuation move
// TODO: easy move code
// TODO: no book moves in analyze mode
// TODO: fix small bug: engine crashes on empty book file path or empty personality file path

#pragma once

#if (!defined(DEBUG) && !defined(NDEBUG)) || (defined(DEBUG) && defined(NDEBUG))
    #if defined(DEBUG)
        #undef DEBUG
    #endif
    #if defined(NDEBUG)
        #undef NDEBUG
    #endif

    // Unclear settings, so use this as default (each cpp-file should include rodent.h)
    #define NDEBUG

#endif

#if !(__cplusplus >= 201402L || _MSVC_LANG >= 201402L)
    #error Rodent requires C++14 compatible compiler.
#endif

#pragma warning( disable : 4577 )
#pragma warning( disable : 4530 )

// catching memory leaks using MS Visual Studio
// https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library
#if defined(_MSC_VER) && !defined(NDEBUG)
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
#endif

#include <cstdint>
#include <cinttypes>
#include <string>
#include "stringfunctions.h"

//#define USE_TUNING

using U64 = uint64_t;

// max size of hashtable

#ifdef _WIN64
static constexpr int max_tt_size_mb = 16384;
#else
static constexpr int max_tt_size_mb = 4096;
#endif

// define how Rodent is to be compiled

#define USE_MAGIC
#ifndef NO_MM_POPCNT
    #define USE_MM_POPCNT
#endif
#define USE_FIRST_ONE_INTRINSICS

// max size of an opening book to fully cache in memory (in MB)
#ifndef NO_BOOK_IN_MEMORY
    #define BOOK_IN_MEMORY_MB 16
#endif

//#define NO_THREADS

#ifndef NO_THREADS
    #include <thread>
    #ifndef USE_THREADS
       #define USE_THREADS
    #endif
    #define MAX_THREADS 48
#else
    #undef USE_THREADS
#endif

enum eColor      { WC, BC, NO_CL };
enum ePieceType  { P, N, B, R, Q, K, NO_TP };
enum ePiece      { WP, BP, WN, BN, WB, BBi, WR, BR, WQ, BQ, WK, BK, NO_PC };
enum eFile       { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };
enum eRank       { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };
enum eCastleFlag { W_KS = 1, W_QS = 2, B_KS = 4, B_QS = 8 };
enum eGUIs { Other, Arena, WinBoard };
enum eCastleNotation { KingMove, TakeRook, OOO };
enum eMoveType   { NORMAL, CASTLE, EP_CAP, EP_SET, N_PROM, B_PROM, R_PROM, Q_PROM };
enum eMoveFlag   { MV_NORMAL, MV_HASH, MV_CAPTURE, MV_REFUTATION, MV_KILLER, MV_BADCAPT };
enum eHashType   { NONE, UPPER, LOWER, EXACT };
enum eSquare {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQ
};

// Operators for eColor type:
// ~ switches color
// ++ (placed before variable) iterates

inline eColor operator~(eColor c) { return eColor(c ^ BC); }
inline eColor operator++(eColor& c) { return c = eColor(int(c) + 1); }

constexpr int PHA_MG = Q;
constexpr int DEF_MG = K;
constexpr int PHA_EG = P;
constexpr int DEF_EG = R;

constexpr int MAX_PLY   = 64;
constexpr int MAX_MOVES = 256;
constexpr int INF       = 32767;
constexpr int MATE      = 32000;
constexpr int MAX_EVAL  = 29999;
constexpr int MAX_HIST  = 1 << 15;
constexpr int MAX_PV    = 12;

constexpr U64 RANK_1_BB = 0x00000000000000FF;
constexpr U64 RANK_2_BB = 0x000000000000FF00;
constexpr U64 RANK_3_BB = 0x0000000000FF0000;
constexpr U64 RANK_4_BB = 0x00000000FF000000;
constexpr U64 RANK_5_BB = 0x000000FF00000000;
constexpr U64 RANK_6_BB = 0x0000FF0000000000;
constexpr U64 RANK_7_BB = 0x00FF000000000000;
constexpr U64 RANK_8_BB = 0xFF00000000000000;

constexpr U64 FILE_A_BB = 0x0101010101010101;
constexpr U64 FILE_B_BB = 0x0202020202020202;
constexpr U64 FILE_C_BB = 0x0404040404040404;
constexpr U64 FILE_D_BB = 0x0808080808080808;
constexpr U64 FILE_E_BB = 0x1010101010101010;
constexpr U64 FILE_F_BB = 0x2020202020202020;
constexpr U64 FILE_G_BB = 0x4040404040404040;
constexpr U64 FILE_H_BB = 0x8080808080808080;

constexpr U64 DIAG_A1H8_BB = 0x8040201008040201;
constexpr U64 DIAG_A8H1_BB = 0x0102040810204080;
constexpr U64 DIAG_B8H2_BB = 0x0204081020408000;

#define REL_SQ(sq,cl)   ( (sq) ^ ((cl) * 56) )
#define RelSqBb(sq,cl)  ( SqBb(REL_SQ(sq,cl) ) )

constexpr U64 bbWhiteSq = 0x55AA55AA55AA55AA;
constexpr U64 bbBlackSq = 0xAA55AA55AA55AA55;

constexpr U64 bbRelRank[2][8] = {
    { RANK_1_BB, RANK_2_BB, RANK_3_BB, RANK_4_BB, RANK_5_BB, RANK_6_BB, RANK_7_BB, RANK_8_BB },
    { RANK_8_BB, RANK_7_BB, RANK_6_BB, RANK_5_BB, RANK_4_BB, RANK_3_BB, RANK_2_BB, RANK_1_BB }
};

constexpr U64 bbCentralFile = FILE_C_BB | FILE_D_BB | FILE_E_BB | FILE_F_BB;

constexpr U64 SIDE_RANDOM = ~UINT64_C(0);

constexpr char START_POS[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";

template<typename T> constexpr T Abs(const T& x) { return x > 0 ? x : -x; }
template<typename T> constexpr const T& Max(const T& x, const T& y) { return x > y ? x : y; }
template<typename T> constexpr const T& Min(const T& x, const T& y) { return x < y ? x : y; }

template<typename T> constexpr T Clip(const T& sc, const T& lim) { if (sc < -lim) return -lim;
                                                                   if (sc > lim) return lim;
                                                                   return sc; }

#define SqBb(x)         (UINT64_C(1) << (x))

#define Cl(x)           ((x) & 1)
#define Tp(x)           ((x) >> 1)
#define Pc(x, y)        (((y) << 1) | (x))

#define File(x)         ((x) & 7)
#define Rank(x)         ((x) >> 3)
#define Sq(x, y)        (((y) << 3) | (x))

#define Fsq(x)          ((x) & 63)
#define Tsq(x)          (((x) >> 6) & 63)
#define MoveType(x)     ((x) >> 12)
#define IsProm(x)       ((x) & 0x4000)
#define PromType(x)     (((x) >> 12) - 3)

#ifndef FORCEINLINE
    #if defined(_MSC_VER)
        #define FORCEINLINE __forceinline
    #else
        #define FORCEINLINE __inline
    #endif
#endif

#ifndef NOINLINE
    #if defined(_MSC_VER)
        #define NOINLINE __declspec(noinline)
    #else
        #define NOINLINE __attribute__((noinline))
    #endif
#endif

// Compiler and architecture dependent versions of FirstOne() function,
// triggered by defines at the top of this file.
#ifdef USE_FIRST_ONE_INTRINSICS

    #if defined(_MSC_VER)

        #include <intrin.h>

        #ifndef _WIN64
            #pragma intrinsic(_BitScanForward)
        #else
            #pragma intrinsic(_BitScanForward64)
        #endif

        static int FORCEINLINE FirstOne(U64 x) {
            unsigned long index;
        #ifndef _WIN64
            if (_BitScanForward(&index, (unsigned long)x)) return index;
            _BitScanForward(&index, x >> 32); return index + 32;
        #else
            _BitScanForward64(&index, x);
            return index;
        #endif
        }

    #elif defined(__GNUC__)

        constexpr int FirstOne(const U64& x) {

        // workaround for GCC's inability to inline __builtin_ctzll() on x32 (it calls `__ctzdi2` runtime function instead)
        #if !defined(__amd64__) && defined(__i386__) && !defined(__clang__)
            const uint32_t xlo = (uint32_t)x;
            return xlo ? __builtin_ctz(xlo) : __builtin_ctz(x >> 32) + 32;
        #else
            return __builtin_ctzll(x);
        #endif
        }

    #endif

#else
    const int bit_table[64] = {
        0,  1,  2,  7,  3, 13,  8, 19,
        4, 25, 14, 28,  9, 34, 20, 40,
        5, 17, 26, 38, 15, 46, 29, 48,
       10, 31, 35, 54, 21, 50, 41, 57,
       63,  6, 12, 18, 24, 27, 33, 39,
       16, 37, 45, 47, 30, 53, 49, 56,
       62, 11, 23, 32, 36, 44, 52, 55,
       61, 22, 43, 51, 60, 42, 59, 58
    };
    #define FirstOne(x)     bit_table[(((x) & (~(x) + 1)) * (U64)0x0218A392CD3D5DBF) >> 58] // first "1" in a bitboard
#endif

constexpr U64 bbNotA = ~FILE_A_BB; // 0xfefefefefefefefe
constexpr U64 bbNotH = ~FILE_H_BB; // 0x7f7f7f7f7f7f7f7f

constexpr U64 ShiftNorth(const U64& b) { return b << 8; }
constexpr U64 ShiftSouth(const U64& b) { return b >> 8; }
constexpr U64 ShiftWest(const U64& b)  { return (b & bbNotA) >> 1; }
constexpr U64 ShiftEast(const U64& b)  { return (b & bbNotH) << 1; }
constexpr U64 ShiftNW(const U64& b)    { return (b & bbNotA) << 7; }
constexpr U64 ShiftNE(const U64& b)    { return (b & bbNotH) << 9; }
constexpr U64 ShiftSW(const U64& b)    { return (b & bbNotA) >> 9; }
constexpr U64 ShiftSE(const U64& b)    { return (b & bbNotH) >> 7; }

constexpr U64 ShiftSideways(const U64& b) { return (ShiftWest(b) | ShiftEast(b)); }
constexpr U64 GetWPControl(const U64& b)  { return (ShiftNE(b) | ShiftNW(b)); }
constexpr U64 GetBPControl(const U64& b)  { return (ShiftSE(b) | ShiftSW(b)); }
constexpr U64 GetDoubleWPControl(const U64& b) { return (ShiftNE(b) & ShiftNW(b)); }
constexpr U64 GetDoubleBPControl(const U64& b) { return (ShiftSE(b) & ShiftSW(b)); }

constexpr bool MoreThanOne(const U64& b) { return b & (b - 1); }

// taunt functions

enum eTaunt{TAUNT_CAPTURE, 
    TAUNT_CRUSHING, TAUNT_WINNING, TAUNT_ADVANTAGE, TAUNT_SMALL_PLUS, TAUNT_BALANCE, TAUNT_SMALL_MINUS,
    TAUNT_GENERIC, TAUNT_DISADVANTAGE, TAUNT_LOSING};
// TAUNT_INCHECK, TAUNT_CHECKMATE, TAUNT_BOOK

void PrintTaunt(int eventType);
void PrintGenericTaunt();
void PrintCaptureTaunt();
void PrintLosingTaunt();
void PrintDisdvantageTaunt();
void PrintBalanceTaunt();
void PrintAdvantageTaunt();
void PrintWinningTaunt();
void PrintCrushingTaunt();
void PrintUserBlunderTaunt();
void PrintEngineBlunderTaunt();
void PrintEngineEscapeTaunt();
void PrintGainingTaunt();

// bitboard functions

int PopCnt(U64);
int PopFirstBit(U64 *bb);

class cBitBoard {
  private:
    U64 p_attacks[2][64];
    U64 n_attacks[64];
    U64 k_attacks[64];

#ifndef USE_MAGIC
    U64 FillOcclSouth(U64 bb_start, U64 bb_block);
    U64 FillOcclNorth(U64 bb_start, U64 bb_block);
    U64 FillOcclEast(U64 bb_start, U64 bb_block);
    U64 FillOcclWest(U64 bb_start, U64 bb_block);
    U64 FillOcclNE(U64 bb_start, U64 bb_block);
    U64 FillOcclNW(U64 bb_start, U64 bb_block);
    U64 FillOcclSE(U64 bb_start, U64 bb_block);
    U64 FillOcclSW(U64 bb_start, U64 bb_block);
#endif

    U64 GetBetween(int sq1, int sq2);

  public:
    U64 bbBetween[64][64];
    void Init();
    void Print(U64 bb);
    U64 ShiftFwd(U64 bb, eColor sd);
    U64 GetPawnControl(U64 bb, eColor sd);
    U64 GetFrontSpan(U64 bb, eColor sd);
    U64 FillNorth(U64 bb);
    U64 FillSouth(U64 bb);
    U64 FillNorthExcl(U64 bb);
    U64 FillSouthExcl(U64 bb);

    U64 PawnAttacks(eColor sd, int sq);
    U64 KingAttacks(int sq);
    U64 KnightAttacks(int sq);
    U64 RookAttacks(U64 occ, int sq);
    U64 BishAttacks(U64 occ, int sq);
    U64 QueenAttacks(U64 occ, int sq);
};

extern cBitBoard BB;

struct UNDO {
    int mTtpUd;
    int mCFlagsUd;
    int mEpSqUd;
    int mRevMovesUd;
    U64 mHashKeyUd;
    U64 mPawnKeyUd;
};

class POS {
    static int msCastleMask[64];
    static U64 msZobPiece[12][64];
    static U64 msZobCastle[16];
    static U64 msZobEp[8];

    static eSquare Castle_W_RQ;
    static eSquare Castle_W_K;
    static eSquare Castle_W_RK;
    static eSquare Castle_B_RQ;
    static eSquare Castle_B_K;
    static eSquare Castle_B_RK;

    static const unsigned char CastleMask [8][8];  // Var2
    static U64 CastleMask_W_KS;
    static U64 CastleMask_W_QS;
    static U64 CastleMask_B_KS;
    static U64 CastleMask_B_QS;

    void ClearPosition();
    void InitHashKey();
    void InitPawnKey();

    U64 AttacksFrom(int sq) const;
    U64 AttacksTo(int sq) const;
    bool Attacked(int sq, eColor sd) const;

    bool CanDiscoverCheck(U64 bb_checkers, eColor op, int from) const; // for GenerateSpecial()

  public:
    U64 mClBb[2];
    U64 mTpBb[6];
    int mPc[64];
    int mKingSq[2];
    int mPhase;
    int mCnt[2][6];
    int mMgScore[2];
    int mEgScore[2];
    eColor mSide;
    int mCFlags;
    int mEpSq;
    int mRevMoves;
    int mHead;
    U64 mHashKey;
    U64 mPawnKey;
    U64 mRepList[256];

    NOINLINE static U64 Random64();

    static void Init();
    static void Init960();
    bool Is960();

    U64 Pawns(eColor sd)   const { return mClBb[sd] & mTpBb[P]; }
    U64 Knights(eColor sd) const { return mClBb[sd] & mTpBb[N]; }
    U64 Bishops(eColor sd) const { return mClBb[sd] & mTpBb[B]; }
    U64 Rooks(eColor sd)   const { return mClBb[sd] & mTpBb[R]; }
    U64 Queens(eColor sd)  const { return mClBb[sd] & mTpBb[Q]; }
    U64 Kings(eColor sd)   const { return mClBb[sd] & mTpBb[K]; }

    U64 StraightMovers(eColor sd) const { return mClBb[sd] & (mTpBb[R] | mTpBb[Q]); }
    U64 DiagMovers(eColor sd)     const { return mClBb[sd] & (mTpBb[B] | mTpBb[Q]); }

    U64 Map(eColor sd, int tp) const { return mClBb[sd] & mTpBb[tp]; }
    U64 Filled() const { return mClBb[WC] | mClBb[BC]; }
    U64 Empty()  const { return ~Filled(); }

    int MinorCount(eColor sd) const { return mCnt[sd][N] + mCnt[sd][B]; }
    int MajorCount(eColor sd) const { return mCnt[sd][R] + mCnt[sd][Q]; }
    int PieceCount(eColor sd) const { return MinorCount(sd) + MajorCount(sd); }

    int KingSq(eColor sd) const { return mKingSq[sd]; }
    int TpOnSq(int sq) const { return Tp(mPc[sq]); }

    bool MayNull() const { return (mClBb[mSide] & ~(mTpBb[P] | mTpBb[K])) != 0; }
    bool IsOnSq(eColor sd, int tp, int sq) const { return Map(sd, tp) & SqBb(sq); }

    bool InCheck() const { return Attacked(KingSq(mSide), ~mSide); }
    bool Illegal() const { return Attacked(KingSq(~mSide), mSide); }

    bool IsPawnComplex(eColor side, eSquare s1, eSquare s2, eSquare s3) {
        return (IsOnSq(side, P, s1) && IsOnSq(side, P, s2) && IsOnSq(side, P, s3));
    }

    bool IsPawnComplex(eColor side, eSquare s1, eSquare s2) {
        return (IsOnSq(side, P, s1) && IsOnSq(side, P, s2));
    }

    bool IsAnyPawn(eColor side, eSquare s1, eSquare s2) {
        return (IsOnSq(side, P, s1) || IsOnSq(side, P, s2));
    }

    void DoMove(int move, UNDO *u = nullptr);
    void DoNull(UNDO *u);
    void UndoNull(UNDO *u);
    void UndoMove(int move, UNDO *u);

    void SetPosition(const char *epd);

    bool IsDraw() const;
    bool KPKdraw(eColor sd) const;

    int DrawScore() const;
    bool Legal(int move) const;
    bool Unambiguous(int move) const;

    NOINLINE void PrintBoard() const;
    NOINLINE void ParseMoves(const char *ptr);
    void ParsePosition(const char *ptr);

    int *GenerateCaptures(int *list) const;
    int *GenerateQuiet(int *list) const;
    int *GenerateSpecial(int *list) const;

    int Swap(int from, int to);

    int StrToMove(const char *move_str) const;
    void ExtractMove(int *pv);
};

struct MOVES {
    POS *p;
    int phase;
    int transTableMove;
    int refutationMove;
    int refutationSquare;
    int killer1;
    int killer2;
    int *next;
    int *last;
    int move[MAX_MOVES];
    int value[MAX_MOVES];
    int *badp;
    int bad[MAX_MOVES];
};

struct ENTRY {
    U64 key;
    int16_t date;
    int16_t move;
    int16_t score;
    uint8_t flags;
    uint8_t depth;
};

class eData {
private:
    U64 nbrAttacks[2];
    U64 qAttacks[2];
public:
    int mgPrimaryPst[2];
    int egPrimaryPst[2];
    int mgSecondaryPst[2];
    int egSecondaryPst[2];
    int mgFlatMob[2];
    int egFlatMob[2];
    int mgDynMob[2];
    int egDynMob[2];
    int mg[2];
    int eg[2];
    int mgPawns[2];
    int egPawns[2];
    int att[2];
    U64 pawnTakes[2];
    U64 twoPawnsTake[2];
    U64 pawnCanTake[2];
    U64 allAttacks[2];
    
    void Init(POS * p);
    U64 GetNbrAttacks(int sd);
    U64 GetNbrqAttacks(int sd);
    void SetKnightAttacks(U64 control, int sd);
    void SetBishopAttacks(U64 control, int sd);
    void SetRookAttacks(U64 control, int sd);
    void SetQueenAttacks(U64 control, int sd);
};

struct sEvalHashEntry {
    U64 key;
    int score;
};

struct sPawnHashEntry {
    U64 key;
    int mg_white_pawns;
    int mg_black_pawns;
    int eg_white_pawns;
    int eg_black_pawns;
};

struct Line {
	int pv[MAX_PLY];
};

enum Values {
    P_MID, P_END, N_MID, N_END, B_MID, B_END, R_MID, R_END, Q_MID, Q_END,               // piece values
    B_PAIR_MG, B_PAIR_EG, N_PAIR, R_PAIR, ELEPH, A_EXC,                                 // material adjustments
    N_ATT1, N_ATT2, B_ATT1, B_ATT2, R_ATT1, R_ATT2, Q_ATT1, Q_ATT2,                     // attacks against enemy king zone
    N_CHK, B_CHK, R_CHK, Q_CHK, Q_CONTACT,                                              // check threats
	N_OWH_MG, N_OWH_EG, B_OWH_MG, B_OWH_EG, 
	N_REACH_MG, N_REACH_EG, B_REACH_MG, B_REACH_EG, 
	N_SH_MG, N_SH_EG, B_SH_MG, B_SH_EG,
    N_CL, R_OP, N_TRAP, N_BLOCK, K_NO_LUFT_MG, K_NO_LUFT_EG, K_CASTLE_KS, K_CASTLE_QS,
    B_TRAP_A2, B_TRAP_A3, B_BLOCK, B_FIANCH, B_BADF, B_KING, B_BF_MG, B_BF_EG,          // bishop parameters
    B_OPP_P, B_OWN_P, B_RETURN,
    P_SH_NONE, P_SH_2, P_SH_3, P_SH_4, P_SH_5, P_SH_6, P_SH_7,                          // king's pawn shield
    P_ST_OPEN, P_ST_3, P_ST_4, P_ST_5,                                                  // pawn storm on enemy king
    ISO_MG, ISO_EG, ISO_OF, BK_MID, BK_END, BK_OPE, DB_MID, DB_END,                     // pawn weaknesses
    PMG2, PMG3, PMG4, PMG5, PMG6, PMG7, PEG2, PEG3, PEG4, PEG5, PEG6, PEG7, P_BL_MUL,   // passed pawns
    CMG2, CMG3, CMG4, CMG5, CMG6, CEG2, CEG3, CEG4, CEG5, CEG6,                         // candidate passers
    P_OURSTOP_MUL, P_OPPSTOP_MUL, P_DEFMUL, P_STOPMUL, P_BIND, P_BADBIND, P_ISL,        // pawn special terms
    P_BIGCHAIN, P_SMALLCHAIN, P_CS1, P_CS2, P_CS_EDGE, P_CSFAIL,
    ROF_MG, ROF_EG, RGH_MG, RGH_EG, RBH_MG, RBH_EG, RSR_MG, RSR_EG, ROQ_MG, ROQ_EG,     // rook bonuses
    RS2_MG, RS2_EG, QSR_MG, QSR_EG, R_BLOCK_MG, R_BLOCK_EG,                             // queen and rook bonuses
    W_MATERIAL, W_PRIM, W_SECO, W_OWN_ATT, W_OPP_ATT, W_OWN_MOB, W_OPP_MOB, W_FLAT,       // weights part 1
    W_THREATS, W_TROPISM, W_PASSERS, W_SHIELD, W_STORM, W_MASS, W_CHAINS, W_STRUCT,     // weights part 2
    W_LINES, W_OUTPOSTS, W_SPACE,
	P_MOB_MG, P_MOB_EG,
    N_OF_VAL
};

const char* const paramNames[N_OF_VAL] = {
    "PawnValueMg", "PawnValueEg", "KnightValueMg", "KnightValueEg", "BishopValueMg", 
    "BishopValueEg", "RookValueMg", "RookValueEg", "QueenValueMg", "QueenValueEg",             // piece values
    "BishopPairMg", "BishopPairEg", "KnightPair", "RookPair", "ELEPH", "A_EXC",                // material adjustments
    "N_ATT1", "N_ATT2", "B_ATT1", "B_ATT2", "R_ATT1", "R_ATT2", "Q_ATT1", "Q_ATT2",            // attacks against enemy king zone
    "N_CHK", "B_CHK", "R_CHK", "Q_CHK", "Q_CONTACT",                                           // check threats
	"N_OWH_MG", "N_OWH_EG", "B_OWH_MG", "B_OWH_EG",
	"N_REACH_MG", "N_REACH_EG", "B_REACH_MG", "B_REACH_EG", 
	"N_SH_MG", "N_SH_EG", "B_SH_MG", "B_SH_EG",
    "N_CL", "R_OP", "N_TRAP", "N_BLOCK", "K_NO_LUFT_MG", "K_NO_LUFT_EG", "K_CASTLE_KS", "K_CASTLE_QS",
    "B_TRAP_A2", "B_TRAP_A3", "B_BLOCK", "B_FIANCH", "B_BADF", "B_KING", "B_BF_MG", "B_BF_EG",  // bishop parameters
    "B_OPP_P", "B_OWN_P", "B_RETURN",
    "P_SH_NONE", "P_SH_2", "P_SH_3", "P_SH_4", "P_SH_5", "P_SH_6", "P_SH_7",                    // king's pawn shield
    "P_ST_OPEN", "P_ST_3", "P_ST_4", "P_ST_5",                                                  // pawn storm on enemy king
    "ISO_MG", "ISO_EG", "ISO_OF", "BK_MID", "BK_END", "BK_OPE", "DB_MID", "DB_END",             // pawn weaknesses
    "PMG2", "PMG3", "PMG4", "PMG5", "PMG6", "PMG7", "PEG2", "PEG3", "PEG4", "PEG5", "PEG6", "PEG7", "P_BL_MUL",   // passed pawns
    "CMG2", "CMG3", "CMG4", "CMG5", "CMG6", "CEG2", "CEG3", "CEG4", "CEG5", "CEG6",             // candidate passers
    "P_OURSTOP_MUL", "P_OPPSTOP_MUL", "P_DEFMUL", "P_STOPMUL", "P_BIND", "P_BADBIND", "P_ISL",  // pawn special terms
    "P_BIGCHAIN", "P_SMALLCHAIN", "P_CS1", "P_CS2", "P_CS_EDGE", "P_CSFAIL",
    "ROF_MG", "ROF_EG", "RGH_MG", "RGH_EG", "RBH_MG", "RBH_EG", "RSR_MG", "RSR_EG", "ROQ_MG", "ROQ_EG",     // rook bonuses
    "RS2_MG", "RS2_EG", "QSR_MG", "QSR_EG", "R_BLOCK_MG",  "R_BLOCK_EG",                                    // queen and rook bonuses
    "Material", "PrimaryPstWeight", "SecondaryPstWeight", "OwnAttack", "OppAttack", "OwnMobility", "OppMobility", "FlatMobility",
    "PiecePressure", // weights part 1
    "KingTropism", "PassedPawns", "PawnShield", "PawnStorm", "W_MASS", "W_CHAINS", "PawnStructure", // weights part 2
    "Lines", "Outposts", "Space",
    "P_MOB_MG", "P_MOB_EG",
};

#define V(x) (Par.values[x]) // a little shorthand to unclutter eval code

class cParam {
  public:
	int wait[N_OF_VAL];
    int values[N_OF_VAL]; // evaluation parameters
    int max_val[N_OF_VAL];
    int min_val[N_OF_VAL];
    bool tunable[N_OF_VAL];
    bool use_ponder; // this option does nothing
	bool chess960;
    bool useBook;
    bool verboseBook;
    int bookFilter;
    int bookDepth;
    int elo;
    bool useMobilityRebalancing;
    bool useWeakening;
    bool shut_up;
    int timePercentage;
    int drawScore;
    eColor programSide;
    int searchSkill;
    int npsLimit;
    int evalBlur;
    int hist_perc;
    int histLimit;
    int keep[7];
    int sideAttack[2];
    int sideMobility[2];
    int mg_mat[6];
    int eg_mat[6];

    int sp_pst[2][6][64];
    int mgPrimaryPstData[2][6][64];
    int egPrimaryPstData[2][6][64];
    int mgSecondaryPstData[2][6][64];
    int egSecondaryPstData[2][6][64];
    int passed_bonus_mg[2][8];
    int passed_bonus_eg[2][8];
    int cand_bonus_mg[2][8];
    int cand_bonus_eg[2][8];
    int primaryPstStyle;
    int secondaryPstStyle;
    int danger[512];
    int np_table[9];
    int rp_table[9];
    int backward_malus_mg[8];

    NOINLINE void InitPst();
    NOINLINE void InitBackward();
    NOINLINE void InitPassers();
    NOINLINE void InitMaterialTweaks();
    NOINLINE void InitKingAttackTable();
    NOINLINE void DefaultWeights();
    NOINLINE void InitAsymmetric(POS *p);
    NOINLINE void PrintValues(int startTune, int endTune);
    void Recalculate();
    void SetSpeed(int elo_in);
    int EloToSpeed(int elo_in);
    int EloToBlur(int elo_in);
    int SpeedToBookDepth(int nps);
    void SetVal(int slot, int val, int min, int max, bool tune);
};

extern cParam Par;

class cDistance {
  public:
    int metric[64][64]; // chebyshev distance for unstoppable passers
    int bonus[64][64];
    int grid[64][64];
    int knightTropism[64][64];
    int bishopTropism[64][64];
    int rookTropism[64][64];
    int queenTropism[64][64];
    void Init();
};

extern cDistance Dist;

class cMask {
  public:
    void Init();
    bool IsOnRank7(U64 b, int side);
    bool IsOnRank8(U64 b, int side);

    // masks of own and opponent's sides of the board

    static constexpr U64 home[2] = { RANK_1_BB | RANK_2_BB | RANK_3_BB | RANK_4_BB,
                                     RANK_8_BB | RANK_7_BB | RANK_6_BB | RANK_5_BB };
    static constexpr U64 away[2] = { RANK_8_BB | RANK_7_BB | RANK_6_BB | RANK_5_BB,
                                     RANK_1_BB | RANK_2_BB | RANK_3_BB | RANK_4_BB };

    // mask of castled king squares

    static constexpr U64 ksCastle[2] = { SqBb(F1) | SqBb(G1) | SqBb(H1) | SqBb(F2) | SqBb(G2) | SqBb(H2),
                                         SqBb(F8) | SqBb(G8) | SqBb(H8) | SqBb(F7) | SqBb(G7) | SqBb(H7) };
    static constexpr U64 qsCastle[2] = { SqBb(A1) | SqBb(B1) | SqBb(C1) | SqBb(A2) | SqBb(B2) | SqBb(C2),
                                         SqBb(A8) | SqBb(B8) | SqBb(C8) | SqBb(A7) | SqBb(B7) | SqBb(C7) };

    // mask of squares taken into account in outpost evaluation

    static constexpr U64 outpostMap[2] = { (bbRelRank[WC][RANK_4] | bbRelRank[WC][RANK_5] | bbRelRank[WC][RANK_6]) & bbNotA & bbNotH,
                                           (bbRelRank[BC][RANK_4] | bbRelRank[BC][RANK_5] | bbRelRank[BC][RANK_6]) & bbNotA & bbNotH };

    // masks of wing squares

    static constexpr U64 kingSide = FILE_F_BB | FILE_G_BB | FILE_H_BB;
    static constexpr U64 queenSide = FILE_A_BB | FILE_B_BB | FILE_C_BB;

    // mask of central squares

    static constexpr U64 center = SqBb(C3) | SqBb(D3) | SqBb(E3) | SqBb(F3)
                                | SqBb(C4) | SqBb(D4) | SqBb(E4) | SqBb(F4)
                                | SqBb(C5) | SqBb(D5) | SqBb(E5) | SqBb(F5)
                                | SqBb(C6) | SqBb(D6) | SqBb(E6) | SqBb(F6);

    // mask of squares taken into account in space evaluation

    static constexpr U64 space[2] = { bbCentralFile & (RANK_2_BB | RANK_3_BB | RANK_4_BB | RANK_5_BB),
                                      bbCentralFile & (RANK_7_BB | RANK_6_BB | RANK_5_BB | RANK_4_BB) };

    // mask showing bishop positions where pattern evaluation can be applied

    static constexpr U64 wb_special = SqBb(A7) | SqBb(A6) | SqBb(B8) | SqBb(H7) | SqBb(H6) | SqBb(G8) 
                                    | SqBb(C1) | SqBb(F1) | SqBb(G2) | SqBb(B2) | SqBb(A1) | SqBb(H1);
    static constexpr U64 bb_special = SqBb(A2) | SqBb(A3) | SqBb(B1) | SqBb(H2) | SqBb(H3) | SqBb(G1) 
                                    | SqBb(C8) | SqBb(F8) | SqBb(G7) | SqBb(B7) | SqBb(A8) | SqBb(H8);

    U64 adjacent[8];
    U64 passed[2][64];
    U64 supported[2][64];

    static_assert(WC == 0 && BC == 1, "must be WC == 0 && BC == 1");
};

extern cMask Mask;

#if defined(USE_THREADS)
    #include <atomic>

    using glob_bool = std::atomic<bool>;
    using glob_int  = std::atomic<int>;
    using glob_U64  = std::atomic<uint64_t>;
#else
    using glob_bool = bool;
    using glob_int  = int;
    using glob_U64  = uint64_t;
#endif

class cGlobals {
  public:
    glob_int finishedDepth;
    glob_int threadOverride;
    glob_U64 nodes;
    glob_bool abortSearch;
    glob_bool isTesting;
    glob_bool isBenching;
	bool isNoisy;
    bool eloSlider;
    bool isConsole;
    bool isTuning;
    bool useTaunting;
    bool printPv;
    glob_bool pondering;
    glob_bool infinite;
    bool isReadingPersonality;
    bool useBooksFromPers;
    bool shouldClear;
    bool goodbye;
    bool usePersonalityFiles;
	bool scoreJump;
    bool showPersonalityFile;
    bool useUciPersonalitySet;
    std::string personalityW;
    std::string personalityB;
    glob_int depthReached;
    int moves_from_start; // to restrict book depth for weaker levels
    int numberOfThreads;
	int multiPv;
    int timeBuffer;
    U64 game_key;         // random key initialized on ucinewgame to ensure non-repeating random eval modification for weak personalities
    int avoidMove[MAX_PV + 1]; // list of moves to avoid in multi-pv re-searches
    int gameValue; // for more accurte taunts
    int previousValue;
    int previousTaunt;
    int currentTaunt;
    eCastleNotation CastleNotation;
    eGUIs usedGUI;

    void ClearData();
    void Init();
    bool CanReadBook();
	bool MoveToAvoid(int move);
	void ClearAvoidList();
	void SetAvoidMove(int loc, int move);
};

extern cGlobals Glob;

#define ZEROARRAY(x) memset(x, 0, sizeof(x))

void CheckTimeout();

constexpr int EVAL_HASH_SIZE = 512 * 512 / 4;
constexpr int PAWN_HASH_SIZE = 512 * 512 / 4;

struct LineFullInfo {
    int depth;
    int val;
    Line line;
};

class cEngine {
    sEvalHashEntry mEvalTT[EVAL_HASH_SIZE];
    sPawnHashEntry mPawnTT[PAWN_HASH_SIZE];
    int mHistory[12][64];
    int mEvalStack[MAX_PLY];
    int mKiller[MAX_PLY][2];
    int mRefutation[64][64];
    const int mcThreadId;
    int mRootDepth;
    bool mFlRootChoice;
	int mEngSide;

#ifdef USE_TUNING

    std::string epd10[10000000];
    std::string epd01[10000000];
    std::string epd05[10000000];

    int cnt10;
    int cnt01;
    int cnt05;
    int step = 20;

#endif

    static void InitCaptures(POS *p, MOVES *m);
    void InitMoves(POS *p, MOVES *m, int trans_move, int ref_move, int ref_sq, int ply);
    int NextMove(MOVES *m, int *flag, int ply);
    int NextSpecialMove(MOVES *m, int *flag);
    static int NextCapture(MOVES *m);
    static void ScoreCaptures(MOVES *m);
    void ScoreQuiet(MOVES *m, int ply);
    static int SelectBest(MOVES *m);
    static int BadCapture(POS *p, int move);
    static int MvvLva(POS *p, int move);
    void ClearHist();
    void AgeHist();
    void ClearEvalHash();
    void ClearPawnHash();
    int Refutation(int move);
    void UpdateHistory(POS *p, int last_move, int move, int depth, int ply);
    void DecreaseHistory(POS *p, int move, int depth);
    void TrimHist();

    void Iterate(POS *p, int *pv);
    int Widen(POS *p, int depth, int *pv, int lastScore);
	int SearchRoot(POS *p, int ply, int alpha, int beta, int depth, int *pv);
    int Search(POS *p, int ply, int alpha, int beta, int depth, bool was_null, int last_move, int last_capt_sq, int *pv);
    int QuiesceChecks(POS *p, int ply, int alpha, int beta, int *pv);
    int QuiesceFlee(POS *p, int ply, int alpha, int beta, int *pv);
    int Quiesce(POS *p, int ply, int alpha, int beta, int *pv);
    void DisplayPv(int multipv, int score, int *pv);
    void DisplayPvDepth(int depth, int multipv, int score, int *pv);
    void ShowMultiPVInfo(POS * p, int curMove, int curDepth,
                         int valThisDepth[MAX_PV + 1], Line lineThisDepth[MAX_PV + 1],
                         int valLastDepth[MAX_PV + 1], Line lineLastDepth[MAX_PV + 1]);
    void Slowdown();
    int SetLateMoveReduction(bool isPv, int depth, int movesTried, int moveHistScore, bool improving);
    int SetNullReductionDepth(int depth, int eval, int beta);
    bool TimeTestIntervalReached();

    int Evaluate(POS *p, eData *e);

    static int EvaluateChains(POS *p, eColor sd);
    static void EvaluateMaterial(POS *p, eData *e, eColor sd);
    static void EvaluatePieces(POS *p, eData *e, eColor sd);
	static void EvaluateShielded(POS *p, eData *e, eColor sd, int sq, int v1, int v2, int *outpost_mg, int *outpost_eg);
    static void EvaluateOutpost(POS *p, eData *e, eColor sd, int pc, int sq, int *outpost_mg, int *outpost_eg);
    static void EvaluatePawns(POS *p, eData *e, eColor sd);
    static void EvaluatePassers(POS *p, eData *e, eColor sd);
    static void EvaluateKing(POS *p, eData *e, eColor sd);
    static void EvaluateKingFile(POS *p, eColor sd, U64 bb_file, int *shield, int *storm);
    static int EvaluateFileShelter(U64 bb_own_pawns, eColor sd);
    static int EvaluateFileStorm(POS * p, U64 bb_opp_pawns, eColor sd);
    void EvaluatePawnStruct(POS *p, eData *e);
    static void EvaluateUnstoppable(eData *e, POS *p);
    static void EvaluateThreats(POS *p, eData *e, eColor sd);
    static int ScalePawnsOnly(POS *p, eColor sd, eColor op);
    static int ScaleKBPK(POS *p, eColor sd, eColor op);
    static int ScaleKNPK(POS *p, eColor sd, eColor op);
    static int ScaleKRPKR(POS *p, eColor sd, eColor op);
    static int ScaleKQKRP(POS *p, eColor sd, eColor op);
    
    static void EvaluateBishopPatterns(POS *p, eData *e);
    static void EvaluateKnightPatterns(POS *p, eData *e);
    static void EvaluateCentralPatterns(POS *p, eData *e);
    static void EvaluateKingPatterns(POS *p, eData *e);
    static void EvalFianchetto(POS *p, eData *e, eColor side, eSquare bSq, eSquare pSq, eSquare sq, eSquare s2, 
                               eSquare obl, eSquare b1, eSquare b2, eSquare b3, U64 kingMask);
    static void EvalBishopOnInitial(POS *p, eData *e, eColor side, eSquare bSq, eSquare pSq, eSquare blockSq, U64 king);
    static void EvaluateKingAttack(POS *p, eData *e, eColor sd);
    void EvalPin(POS * p, eData * e, eColor c, eSquare pinned0, eSquare pinned1, eSquare knight, eSquare bish0, eSquare bish1);


    static int Interpolate(POS *p, eData *e);
    static int GetDrawFactor(POS *p, eColor sd);
    static int CheckmateHelper(POS *p);
    static void Add(eData *e, eColor sd, int mg_val, int eg_val);
    static void Add(eData *e, eColor sd, int val);
    static void AddPawns(eData *e, eColor sd, int mg_val, int eg_val);
    static void AddPst(eData *e, eColor c, int type, eSquare s);
    static bool NotOnBishColor(POS *p, eColor bish_side, int sq);
    static bool DifferentBishops(POS *p);
    static void PvToStr(int *pv, char *pv_str);
    static void BuildPv(int *dst, int *src, int move);
    static void WasteTime(int milliseconds);
    static int BulletCorrection(int time);

    bool ShouldAbortSearch();

    static const int mscRazorMargin[];
    static const int mscFutMargin[];
    static const int mscSelectiveDepth;
    static const int mscSnpDepth;      // max depth at which static null move pruning is applied
    static const int mscRazorDepth;    // max depth at which razoring is applied
    static const int mscFutDepth;      // max depth at which futility pruning is applied
    static const int mscSingDepth;     // min depth at which singular extension is applied
    static const int mscSEEmargin;     // margin for SEE pruning of bad captures
    static int msLmrSize[2][MAX_PLY][MAX_MOVES];

  public:

    static int msMoveTime;
    static int msMoveNodes;
    static int msSearchDepth;
    static int msStartTime;

    static void InitSearch();
    static void ReadyForBestmove();

    int mPvEng[MAX_PLY];
    int mDpCompleted;

    cEngine(const cEngine&) = delete;
    cEngine& operator=(const cEngine&) = delete;
    cEngine(int th = 0): mcThreadId(th) { ClearAll(); };

#ifdef USE_THREADS
    std::thread mWorker;
    void StartThinkThread(POS *p) {
        mDpCompleted = 0;
        mWorker = std::thread([&] { Think(p); });
    }

    ~cEngine() { WaitThinkThread(); };  // should fix crash on windows on console closing
    void WaitThinkThread() { if (mWorker.joinable()) mWorker.join(); }
#endif

    static void SetMoveTime(int base, int inc, int movestogo);

    void Bench(int depth);
    void ClearAll();
    void Think(POS *p);
	void MultiPv(POS *p, int * pv);

#ifdef USE_TUNING
    double best_tune;
    double TexelFit(POS *p, int *pv);
    bool TuneOne(POS *p, int *pv, int par);
    void TuneMe(POS *p, int *pv, int iterations);
    void LoadEpd();
    double TexelSigmoid(int score, double k);
#endif
};

#ifdef USE_THREADS
    #include <list>
    extern std::list<cEngine> Engines;
#else
    extern cEngine EngineSingle;
#endif

void PrintVersion();

void DisplayCurrmove(int move, int tried);
int GetMS();
U64 GetNps(int elapsed);
bool InputAvailable();
std::string MoveToStr(int move);
void MoveToStr(int move, char *move_str);
void ParseGo(POS *p, const char *ptr);
void ParseSetoption(const char *);
const char *ParseToken(const char *, char *);
void PrintMove(int move);
void PrintSingleOption(int ind);
void PrintUciOptions();
void ReadLine(char *, int);
void ReadPersonality(const char *fileName);
void ReadThreadNumber(const char *fileName);
void UciLoop();
int PolyglotRandom(int n);
int Percent(int val, int perc);

extern const int tp_value[7];
extern const int ph_value[7];

#define MAKESTRHLP(x) #x
#define MAKESTR(x) MAKESTRHLP(x)

// macro BOOKSPATH is where books live, default is relative "books/"
// macro PERSONALITIESPATH is where personalities and `basic.ini` live, default is relative "personalities/"

#if defined(_WIN32) || defined(_WIN64)
    #if defined(BOOKSPATH)
        constexpr wchar_t _BOOKSPATH[] = MAKESTR(BOOKSPATH) L"";
    #else
        constexpr wchar_t _BOOKSPATH[] = L"books\\";
    #endif
    #if defined(PERSONALITIESPATH)
        constexpr wchar_t _PERSONALITIESPATH[] = MAKESTR(PERSONALITIESPATH) L"";
    #else
        constexpr wchar_t _PERSONALITIESPATH[] = L"personalities\\";
    #endif
    #define PrintOverrides() {}
    // change dir and return true on success
    #define ChDirEnv(dummy) false
    bool ChDir(const wchar_t *new_path);
    // classify path
    constexpr bool isabsolute(const char *path) { return path[0] != '\0' && path[1] == ':'; }
    #if __CYGWIN__
        // Workaround for win-builds with cygwin
        std::string GetFullPath(const char *exe_file);
        #define fopen(file, mode) ( fopen(GetFullPath(file).c_str(), mode) )
    #endif

#else
    #if defined(BOOKSPATH)
        constexpr char _BOOKSPATH[] = MAKESTR(BOOKSPATH) "";
    #else
        constexpr char _BOOKSPATH[] = "books/";
    #endif
    #if defined(PERSONALITIESPATH)
        constexpr char _PERSONALITIESPATH[] = MAKESTR(PERSONALITIESPATH) "";
    #else
        constexpr char _PERSONALITIESPATH[] = "personalities/";
    #endif
    void PrintOverrides();
    // change dir and return true on success
    #ifndef ANDROID
        bool ChDirEnv(const char *env_name);
    #else
        #define ChDirEnv(dummy) false
    #endif
	
    bool ChDir(const char *new_path);
    // classify path
    constexpr bool isabsolute(const char *path) { return path[0] == '/'; }
#endif

#define printfUciIn(...)  printfLog(">> ", __VA_ARGS__)
#define printfUciOut(...) printfLog("<< ", __VA_ARGS__)
#define printfUciAdd(...) printfLog("", __VA_ARGS__)
void printfLog(const char *preStr, const char *fmt, ...);

#ifndef NDEBUG
    #define printf_debug(...) printfLog("", "(debug) " __VA_ARGS__)
#else
    #define printf_debug(...) {}
#endif

#include "chessheapclass.h"
extern ChessHeapClass Trans;

#ifndef NO_THREADS
    extern int tDepth[MAX_THREADS];
#endif

// I don't belive anybody will use UNICODE in pathnames (only filenames), but
// still being ready for it using (more complex) wstring:
extern std::wstring RodentHomeDirWStr;
extern std::wstring LogFileWStr;

// as the GUI can set the filename for the log, we skip writing anything to the log,
// till we got "setoption name LogFile value ..." or "ucinewgame" (in release build only).
extern bool SkipBeginningOfLog;

void FullPathEndSlash(std::wstring &pathWStr);
void SetRodentHomeDir();
void CreateRodentHome(const char *RodentDir);
void ChangePersonalitySet(int persSetNo);

extern unsigned char CastleFile_RQ;
extern unsigned char CastleFile_RK;

void CheckGUI();
