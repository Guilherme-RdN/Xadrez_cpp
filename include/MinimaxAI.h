#pragma once
#include "Board.h"
#include "Evaluator.h"
#include <memory>

class MinimaxAI {
public:
    MinimaxAI(int depth, std::unique_ptr<Evaluator> evaluator);

    Move chooseMove(Board& board, Color side, bool& found);
    long nodesVisited() const { return nodes; }
    int depth() const { return depth_; }
    const char* evaluatorName() const { return evaluator->name(); }

private:
    int depth_;
    std::unique_ptr<Evaluator> evaluator;
    long nodes = 0;

    int minimax(Board& board, int depth, int alpha, int beta, bool maximizing, Color rootSide);
};
