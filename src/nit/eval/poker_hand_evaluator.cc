/**
 * Copyright (c) 2012 Andrew Prock. All rights reserved.
 */
#include "poker_hand_evaluator.h"

namespace nit {

PokerHandEvaluator::PokerHandEvaluator()
    : m_subclassID("abstract-base-class") {}

PokerHandEvaluator::~PokerHandEvaluator() = default;

static double INV_LUT[] = {0,       1 / 1.0, 1 / 2.0, 1 / 3.0, 1 / 4.0, 1 / 5.0,
                           1 / 6.0, 1 / 7.0, 1 / 8.0, 1 / 9.0, 1 / 10.0};

void PokerHandEvaluator::evaluateShowdown(
    const std::vector<CardSet>& hands, const CardSet& board,
    std::vector<PokerHandEvaluation>& evals, std::vector<EquityResult>& result,
    double weight) const {
  // this is a special trick we use.  the hands vector could actually
  // contain hands [0..n],board because of the way we step through the
  // ParitionEnumerator, however, the size evals vector *must* be equal to
  // the number of hands, board or not.  So we use the size of the evals
  // here, not the size of the hand vector
  size_t hsize = evals.size();
  size_t nevals = 1;

  // gather all the evaluations
  for (size_t i = 0; i < hsize; i++) {
    // we track whether or not an eval is used in the nevals
    // variable to avoid looping through the low half of split
    // pot games when no one has a low.  This only covers games
    // which have one or two pots.
    evals[i] = evaluateHand(hands[i], board);
    if (nevals == 1 && evals[i].eval(1) > PokerEvaluation(0))
      nevals = 2;
  }

  // award share(s)
  for (size_t e = 0; e < nevals; e++) {
    // find the best eval, and adjust shares if there are ties
    PokerEvaluation maxeval = evals[0].eval(e);
    size_t winner = 0;
    size_t shares = 1;
    for (size_t i = 1; i < hsize; i++) {
      PokerEvaluation eval = evals[i].eval(e);
      if (eval > maxeval) {
        shares = 1;
        maxeval = eval;
        winner = i;
      } else if (eval == maxeval) {
        shares++;
      }
    }
    // award shares to the winner, or...
    if (shares == 1) {
      result[winner].winShares += INV_LUT[nevals] * weight;
    }
    // award shares to those who tie
    else {
      for (size_t i = 0; i < hsize; i++)
        if (evals[i].eval(e) == maxeval)
          result[i].tieShares += INV_LUT[shares * nevals] * weight;
    }
  }
}

}  // namespace nit
