/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2017 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>

#include "types.h"

Value PieceValue[VARIANT_NB][PHASE_NB][PIECE_NB] = {
{
  { VALUE_ZERO, PawnValueMg, KnightValueMg, BishopValueMg, RookValueMg, QueenValueMg },
  { VALUE_ZERO, PawnValueEg, KnightValueEg, BishopValueEg, RookValueEg, QueenValueEg },
},
#ifdef ANTI
{
  { VALUE_ZERO, PawnValueMgAnti, KnightValueMgAnti, BishopValueMgAnti, RookValueMgAnti, QueenValueMgAnti, KingValueMgAnti },
  { VALUE_ZERO, PawnValueEgAnti, KnightValueEgAnti, BishopValueEgAnti, RookValueEgAnti, QueenValueEgAnti, KingValueEgAnti },
},
#endif
#ifdef ATOMIC
{
  { VALUE_ZERO, PawnValueMgAtomic, KnightValueMgAtomic, BishopValueMgAtomic, RookValueMgAtomic, QueenValueMgAtomic },
  { VALUE_ZERO, PawnValueEgAtomic, KnightValueEgAtomic, BishopValueEgAtomic, RookValueEgAtomic, QueenValueEgAtomic },
},
#endif
#ifdef CRAZYHOUSE
{
  { VALUE_ZERO, PawnValueMgHouse, KnightValueMgHouse, BishopValueMgHouse, RookValueMgHouse, QueenValueMgHouse },
  { VALUE_ZERO, PawnValueEgHouse, KnightValueEgHouse, BishopValueEgHouse, RookValueEgHouse, QueenValueEgHouse },
},
#endif
#ifdef HORDE
{
  { VALUE_ZERO, PawnValueMgHorde, KnightValueMgHorde, BishopValueMgHorde, RookValueMgHorde, QueenValueMgHorde, KingValueMgHorde },
  { VALUE_ZERO, PawnValueEgHorde, KnightValueEgHorde, BishopValueEgHorde, RookValueEgHorde, QueenValueEgHorde, KingValueEgHorde },
},
#endif
#ifdef KOTH
{
  { VALUE_ZERO, PawnValueMgHill, KnightValueMgHill, BishopValueMgHill, RookValueMgHill, QueenValueMgHill },
  { VALUE_ZERO, PawnValueEgHill, KnightValueEgHill, BishopValueEgHill, RookValueEgHill, QueenValueEgHill },
},
#endif
#ifdef LOSERS
{
  { VALUE_ZERO, PawnValueMgLosers, KnightValueMgLosers, BishopValueMgLosers, RookValueMgLosers, QueenValueMgLosers },
  { VALUE_ZERO, PawnValueEgLosers, KnightValueEgLosers, BishopValueEgLosers, RookValueEgLosers, QueenValueEgLosers },
},
#endif
#ifdef RACE
{
  { VALUE_ZERO, VALUE_ZERO, KnightValueMgRace, BishopValueMgRace, RookValueMgRace, QueenValueMgRace },
  { VALUE_ZERO, VALUE_ZERO, KnightValueEgRace, BishopValueEgRace, RookValueEgRace, QueenValueEgRace },
},
#endif
#ifdef RELAY
{
  { VALUE_ZERO, PawnValueMg, KnightValueMg, BishopValueMg, RookValueMg, QueenValueMg },
  { VALUE_ZERO, PawnValueEg, KnightValueEg, BishopValueEg, RookValueEg, QueenValueEg },
},
#endif
#ifdef THREECHECK
{
  { VALUE_ZERO, PawnValueMgThreeCheck, KnightValueMgThreeCheck, BishopValueMgThreeCheck, RookValueMgThreeCheck, QueenValueMgThreeCheck },
  { VALUE_ZERO, PawnValueEgThreeCheck, KnightValueEgThreeCheck, BishopValueEgThreeCheck, RookValueEgThreeCheck, QueenValueEgThreeCheck },
},
#endif
};

