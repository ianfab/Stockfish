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

#include <cassert>

#include "movepick.h"
#include "thread.h"

namespace {

  enum Stages {
    MAIN_SEARCH, CAPTURES_INIT, GOOD_CAPTURES, KILLERS, COUNTERMOVE, QUIET_INIT, QUIET, BAD_CAPTURES,
    EVASION, EVASIONS_INIT, ALL_EVASIONS,
    PROBCUT, PROBCUT_INIT, PROBCUT_CAPTURES,
    QSEARCH_WITH_CHECKS, QCAPTURES_1_INIT, QCAPTURES_1, QCHECKS,
    QSEARCH_NO_CHECKS, QCAPTURES_2_INIT, QCAPTURES_2,
    QSEARCH_RECAPTURES, QRECAPTURES
  };

  // partial_insertion_sort() sorts moves in descending order up to and including
  // a given limit. The order of moves smaller than the limit is left unspecified.
  void partial_insertion_sort(ExtMove* begin, ExtMove* end, int limit) {

    for (ExtMove *sortedEnd = begin, *p = begin + 1; p < end; ++p)
        if (p->value >= limit)
        {
            ExtMove tmp = *p, *q;
            *p = *++sortedEnd;
            for (q = sortedEnd; q != begin && *(q - 1) < tmp; --q)
                *q = *(q - 1);
            *q = tmp;
        }
  }

  // pick_best() finds the best move in the range (begin, end) and moves it to
  // the front. It's faster than sorting all the moves in advance when there
  // are few moves, e.g., the possible captures.
  Move pick_best(ExtMove* begin, ExtMove* end) {

    std::swap(*begin, *std::max_element(begin, end));
    return *begin;
  }

} // namespace


/// Constructors of the MovePicker class. As arguments we pass information
/// to help it to return the (presumably) good moves first, to decide which
/// moves to return (in the quiescence search, for instance, we only want to
/// search captures, promotions, and some checks) and how important good move
/// ordering is at the current node.

MovePicker::MovePicker(const Position& p, Move ttm, Depth d, Search::Stack* s)
           : pos(p), ss(s), depth(d) {

  assert(d > DEPTH_ZERO);

  Square prevSq = to_sq((ss-1)->currentMove);
  countermove = pos.this_thread()->counterMoves[pos.piece_on(prevSq)][prevSq];
  killers[0] = ss->killers[0];
  killers[1] = ss->killers[1];

  stage = pos.checkers() ? EVASION : MAIN_SEARCH;
  ttMove = ttm && pos.pseudo_legal(ttm) ? ttm : MOVE_NONE;
  stage += (ttMove == MOVE_NONE);
}

MovePicker::MovePicker(const Position& p, Move ttm, Depth d, Square s)
           : pos(p) {

  assert(d <= DEPTH_ZERO);

  if (pos.checkers())
      stage = EVASION;

  else if (d > DEPTH_QS_NO_CHECKS)
      stage = QSEARCH_WITH_CHECKS;

  else if (d > DEPTH_QS_RECAPTURES)
      stage = QSEARCH_NO_CHECKS;

  else
  {
      stage = QSEARCH_RECAPTURES;
      recaptureSquare = s;
      return;
  }

  ttMove = ttm && pos.pseudo_legal(ttm) ? ttm : MOVE_NONE;
  stage += (ttMove == MOVE_NONE);
}

MovePicker::MovePicker(const Position& p, Move ttm, Value th)
           : pos(p), threshold(th) {

  assert(!pos.checkers());

  stage = PROBCUT;

  // In ProbCut we generate captures with SEE higher than or equal to the given threshold
  ttMove =   ttm
          && pos.pseudo_legal(ttm)
          && pos.capture(ttm)
          && pos.see_ge(ttm, threshold) ? ttm : MOVE_NONE;

  stage += (ttMove == MOVE_NONE);
}


