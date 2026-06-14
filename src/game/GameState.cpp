#include "GameState.h"

bool GameState::isSquareAttacked(const Board& board, int row, int col, Color attacker) {
    for (const Move& m : board.generatePseudoLegalMoves(attacker))
        if (m.toRow == row && m.toCol == col && m.type != MoveType::Castle)
            return true;
    return false;
}

bool GameState::isInCheck(const Board& board, Color side) {
    int kr, kc;
    if (!board.findKing(side, kr, kc)) return false;
    return isSquareAttacked(board, kr, kc, opponent(side));
}

std::vector<Move> GameState::generateLegalMoves(Board& board, Color side) {
    std::vector<Move> legal;
    Color enemy = opponent(side);

    for (const Move& m : board.generatePseudoLegalMoves(side)) {
        if (m.type == MoveType::Castle) {
            if (isInCheck(board, side)) continue;
            int pass = (m.toCol == 6) ? 5 : 3;
            if (isSquareAttacked(board, m.fromRow, pass, enemy)) continue;
        }

        board.makeMove(m);
        bool safe = !isInCheck(board, side);
        board.undoMove(m);

        if (safe) legal.push_back(m);
    }
    return legal;
}

bool GameState::isCheckmate(Board& board, Color side) {
    return isInCheck(board, side) && generateLegalMoves(board, side).empty();
}

bool GameState::isStalemate(Board& board, Color side) {
    return !isInCheck(board, side) && generateLegalMoves(board, side).empty();
}