namespace PSQT {

#define S(mg, eg) make_score(mg, eg)

// Bonus[PieceType][Square / 2] contains Piece-Square scores. For each piece
// type on a given square a (middlegame, endgame) score pair is assigned. Table
// is defined for files A..D and white side: it is symmetric for black side and
// second half of the files.
const Score Bonus[VARIANT_NB][PIECE_TYPE_NB][RANK_NB][int(FILE_NB) / 2] = {
  {
    { },
    { // Pawn
      { S(  0, 0), S(  0, 0), S(  0, 0), S( 0, 0) },
      { S(-11, 7), S(  6,-4), S(  7, 8), S( 3,-2) },
      { S(-18,-4), S( -2,-5), S( 19, 5), S(24, 4) },
      { S(-17, 3), S( -9, 3), S( 20,-8), S(35,-3) },
      { S( -6, 8), S(  5, 9), S(  3, 7), S(21,-6) },
      { S( -6, 8), S( -8,-5), S( -6, 2), S(-2, 4) },
      { S( -4, 3), S( 20,-9), S( -8, 1), S(-4,18) }
    },
    { // Knight
     { S(-144, -98), S(-96,-82), S(-80,-46), S(-73,-14) },
     { S( -83, -69), S(-43,-54), S(-21,-17), S(-10,  9) },
     { S( -71, -50), S(-22,-39), S(  0, -7), S(  9, 28) },
     { S( -25, -41), S( 18,-25), S( 43,  6), S( 47, 38) },
     { S( -26, -46), S( 16,-25), S( 38,  3), S( 50, 40) },
     { S( -11, -54), S( 37,-38), S( 56, -7), S( 65, 27) },
     { S( -62, -65), S(-17,-50), S(  5,-24), S( 14, 13) },
     { S(-194,-109), S(-66,-89), S(-42,-50), S(-29,-13) }
    },
    { // Bishop
     { S(-44,-58), S(-13,-31), S(-25,-37), S(-34,-19) },
     { S(-20,-34), S( 20, -9), S( 12,-14), S(  1,  4) },
     { S( -9,-23), S( 27,  0), S( 21, -3), S( 11, 16) },
     { S(-11,-26), S( 28, -3), S( 21, -5), S( 10, 16) },
     { S(-11,-26), S( 24, -4), S( 16, -7), S(  9, 14) },
     { S(-17,-24), S( 16, -2), S( 12,  0), S(  2, 13) },
     { S(-23,-34), S( 17,-10), S(  6,-12), S( -2,  6) },
     { S(-35,-55), S(-11,-32), S(-19,-36), S(-29,-17) }
    },
    { // Rook
      { S(-25, 0), S(-16, 0), S(-16, 0), S(-9, 0) },
      { S(-21, 0), S( -8, 0), S( -3, 0), S( 0, 0) },
      { S(-21, 0), S( -9, 0), S( -4, 0), S( 2, 0) },
      { S(-22, 0), S( -6, 0), S( -1, 0), S( 2, 0) },
      { S(-22, 0), S( -7, 0), S(  0, 0), S( 1, 0) },
      { S(-21, 0), S( -7, 0), S(  0, 0), S( 2, 0) },
      { S(-12, 0), S(  4, 0), S(  8, 0), S(12, 0) },
      { S(-23, 0), S(-15, 0), S(-11, 0), S(-5, 0) }
    },
    { // Queen
     { S( 0,-71), S(-4,-56), S(-3,-42), S(-1,-29) },
     { S(-4,-56), S( 6,-30), S( 9,-21), S( 8, -5) },
     { S(-2,-39), S( 6,-17), S( 9, -8), S( 9,  5) },
     { S(-1,-29), S( 8, -5), S(10,  9), S( 7, 19) },
     { S(-3,-27), S( 9, -5), S( 8, 10), S( 7, 21) },
     { S(-2,-40), S( 6,-16), S( 8,-10), S(10,  3) },
     { S(-2,-55), S( 7,-30), S( 7,-21), S( 6, -6) },
     { S(-1,-74), S(-4,-55), S(-1,-43), S( 0,-30) }
    },
    { // King
     { S(267,  0), S(320, 48), S(270, 75), S(195, 84) },
     { S(264, 43), S(304, 92), S(238,143), S(180,132) },
     { S(200, 83), S(245,138), S(176,167), S(110,165) },
     { S(177,106), S(185,169), S(148,169), S(110,179) },
     { S(149,108), S(177,163), S(115,200), S( 66,203) },
     { S(118, 95), S(159,155), S( 84,176), S( 41,174) },
     { S( 86, 50), S(128, 99), S( 63,122), S( 18,139) },
     { S( 63,  9), S( 89, 55), S( 47, 80), S(  0, 90) }
    }
  },
#ifdef ANTI
  {
    { },
    { // Pawn
      { S(  0, 0), S(  0, 0), S(  0, 0), S( 0, 0) },
      { S(-11, 7), S(  6,-4), S(  7, 8), S( 3,-2) },
      { S(-18,-4), S( -2,-5), S( 19, 5), S(24, 4) },
      { S(-17, 3), S( -9, 3), S( 20,-8), S(35,-3) },
      { S( -6, 8), S(  5, 9), S(  3, 7), S(21,-6) },
      { S( -6, 8), S( -8,-5), S( -6, 2), S(-2, 4) },
      { S( -4, 3), S( 20,-9), S( -8, 1), S(-4,18) }
    },
    { // Knight
      { S(-143, -97), S(-96,-82), S(-80,-46), S(-73,-14) },
      { S( -83, -69), S(-43,-55), S(-21,-17), S(-10,  9) },
      { S( -71, -50), S(-22,-39), S(  0, -8), S(  9, 28) },
      { S( -25, -41), S( 18,-25), S( 43,  7), S( 47, 38) },
      { S( -26, -46), S( 16,-25), S( 38,  2), S( 50, 41) },
      { S( -11, -55), S( 37,-38), S( 56, -8), S( 71, 27) },
      { S( -62, -64), S(-17,-50), S(  5,-24), S( 14, 13) },
      { S(-195,-110), S(-66,-90), S(-42,-50), S(-29,-13) }
    },
    { // Bishop
      { S(-54,-68), S(-23,-40), S(-35,-46), S(-44,-28) },
      { S(-30,-43), S( 10,-17), S(  2,-23), S( -9, -5) },
      { S(-19,-32), S( 17, -9), S( 11,-13), S(  1,  8) },
      { S(-21,-36), S( 18,-13), S( 11,-15), S(  0,  7) },
      { S(-21,-36), S( 14,-14), S(  6,-17), S( -1,  3) },
      { S(-27,-35), S(  6,-13), S(  2,-10), S( -8,  1) },
      { S(-33,-44), S(  7,-21), S( -4,-22), S(-12, -4) },
      { S(-45,-65), S(-21,-42), S(-29,-46), S(-39,-27) }
    },
    { // Rook
      { S(-25, 0), S(-16, 0), S(-16, 0), S(-9, 0) },
      { S(-21, 0), S( -8, 0), S( -3, 0), S( 0, 0) },
      { S(-21, 0), S( -9, 0), S( -4, 0), S( 2, 0) },
      { S(-22, 0), S( -6, 0), S( -1, 0), S( 2, 0) },
      { S(-22, 0), S( -7, 0), S(  0, 0), S( 1, 0) },
      { S(-21, 0), S( -7, 0), S(  0, 0), S( 2, 0) },
      { S(-12, 0), S(  4, 0), S(  8, 0), S(12, 0) },
      { S(-23, 0), S(-15, 0), S(-11, 0), S(-5, 0) }
    },
    { // Queen
      { S( 0,-70), S(-3,-57), S(-4,-41), S(-1,-29) },
      { S(-4,-58), S( 6,-30), S( 9,-21), S( 8, -4) },
      { S(-2,-39), S( 6,-17), S( 9, -7), S( 9,  5) },
      { S(-1,-29), S( 8, -5), S(10,  9), S( 7, 17) },
      { S(-3,-27), S( 9, -5), S( 8, 10), S( 7, 23) },
      { S(-2,-40), S( 6,-16), S( 8,-11), S(10,  3) },
      { S(-2,-54), S( 7,-30), S( 7,-21), S( 6, -7) },
      { S(-1,-75), S(-4,-54), S(-1,-44), S( 0,-30) }
    },
    { // King
      { S(291, 28), S(344, 76), S(294,103), S(219,112) },
      { S(289, 70), S(329,119), S(263,170), S(205,159) },
      { S(226,109), S(271,164), S(202,195), S(136,191) },
      { S(204,131), S(212,194), S(175,194), S(137,204) },
      { S(177,132), S(205,187), S(143,224), S( 94,227) },
      { S(147,118), S(188,178), S(113,199), S( 70,197) },
      { S(116, 72), S(158,121), S( 93,142), S( 48,161) },
      { S( 94, 30), S(120, 76), S( 78,101), S( 31,111) }
    }
  },
#endif
#ifdef ATOMIC
  {
    { },
    { // Pawn
      { S(  0, 0), S(  0,  0), S(  0, 0), S( 0, 0) },
      { S( -9,12), S(  7, -4), S( 12, 6), S( 6,-3) },
      { S(-18,-6), S(  0,-10), S( 18, 4), S(23, 6) },
      { S(-15,-1), S( -7,  1), S( 21,-7), S(36,-4) },
      { S(-10, 7), S(  4,  7), S(  8, 9), S(26,-4) },
      { S(-13,10), S( -5, -3), S( -3, 2), S( 1, 7) },
      { S( -2,-1), S( 16, -5), S( -6, 2), S(-3,28) }
    },
    { // Knight
      { S(-143, -97), S(-96,-82), S(-80,-46), S(-73,-14) },
      { S( -83, -69), S(-43,-55), S(-21,-17), S(-10,  9) },
      { S( -71, -50), S(-22,-39), S(  0, -8), S(  9, 28) },
      { S( -25, -41), S( 18,-25), S( 43,  7), S( 47, 38) },
      { S( -26, -46), S( 16,-25), S( 38,  2), S( 50, 41) },
      { S( -11, -55), S( 37,-38), S( 56, -8), S( 71, 27) },
      { S( -62, -64), S(-17,-50), S(  5,-24), S( 14, 13) },
      { S(-195,-110), S(-66,-90), S(-42,-50), S(-29,-13) }
    },
    { // Bishop
      { S(-54,-68), S(-23,-40), S(-35,-46), S(-44,-28) },
      { S(-30,-43), S( 10,-17), S(  2,-23), S( -9, -5) },
      { S(-19,-32), S( 17, -9), S( 11,-13), S(  1,  8) },
      { S(-21,-36), S( 18,-13), S( 11,-15), S(  0,  7) },
      { S(-21,-36), S( 14,-14), S(  6,-17), S( -1,  3) },
      { S(-27,-35), S(  6,-13), S(  2,-10), S( -8,  1) },
      { S(-33,-44), S(  7,-21), S( -4,-22), S(-12, -4) },
      { S(-45,-65), S(-21,-42), S(-29,-46), S(-39,-27) }
    },
    { // Rook
      { S(-25, 0), S(-16, 0), S(-16, 0), S(-9, 0) },
      { S(-21, 0), S( -8, 0), S( -3, 0), S( 0, 0) },
      { S(-21, 0), S( -9, 0), S( -4, 0), S( 2, 0) },
      { S(-22, 0), S( -6, 0), S( -1, 0), S( 2, 0) },
      { S(-22, 0), S( -7, 0), S(  0, 0), S( 1, 0) },
      { S(-21, 0), S( -7, 0), S(  0, 0), S( 2, 0) },
      { S(-12, 0), S(  4, 0), S(  8, 0), S(12, 0) },
      { S(-23, 0), S(-15, 0), S(-11, 0), S(-5, 0) }
    },
    { // Queen
      { S( 0,-70), S(-3,-57), S(-4,-41), S(-1,-29) },
      { S(-4,-58), S( 6,-30), S( 9,-21), S( 8, -4) },
      { S(-2,-39), S( 6,-17), S( 9, -7), S( 9,  5) },
      { S(-1,-29), S( 8, -5), S(10,  9), S( 7, 17) },
      { S(-3,-27), S( 9, -5), S( 8, 10), S( 7, 23) },
      { S(-2,-40), S( 6,-16), S( 8,-11), S(10,  3) },
      { S(-2,-54), S( 7,-30), S( 7,-21), S( 6, -7) },
      { S(-1,-75), S(-4,-54), S(-1,-44), S( 0,-30) }
    },
    { // King
      { S(260,  0), S(313, 48), S(263, 75), S(188, 84) },
      { S(258, 42), S(298, 91), S(232,142), S(174,131) },
      { S(195, 81), S(240,136), S(171,167), S(105,163) },
      { S(173,103), S(181,166), S(144,166), S(106,176) },
      { S(146,104), S(174,159), S(112,196), S( 63,199) },
      { S(116, 90), S(157,150), S( 82,171), S( 39,169) },
      { S( 85, 44), S(127, 93), S( 62,114), S( 17,133) },
      { S( 63,  2), S( 89, 48), S( 47, 73), S(  0, 83) }
    }
  },
#endif
#ifdef CRAZYHOUSE
  {
    { },
    { // Pawn
      { S(  0, 0), S(  0, 0), S(  0, 0), S( 0, 0) },
      { S(-11, 7), S(  6,-4), S(  7, 8), S( 3,-2) },
      { S(-18,-4), S( -2,-5), S( 19, 5), S(24, 4) },
      { S(-17, 3), S( -9, 3), S( 20,-8), S(35,-3) },
      { S( -6, 8), S(  5, 9), S(  3, 7), S(21,-6) },
      { S( -6, 8), S( -8,-5), S( -6, 2), S(-2, 4) },
      { S( -4, 3), S( 20,-9), S( -8, 1), S(-4,18) }
    },
    { // Knight
      { S(-132,-159), S(-309, -68), S(-117, -37), S( -79,-20) },
      { S( -91,-153), S( -24, -78), S( -80,  -4), S(-129, 62) },
      { S( -28, -18), S( -59, -11), S( -17,  39), S( -10,-50) },
      { S( -17, -56), S(  85, -20), S(  75,  28), S(  95, 20) },
      { S(  91, -53), S(  38, -60), S(  -7,  60), S( 115, 90) },
      { S(  20,  -3), S(  88,-102), S(  33, -71), S(  57,  2) },
      { S(-119, -48), S( -17, -13), S( -15, -56), S(  24,-60) },
      { S(-222,-103), S( -84, -68), S(-126,-112), S(  73, 38) }
    },
    { // Bishop
      { S(-54,-68), S(-23,-40), S(-35,-46), S(-44,-28) },
      { S(-30,-43), S( 10,-17), S(  2,-23), S( -9, -5) },
      { S(-19,-32), S( 17, -9), S( 11,-13), S(  1,  8) },
      { S(-21,-36), S( 18,-13), S( 11,-15), S(  0,  7) },
      { S(-21,-36), S( 14,-14), S(  6,-17), S( -1,  3) },
      { S(-27,-35), S(  6,-13), S(  2,-10), S( -8,  1) },
      { S(-33,-44), S(  7,-21), S( -4,-22), S(-12, -4) },
      { S(-45,-65), S(-21,-42), S(-29,-46), S(-39,-27) }
    },
    { // Rook
      { S(-25, 0), S(-16, 0), S(-16, 0), S(-9, 0) },
      { S(-21, 0), S( -8, 0), S( -3, 0), S( 0, 0) },
      { S(-21, 0), S( -9, 0), S( -4, 0), S( 2, 0) },
      { S(-22, 0), S( -6, 0), S( -1, 0), S( 2, 0) },
      { S(-22, 0), S( -7, 0), S(  0, 0), S( 1, 0) },
      { S(-21, 0), S( -7, 0), S(  0, 0), S( 2, 0) },
      { S(-12, 0), S(  4, 0), S(  8, 0), S(12, 0) },
      { S(-23, 0), S(-15, 0), S(-11, 0), S(-5, 0) }
    },
    { // Queen
      { S( 0,-70), S(-3,-57), S(-4,-41), S(-1,-29) },
      { S(-4,-58), S( 6,-30), S( 9,-21), S( 8, -4) },
      { S(-2,-39), S( 6,-17), S( 9, -7), S( 9,  5) },
      { S(-1,-29), S( 8, -5), S(10,  9), S( 7, 17) },
      { S(-3,-27), S( 9, -5), S( 8, 10), S( 7, 23) },
      { S(-2,-40), S( 6,-16), S( 8,-11), S(10,  3) },
      { S(-2,-54), S( 7,-30), S( 7,-21), S( 6, -7) },
      { S(-1,-75), S(-4,-54), S(-1,-44), S( 0,-30) }
    },
    { // King
      { S(320,212), S(474,410), S(237, 64), S(133, 65) },
      { S(247,107), S(321, 77), S(292, 50), S(191, -4) },
      { S(198, 19), S(281, 79), S(231, 91), S(141, 87) },
      { S(263, 91), S(223,190), S(157,200), S(242,145) },
      { S(177,132), S(205,187), S(143,224), S( 94,227) },
      { S(147,118), S(188,178), S(113,199), S( 70,197) },
      { S(116, 72), S(158,121), S( 93,142), S( 48,161) },
      { S( 94, 30), S(120, 76), S( 78,101), S( 31,111) }
    }
  },
#endif
#ifdef HORDE
  {
    { },
    { // Pawn
      { S(-32,-45), S(-30, -46), S(-23, -20), S( -7,  -7) },
      { S(-27,-27), S( -6, -15), S( -8, -10), S( -7, -12) },
      { S(-59,-21), S( -5,  -9), S(-11,   6), S(  4,   4) },
      { S( -3, -7), S( 18,  13), S(  5,  10), S( 32,   2) },
      { S( 11,  0), S( 15,  18), S( 11,  21), S( 30,  38) },
      { S( 21, 13), S( 12,  20), S( 32,  35), S( 36,  33) },
      { S( 21, 33), S( 32,  33), S( 44,  38), S( 50,  55) },
    },
    { // Knight
      { S(-143, -97), S(-96,-82), S(-80,-46), S(-73,-14) },
      { S( -83, -69), S(-43,-55), S(-21,-17), S(-10,  9) },
      { S( -71, -50), S(-22,-39), S(  0, -8), S(  9, 28) },
      { S( -25, -41), S( 18,-25), S( 43,  7), S( 47, 38) },
      { S( -26, -46), S( 16,-25), S( 38,  2), S( 50, 41) },
      { S( -11, -55), S( 37,-38), S( 56, -8), S( 71, 27) },
      { S( -62, -64), S(-17,-50), S(  5,-24), S( 14, 13) },
      { S(-195,-110), S(-66,-90), S(-42,-50), S(-29,-13) }
    },
    { // Bishop
      { S(-54,-68), S(-23,-40), S(-35,-46), S(-44,-28) },
      { S(-30,-43), S( 10,-17), S(  2,-23), S( -9, -5) },
      { S(-19,-32), S( 17, -9), S( 11,-13), S(  1,  8) },
      { S(-21,-36), S( 18,-13), S( 11,-15), S(  0,  7) },
      { S(-21,-36), S( 14,-14), S(  6,-17), S( -1,  3) },
      { S(-27,-35), S(  6,-13), S(  2,-10), S( -8,  1) },
      { S(-33,-44), S(  7,-21), S( -4,-22), S(-12, -4) },
      { S(-45,-65), S(-21,-42), S(-29,-46), S(-39,-27) }
    },
    { // Rook
      { S(-25, 0), S(-16, 0), S(-16, 0), S(-9, 0) },
      { S(-21, 0), S( -8, 0), S( -3, 0), S( 0, 0) },
      { S(-21, 0), S( -9, 0), S( -4, 0), S( 2, 0) },
      { S(-22, 0), S( -6, 0), S( -1, 0), S( 2, 0) },
      { S(-22, 0), S( -7, 0), S(  0, 0), S( 1, 0) },
      { S(-21, 0), S( -7, 0), S(  0, 0), S( 2, 0) },
      { S(-12, 0), S(  4, 0), S(  8, 0), S(12, 0) },
      { S(-23, 0), S(-15, 0), S(-11, 0), S(-5, 0) }
    },
    { // Queen
      { S( 0,-70), S(-3,-57), S(-4,-41), S(-1,-29) },
      { S(-4,-58), S( 6,-30), S( 9,-21), S( 8, -4) },
      { S(-2,-39), S( 6,-17), S( 9, -7), S( 9,  5) },
      { S(-1,-29), S( 8, -5), S(10,  9), S( 7, 17) },
      { S(-3,-27), S( 9, -5), S( 8, 10), S( 7, 23) },
      { S(-2,-40), S( 6,-16), S( 8,-11), S(10,  3) },
      { S(-2,-54), S( 7,-30), S( 7,-21), S( 6, -7) },
      { S(-1,-75), S(-4,-54), S(-1,-44), S( 0,-30) }
    },
    { // King
      { S(205, 67), S(403, -6), S(394,115), S(518,102) },
      { S(214, 63), S(308,116), S(333,169), S(245,255) },
      { S(277, -2), S(261,141), S(286, 69), S(189,207) },
      { S(280,164), S(298,233), S(126,266), S(-45, 55) },
      { S(134, 89), S(255,225), S(152,245), S( 28,-67) },
      { S(212,156), S(191,171), S( 59,160), S(153,248) },
      { S( 75,-25), S(203,134), S(168,141), S(-76,133) },
      { S(137, 49), S(187,  1), S( 60, 26), S(-25, 56) }
    }
  },
#endif
#ifdef KOTH
  {
    { },
    { // Pawn
      { S(  0, 0), S(  0, 0), S(  0, 0), S( 0, 0) },
      { S(-11, 7), S(  6,-4), S(  7, 8), S( 3,-2) },
      { S(-18,-4), S( -2,-5), S( 19, 5), S(24, 4) },
      { S(-17, 3), S( -9, 3), S( 20,-8), S(35,-3) },
      { S( -6, 8), S(  5, 9), S(  3, 7), S(21,-6) },
      { S( -6, 8), S( -8,-5), S( -6, 2), S(-2, 4) },
      { S( -4, 3), S( 20,-9), S( -8, 1), S(-4,18) }
    },
    { // Knight
      { S(-143, -97), S(-96,-82), S(-80,-46), S(-73,-14) },
      { S( -83, -69), S(-43,-55), S(-21,-17), S(-10,  9) },
      { S( -71, -50), S(-22,-39), S(  0, -8), S(  9, 28) },
      { S( -25, -41), S( 18,-25), S( 43,  7), S( 47, 38) },
      { S( -26, -46), S( 16,-25), S( 38,  2), S( 50, 41) },
      { S( -11, -55), S( 37,-38), S( 56, -8), S( 71, 27) },
      { S( -62, -64), S(-17,-50), S(  5,-24), S( 14, 13) },
      { S(-195,-110), S(-66,-90), S(-42,-50), S(-29,-13) }
    },
    { // Bishop
      { S(-54,-68), S(-23,-40), S(-35,-46), S(-44,-28) },
      { S(-30,-43), S( 10,-17), S(  2,-23), S( -9, -5) },
      { S(-19,-32), S( 17, -9), S( 11,-13), S(  1,  8) },
      { S(-21,-36), S( 18,-13), S( 11,-15), S(  0,  7) },
      { S(-21,-36), S( 14,-14), S(  6,-17), S( -1,  3) },
      { S(-27,-35), S(  6,-13), S(  2,-10), S( -8,  1) },
      { S(-33,-44), S(  7,-21), S( -4,-22), S(-12, -4) },
      { S(-45,-65), S(-21,-42), S(-29,-46), S(-39,-27) }
    },
    { // Rook
      { S(-25, 0), S(-16, 0), S(-16, 0), S(-9, 0) },
      { S(-21, 0), S( -8, 0), S( -3, 0), S( 0, 0) },
      { S(-21, 0), S( -9, 0), S( -4, 0), S( 2, 0) },
      { S(-22, 0), S( -6, 0), S( -1, 0), S( 2, 0) },
      { S(-22, 0), S( -7, 0), S(  0, 0), S( 1, 0) },
      { S(-21, 0), S( -7, 0), S(  0, 0), S( 2, 0) },
      { S(-12, 0), S(  4, 0), S(  8, 0), S(12, 0) },
      { S(-23, 0), S(-15, 0), S(-11, 0), S(-5, 0) }
    },
    { // Queen
      { S( 0,-70), S(-3,-57), S(-4,-41), S(-1,-29) },
      { S(-4,-58), S( 6,-30), S( 9,-21), S( 8, -4) },
      { S(-2,-39), S( 6,-17), S( 9, -7), S( 9,  5) },
      { S(-1,-29), S( 8, -5), S(10,  9), S( 7, 17) },
      { S(-3,-27), S( 9, -5), S( 8, 10), S( 7, 23) },
      { S(-2,-40), S( 6,-16), S( 8,-11), S(10,  3) },
      { S(-2,-54), S( 7,-30), S( 7,-21), S( 6, -7) },
      { S(-1,-75), S(-4,-54), S(-1,-44), S( 0,-30) }
    },
    { // King
      { S(230, 28), S(390,-35), S(397,146), S(352, 83) },
      { S(243, 37), S(274, 74), S(348,229), S(296,393) },
      { S(236, 25), S(305,105), S(255, 95), S(254,188) },
      { S(246,175), S(267,245), S(180,249), S(  0,  0) },
      { S(151,120), S(199,207), S(156,231), S(  0,  0) },
      { S(177,107), S(189,230), S(141,172), S(211,251) },
      { S( 26, 36), S(164,145), S(102,139), S(-42,133) },
      { S(147,  2), S(186,  7), S( 49, 79), S( 48, 50) }
    }
  },
#endif
#ifdef LOSERS
  {
    { },
    { // Pawn
      { S(  0, 0), S(  0, 0), S(  0, 0), S( 0, 0) },
      { S(-11, 7), S(  6,-4), S(  7, 8), S( 3,-2) },
      { S(-18,-4), S( -2,-5), S( 19, 5), S(24, 4) },
      { S(-17, 3), S( -9, 3), S( 20,-8), S(35,-3) },
      { S( -6, 8), S(  5, 9), S(  3, 7), S(21,-6) },
      { S( -6, 8), S( -8,-5), S( -6, 2), S(-2, 4) },
      { S( -4, 3), S( 20,-9), S( -8, 1), S(-4,18) }
    },
    { // Knight
      { S(-143, -97), S(-96,-82), S(-80,-46), S(-73,-14) },
      { S( -83, -69), S(-43,-55), S(-21,-17), S(-10,  9) },
      { S( -71, -50), S(-22,-39), S(  0, -8), S(  9, 28) },
      { S( -25, -41), S( 18,-25), S( 43,  7), S( 47, 38) },
      { S( -26, -46), S( 16,-25), S( 38,  2), S( 50, 41) },
      { S( -11, -55), S( 37,-38), S( 56, -8), S( 71, 27) },
      { S( -62, -64), S(-17,-50), S(  5,-24), S( 14, 13) },
      { S(-195,-110), S(-66,-90), S(-42,-50), S(-29,-13) }
    },
    { // Bishop
      { S(-54,-68), S(-23,-40), S(-35,-46), S(-44,-28) },
      { S(-30,-43), S( 10,-17), S(  2,-23), S( -9, -5) },
      { S(-19,-32), S( 17, -9), S( 11,-13), S(  1,  8) },
      { S(-21,-36), S( 18,-13), S( 11,-15), S(  0,  7) },
      { S(-21,-36), S( 14,-14), S(  6,-17), S( -1,  3) },
      { S(-27,-35), S(  6,-13), S(  2,-10), S( -8,  1) },
      { S(-33,-44), S(  7,-21), S( -4,-22), S(-12, -4) },
      { S(-45,-65), S(-21,-42), S(-29,-46), S(-39,-27) }
    },
    { // Rook
      { S(-25, 0), S(-16, 0), S(-16, 0), S(-9, 0) },
      { S(-21, 0), S( -8, 0), S( -3, 0), S( 0, 0) },
      { S(-21, 0), S( -9, 0), S( -4, 0), S( 2, 0) },
      { S(-22, 0), S( -6, 0), S( -1, 0), S( 2, 0) },
      { S(-22, 0), S( -7, 0), S(  0, 0), S( 1, 0) },
      { S(-21, 0), S( -7, 0), S(  0, 0), S( 2, 0) },
      { S(-12, 0), S(  4, 0), S(  8, 0), S(12, 0) },
      { S(-23, 0), S(-15, 0), S(-11, 0), S(-5, 0) }
    },
    { // Queen
      { S( 0,-70), S(-3,-57), S(-4,-41), S(-1,-29) },
      { S(-4,-58), S( 6,-30), S( 9,-21), S( 8, -4) },
      { S(-2,-39), S( 6,-17), S( 9, -7), S( 9,  5) },
      { S(-1,-29), S( 8, -5), S(10,  9), S( 7, 17) },
      { S(-3,-27), S( 9, -5), S( 8, 10), S( 7, 23) },
      { S(-2,-40), S( 6,-16), S( 8,-11), S(10,  3) },
      { S(-2,-54), S( 7,-30), S( 7,-21), S( 6, -7) },
      { S(-1,-75), S(-4,-54), S(-1,-44), S( 0,-30) }
    },
    { // King
      { S(260,  0), S(313, 48), S(263, 75), S(188, 84) },
      { S(258, 42), S(298, 91), S(232,142), S(174,131) },
      { S(195, 81), S(240,136), S(171,167), S(105,163) },
      { S(173,103), S(181,166), S(144,166), S(106,176) },
      { S(146,104), S(174,159), S(112,196), S( 63,199) },
      { S(116, 90), S(157,150), S( 82,171), S( 39,169) },
      { S( 85, 44), S(127, 93), S( 62,114), S( 17,133) },
      { S( 63,  2), S( 89, 48), S( 47, 73), S(  0, 83) }
    }
  },
#endif
#ifdef RACE
  {
    { },
    { // Pawn
    },
    { // Knight
      { S(-237,-120), S(-56,-154), S(-138,-162), S(-100,-91) },
      { S(-104, -73), S(-27, -69), S(  19, -27), S( -30,-54) },
      { S(  51, -45), S( 29,  -1), S(  58, -25), S( 105, 35) },
      { S(  20,  27), S( 54, -27), S(  77,  59), S(125, 132) },
      { S(  32,  29), S( 75,  -2), S(  95, 123), S(130, 132) },
      { S(  16,  21), S( 31,  86), S( 108,  74), S(135, 142) },
      { S( -17,   0), S( 40,  51), S(  41, 116), S(  8,  74) },
      { S( -22, -42), S(-11, -22), S( -34,  47), S( 12,  97) }
    },
    { // Bishop
      { S(-45,-65), S(-21,-42), S(-29,-46), S(-39,-27) },
      { S(-23,-34), S( 17,-11), S(  6,-12), S( -2,  6) },
      { S( -7,-15), S( 26,  7), S( 22, 10), S( 12, 21) },
      { S(  9, -6), S( 44, 16), S( 36, 13), S( 29, 33) },
      { S( 19,  4), S( 58, 27), S( 51, 25), S( 40, 47) },
      { S( 31, 18), S( 67, 41), S( 61, 37), S( 51, 58) },
      { S( 30, 17), S( 70, 43), S( 62, 37), S( 51, 55) },
      { S( 16,  2), S( 47, 30), S( 35, 24), S( 26, 42) }
    },
    { // Rook
      { S(-23, 0), S(-15,  0), S(-11,  0), S( -5,  0) },
      { S(-2, 10), S( 14, 10), S( 18, 10), S( 22, 10) },
      { S(-1, 20), S( 13, 20), S( 20, 20), S( 22, 20) },
      { S( 8, 30), S( 23, 30), S( 30, 30), S( 31, 30) },
      { S(18, 40), S( 34, 40), S( 39, 40), S( 42, 40) },
      { S(29, 50), S( 41, 50), S( 46, 50), S( 52, 50) },
      { S(39, 60), S( 52, 60), S( 57, 60), S( 60, 60) },
      { S(45, 70), S( 54, 70), S( 54, 70), S( 61, 70) }
    },
    { // Queen
      { S(-1,-75), S(-4,-54), S(-1,-44), S( 0,-30) },
      { S( 8,-44), S(17,-20), S(17,-11), S(16,  3) },
      { S(18,-20), S(26,  4), S(28,  9), S(30, 23) },
      { S(27,  3), S(39, 25), S(38, 40), S(37, 53) },
      { S(39, 11), S(48, 35), S(50, 49), S(47, 57) },
      { S(48, 11), S(56, 33), S(59, 43), S(59, 55) },
      { S(56,  2), S(66, 30), S(69, 39), S(68, 56) },
      { S(70,  0), S(67, 13), S(66, 29), S(69, 41) }
    },
    { // King
      { S( 63,   2), S( 89, 48), S( 47, 73), S(  0, 83) },
      { S( 95,  54), S(137,103), S( 72,124), S( 27,143) },
      { S(136, 110), S(177,170), S(102,191), S( 59,189) },
      { S(176, 134), S(204,189), S(142,226), S( 93,229) },
      { S(213, 143), S(221,206), S(184,206), S(146,216) },
      { S(245, 131), S(290,186), S(221,217), S(155,213) },
      { S(318, 102), S(358,151), S(292,202), S(234,191) },
      { S(330,  70), S(383,118), S(333,145), S(258,154) }
    }
  },
#endif
#ifdef RELAY
  {
    { },
    { // Pawn
      { S(  0, 0), S(  0, 0), S(  0, 0), S( 0, 0) },
      { S(-11, 7), S(  6,-4), S(  7, 8), S( 3,-2) },
      { S(-18,-4), S( -2,-5), S( 19, 5), S(24, 4) },
      { S(-17, 3), S( -9, 3), S( 20,-8), S(35,-3) },
      { S( -6, 8), S(  5, 9), S(  3, 7), S(21,-6) },
      { S( -6, 8), S( -8,-5), S( -6, 2), S(-2, 4) },
      { S( -4, 3), S( 20,-9), S( -8, 1), S(-4,18) }
    },
    { // Knight
      { S(-143, -97), S(-96,-82), S(-80,-46), S(-73,-14) },
      { S( -83, -69), S(-43,-55), S(-21,-17), S(-10,  9) },
      { S( -71, -50), S(-22,-39), S(  0, -8), S(  9, 28) },
      { S( -25, -41), S( 18,-25), S( 43,  7), S( 47, 38) },
      { S( -26, -46), S( 16,-25), S( 38,  2), S( 50, 41) },
      { S( -11, -55), S( 37,-38), S( 56, -8), S( 71, 27) },
      { S( -62, -64), S(-17,-50), S(  5,-24), S( 14, 13) },
      { S(-195,-110), S(-66,-90), S(-42,-50), S(-29,-13) }
    },
    { // Bishop
      { S(-54,-68), S(-23,-40), S(-35,-46), S(-44,-28) },
      { S(-30,-43), S( 10,-17), S(  2,-23), S( -9, -5) },
      { S(-19,-32), S( 17, -9), S( 11,-13), S(  1,  8) },
      { S(-21,-36), S( 18,-13), S( 11,-15), S(  0,  7) },
      { S(-21,-36), S( 14,-14), S(  6,-17), S( -1,  3) },
      { S(-27,-35), S(  6,-13), S(  2,-10), S( -8,  1) },
      { S(-33,-44), S(  7,-21), S( -4,-22), S(-12, -4) },
      { S(-45,-65), S(-21,-42), S(-29,-46), S(-39,-27) }
    },
    { // Rook
      { S(-25, 0), S(-16, 0), S(-16, 0), S(-9, 0) },
      { S(-21, 0), S( -8, 0), S( -3, 0), S( 0, 0) },
      { S(-21, 0), S( -9, 0), S( -4, 0), S( 2, 0) },
      { S(-22, 0), S( -6, 0), S( -1, 0), S( 2, 0) },
      { S(-22, 0), S( -7, 0), S(  0, 0), S( 1, 0) },
      { S(-21, 0), S( -7, 0), S(  0, 0), S( 2, 0) },
      { S(-12, 0), S(  4, 0), S(  8, 0), S(12, 0) },
      { S(-23, 0), S(-15, 0), S(-11, 0), S(-5, 0) }
    },
    { // Queen
      { S( 0,-70), S(-3,-57), S(-4,-41), S(-1,-29) },
      { S(-4,-58), S( 6,-30), S( 9,-21), S( 8, -4) },
      { S(-2,-39), S( 6,-17), S( 9, -7), S( 9,  5) },
      { S(-1,-29), S( 8, -5), S(10,  9), S( 7, 17) },
      { S(-3,-27), S( 9, -5), S( 8, 10), S( 7, 23) },
      { S(-2,-40), S( 6,-16), S( 8,-11), S(10,  3) },
      { S(-2,-54), S( 7,-30), S( 7,-21), S( 6, -7) },
      { S(-1,-75), S(-4,-54), S(-1,-44), S( 0,-30) }
    },
    { // King
      { S(260,  0), S(313, 48), S(263, 75), S(188, 84) },
      { S(258, 42), S(298, 91), S(232,142), S(174,131) },
      { S(195, 81), S(240,136), S(171,167), S(105,163) },
      { S(173,103), S(181,166), S(144,166), S(106,176) },
      { S(146,104), S(174,159), S(112,196), S( 63,199) },
      { S(116, 90), S(157,150), S( 82,171), S( 39,169) },
      { S( 85, 44), S(127, 93), S( 62,114), S( 17,133) },
      { S( 63,  2), S( 89, 48), S( 47, 73), S(  0, 83) }
    }
  },
#endif
#ifdef THREECHECK
  {
    { },
    { // Pawn
      { S(  0, 0), S(  0, 0), S(  0, 0), S( 0, 0) },
      { S(-11, 7), S(  6,-4), S(  7, 8), S( 3,-2) },
      { S(-18,-4), S( -2,-5), S( 19, 5), S(24, 4) },
      { S(-17, 3), S( -9, 3), S( 20,-8), S(35,-3) },
      { S( -6, 8), S(  5, 9), S(  3, 7), S(21,-6) },
      { S( -6, 8), S( -8,-5), S( -6, 2), S(-2, 4) },
      { S( -4, 3), S( 20,-9), S( -8, 1), S(-4,18) }
    },
    { // Knight
      { S(-143, -97), S(-96,-82), S(-80,-46), S(-73,-14) },
      { S( -83, -69), S(-43,-55), S(-21,-17), S(-10,  9) },
      { S( -71, -50), S(-22,-39), S(  0, -8), S(  9, 28) },
      { S( -25, -41), S( 18,-25), S( 43,  7), S( 47, 38) },
      { S( -26, -46), S( 16,-25), S( 38,  2), S( 50, 41) },
      { S( -11, -55), S( 37,-38), S( 56, -8), S( 71, 27) },
      { S( -62, -64), S(-17,-50), S(  5,-24), S( 14, 13) },
      { S(-195,-110), S(-66,-90), S(-42,-50), S(-29,-13) }
    },
    { // Bishop
      { S(-54,-68), S(-23,-40), S(-35,-46), S(-44,-28) },
      { S(-30,-43), S( 10,-17), S(  2,-23), S( -9, -5) },
      { S(-19,-32), S( 17, -9), S( 11,-13), S(  1,  8) },
      { S(-21,-36), S( 18,-13), S( 11,-15), S(  0,  7) },
      { S(-21,-36), S( 14,-14), S(  6,-17), S( -1,  3) },
      { S(-27,-35), S(  6,-13), S(  2,-10), S( -8,  1) },
      { S(-33,-44), S(  7,-21), S( -4,-22), S(-12, -4) },
      { S(-45,-65), S(-21,-42), S(-29,-46), S(-39,-27) }
    },
    { // Rook
      { S(-25, 0), S(-16, 0), S(-16, 0), S(-9, 0) },
      { S(-21, 0), S( -8, 0), S( -3, 0), S( 0, 0) },
      { S(-21, 0), S( -9, 0), S( -4, 0), S( 2, 0) },
      { S(-22, 0), S( -6, 0), S( -1, 0), S( 2, 0) },
      { S(-22, 0), S( -7, 0), S(  0, 0), S( 1, 0) },
      { S(-21, 0), S( -7, 0), S(  0, 0), S( 2, 0) },
      { S(-12, 0), S(  4, 0), S(  8, 0), S(12, 0) },
      { S(-23, 0), S(-15, 0), S(-11, 0), S(-5, 0) }
    },
    { // Queen
      { S( 0,-70), S(-3,-57), S(-4,-41), S(-1,-29) },
      { S(-4,-58), S( 6,-30), S( 9,-21), S( 8, -4) },
      { S(-2,-39), S( 6,-17), S( 9, -7), S( 9,  5) },
      { S(-1,-29), S( 8, -5), S(10,  9), S( 7, 17) },
      { S(-3,-27), S( 9, -5), S( 8, 10), S( 7, 23) },
      { S(-2,-40), S( 6,-16), S( 8,-11), S(10,  3) },
      { S(-2,-54), S( 7,-30), S( 7,-21), S( 6, -7) },
      { S(-1,-75), S(-4,-54), S(-1,-44), S( 0,-30) }
    },
    { // King
      { S(260,  0), S(313, 48), S(263, 75), S(188, 84) },
      { S(258, 42), S(298, 91), S(232,142), S(174,131) },
      { S(195, 81), S(240,136), S(171,167), S(105,163) },
      { S(173,103), S(181,166), S(144,166), S(106,176) },
      { S(146,104), S(174,159), S(112,196), S( 63,199) },
      { S(116, 90), S(157,150), S( 82,171), S( 39,169) },
      { S( 85, 44), S(127, 93), S( 62,114), S( 17,133) },
      { S( 63,  2), S( 89, 48), S( 47, 73), S(  0, 83) }
    }
  },
#endif
};

#ifdef CRAZYHOUSE
Score psq[VARIANT_NB][PIECE_NB][SQUARE_NB+1];
const Score inHandBonus[PIECE_TYPE_NB] = {
    S(0, 0), S(52, 12), S(56, 32), S(4, 4), S(13, 3), S(25, 10)
};
#else
Score psq[VARIANT_NB][PIECE_NB][SQUARE_NB];
#endif

#undef S

// init() initializes piece-square tables: the white halves of the tables are
// copied from Bonus[] adding the piece value, then the black halves of the
// tables are initialized by flipping and changing the sign of the white scores.
void init() {

  for (Variant var = CHESS_VARIANT; var < VARIANT_NB; ++var)
      for (Piece pc = W_PAWN; pc <= W_KING; ++pc)
      {
          PieceValue[var][MG][~pc] = PieceValue[var][MG][pc];
          PieceValue[var][EG][~pc] = PieceValue[var][EG][pc];

          Score v = make_score(PieceValue[var][MG][pc], PieceValue[var][EG][pc]);

          for (Square s = SQ_A1; s <= SQ_H8; ++s)
          {
              File f = std::min(file_of(s), FILE_H - file_of(s));
              psq[var][ pc][ s] = v + Bonus[var][pc][rank_of(s)][f];
#ifdef RACE
              if (var == RACE_VARIANT)
                  psq[var][~pc][horizontal_flip(s)] = -psq[var][pc][s];
              else
#endif
              psq[var][~pc][~s] = -psq[var][pc][s];
          }
#ifdef CRAZYHOUSE
          psq[var][ pc][SQ_NONE] = v + inHandBonus[type_of(pc)];
          psq[var][~pc][SQ_NONE] = -psq[var][pc][SQ_NONE];
#endif
      }
}
TUNE(PieceValue[THREECHECK_VARIANT], init);

} // namespace PSQT