/// score() assigns a numerical value to each move in a move list. The moves with
/// highest values will be picked first.
template<>
void MovePicker::score<CAPTURES>() {
  // Winning and equal captures in the main search are ordered by MVV, preferring
  // captures near our home rank. Surprisingly, this appears to perform slightly
  // better than SEE-based move ordering: exchanging big pieces before capturing
  // a hanging piece probably helps to reduce the subtree size.
  // In the main search we want to push captures with negative SEE values to the
  // badCaptures[] array, but instead of doing it now we delay until the move
  // has been picked up, saving some SEE calls in case we get a cutoff.
  for (auto& m : *this)
#ifdef ANTI
      if (pos.is_anti())
          m.value = PieceValue[pos.variant()][MG][pos.piece_on(to_sq(m))]
                   - Value(50 * relative_rank(pos.side_to_move(), to_sq(m)));
      else
#endif
#ifdef ATOMIC
      if (pos.is_atomic())
          m.value = pos.see<ATOMIC_VARIANT>(m)
                   - Value(200 * std::min(distance(to_sq(m), pos.square<KING>(~pos.side_to_move())),
                                          distance(to_sq(m), pos.square<KING>(pos.side_to_move()))));
      else
#endif
#ifdef CRAZYHOUSE
      if (pos.is_house())
          m.value = PieceValue[pos.variant()][MG][pos.piece_on(to_sq(m))]
                   - Value(200 * std::min(distance(to_sq(m), pos.square<KING>(~pos.side_to_move())),
                                          distance(to_sq(m), pos.square<KING>(pos.side_to_move()))));
      else
#endif
#ifdef RACE
      if (pos.is_race())
          m.value = PieceValue[pos.variant()][MG][pos.piece_on(to_sq(m))]
                   - Value(200 * relative_rank(BLACK, to_sq(m)));
      else
#endif
      m.value =  PieceValue[pos.variant()][MG][pos.piece_on(to_sq(m))]
               - Value(200 * relative_rank(pos.side_to_move(), to_sq(m)));
}

template<>
void MovePicker::score<QUIETS>() {

  const ButterflyHistory& history = pos.this_thread()->history;

  const PieceToHistory& cmh = *(ss-1)->history;
  const PieceToHistory& fmh = *(ss-2)->history;
  const PieceToHistory& fm2 = *(ss-4)->history;

  Color c = pos.side_to_move();

  for (auto& m : *this)
  {
      m.value =  cmh[pos.moved_piece(m)][to_sq(m)]
               + fmh[pos.moved_piece(m)][to_sq(m)]
               + fm2[pos.moved_piece(m)][to_sq(m)]
               + history[c][from_to(m)];
#ifdef ANTI
      if (pos.is_anti() && pos.attackers_to(to_sq(m), pos.pieces() ^ from_sq(m)) & pos.pieces(~c))
      {
          m.value += (1 << 28);
          if (!(pos.attackers_to(from_sq(m)) & pos.pieces(~c)))
              m.value += (1 << 27);
      }
#endif
  }
}

template<>
void MovePicker::score<EVASIONS>() {
  // Try captures ordered by MVV/LVA, then non-captures ordered by stats heuristics
  const ButterflyHistory& history = pos.this_thread()->history;
  Color c = pos.side_to_move();

  for (auto& m : *this)
      if (pos.capture(m))
          m.value =  PieceValue[pos.variant()][MG][pos.piece_on(to_sq(m))]
                   - Value(type_of(pos.moved_piece(m))) + (1 << 28);
      else
          m.value = history[c][from_to(m)];
}


/// next_move() is the most important method of the MovePicker class. It returns
/// a new pseudo legal move every time it is called, until there are no more moves
/// left. It picks the move with the biggest value from a list of generated moves
/// taking care not to return the ttMove if it has already been searched.

