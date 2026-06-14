#pragma once
#include "Board.h"
#include <vector>

class GameState {
public:
    static bool isSquareAttacked(const Board& board, int row, int col, Color attacker);
    static bool isInCheck(const Board& board, Color side);
    static std::vector<Move> generateLegalMoves(Board& board, Color side);
    static bool isCheckmate(Board& board, Color side);
    static bool isStalemate(Board& board, Color side);
};