Move MovePicker::next_move(bool skipQuiets) {

  Move move;

  switch (stage) {

  case MAIN_SEARCH: case EVASION: case QSEARCH_WITH_CHECKS:
  case QSEARCH_NO_CHECKS: case PROBCUT:
      ++stage;
      return ttMove;

  case CAPTURES_INIT:
      endBadCaptures = cur = moves;
      endMoves = generate<CAPTURES>(pos, cur);
      score<CAPTURES>();
      ++stage;
      /* fallthrough */

  case GOOD_CAPTURES:
      while (cur < endMoves)
      {
          move = pick_best(cur++, endMoves);
          if (move != ttMove)
          {
              if (pos.see_ge(move))
                  return move;

              // Losing capture, move it to the beginning of the array
              *endBadCaptures++ = move;
          }
      }

      ++stage;
      move = killers[0];  // First killer move
      if (    move != MOVE_NONE
          &&  move != ttMove
          &&  pos.pseudo_legal(move)
          && !pos.capture(move))
          return move;
      /* fallthrough */

  case KILLERS:
      ++stage;
      move = killers[1]; // Second killer move
      if (    move != MOVE_NONE
          &&  move != ttMove
          &&  pos.pseudo_legal(move)
          && !pos.capture(move))
          return move;
      /* fallthrough */

  case COUNTERMOVE:
      ++stage;
      move = countermove;
      if (    move != MOVE_NONE
          &&  move != ttMove
          &&  move != killers[0]
          &&  move != killers[1]
          &&  pos.pseudo_legal(move)
          && !pos.capture(move))
          return move;
      /* fallthrough */

  case QUIET_INIT:
      cur = endBadCaptures;
      endMoves = generate<QUIETS>(pos, cur);
      score<QUIETS>();
      partial_insertion_sort(cur, endMoves, -4000 * depth / ONE_PLY);
      ++stage;
      /* fallthrough */

  case QUIET:
      while (    cur < endMoves
             && (!skipQuiets || cur->value >= VALUE_ZERO))
      {
          move = *cur++;

          if (   move != ttMove
              && move != killers[0]
              && move != killers[1]
              && move != countermove)
              return move;
      }
      ++stage;
      cur = moves; // Point to beginning of bad captures
      /* fallthrough */

  case BAD_CAPTURES:
      if (cur < endBadCaptures)
          return *cur++;
      break;

  case EVASIONS_INIT:
      cur = moves;
      endMoves = generate<EVASIONS>(pos, cur);
      score<EVASIONS>();
      ++stage;
      /* fallthrough */

  case ALL_EVASIONS:
      while (cur < endMoves)
      {
          move = pick_best(cur++, endMoves);
          if (move != ttMove)
              return move;
      }
      break;

  case PROBCUT_INIT:
      cur = moves;
      endMoves = generate<CAPTURES>(pos, cur);
      score<CAPTURES>();
      ++stage;
      /* fallthrough */

  case PROBCUT_CAPTURES:
      while (cur < endMoves)
      {
          move = pick_best(cur++, endMoves);
          if (   move != ttMove
              && pos.see_ge(move, threshold))
              return move;
      }
      break;

  case QCAPTURES_1_INIT: case QCAPTURES_2_INIT:
      cur = moves;
      endMoves = generate<CAPTURES>(pos, cur);
      score<CAPTURES>();
      ++stage;
      /* fallthrough */

  case QCAPTURES_1: case QCAPTURES_2:
      while (cur < endMoves)
      {
          move = pick_best(cur++, endMoves);
          if (move != ttMove)
              return move;
      }
      if (stage == QCAPTURES_2)
          break;
      cur = moves;
      endMoves = generate<QUIET_CHECKS>(pos, cur);
      ++stage;
      /* fallthrough */

  case QCHECKS:
      while (cur < endMoves)
      {
          move = cur++->move;
          if (move != ttMove)
              return move;
      }
      break;

  case QSEARCH_RECAPTURES:
      cur = moves;
      endMoves = generate<CAPTURES>(pos, cur);
      score<CAPTURES>();
      ++stage;
      /* fallthrough */

  case QRECAPTURES:
      while (cur < endMoves)
      {
          move = pick_best(cur++, endMoves);
          if (to_sq(move) == recaptureSquare)
              return move;
      }
      break;

  default:
      assert(false);
  }

  return MOVE_NONE;
}
