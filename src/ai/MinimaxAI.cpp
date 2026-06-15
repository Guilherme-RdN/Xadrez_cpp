#include "MinimaxAI.h"
#include "GameState.h"
#include <algorithm>
#include <utility>

static const int INF = 1000000;
static const int MATE = 100000;

MinimaxAI::MinimaxAI(int depth, std::unique_ptr<Evaluator> evaluator, bool avoidRepetitions)
    : depth_(depth), evaluator(std::move(evaluator)), avoidRepetitions_(avoidRepetitions) {}

int MinimaxAI::minimax(Board& board, int depth, int alpha, int beta, bool maximizing, Color rootSide) {
    nodes++;

    Color side = maximizing ? rootSide : opponent(rootSide);
    std::vector<Move> moves = GameState::generateLegalMoves(board, side);

    if (moves.empty()) {
        if (GameState::isInCheck(board, side))
            return maximizing ? (-MATE - depth) : (MATE + depth);
        return 0;
    }

    if (depth == 0)
        return evaluator->evaluate(board, rootSide);

    std::sort(moves.begin(), moves.end(), [&board](const Move& a, const Move& b) {
        return !board.getPiece(a.toRow, a.toCol).isEmpty() >
               !board.getPiece(b.toRow, b.toCol).isEmpty();
    });

    if (maximizing) {
        int best = -INF;
        for (const Move& m : moves) {
            board.makeMove(m);
            best = std::max(best, minimax(board, depth - 1, alpha, beta, false, rootSide));
            board.undoMove(m);
            alpha = std::max(alpha, best);
            if (beta <= alpha) break;
        }
        return best;
    } else {
        int best = INF;
        for (const Move& m : moves) {
            board.makeMove(m);
            best = std::min(best, minimax(board, depth - 1, alpha, beta, true, rootSide));
            board.undoMove(m);
            beta = std::min(beta, best);
            if (beta <= alpha) break;
        }
        return best;
    }
}

Move MinimaxAI::chooseMove(Board& board, Color side, bool& found) {
    nodes = 0;
    std::vector<Move> moves = GameState::generateLegalMoves(board, side);

    if (moves.empty()) {
        found = false;
        return Move{};
    }

    Move bestMove = moves.front();
    int bestScore = -INF;
    int alpha = -INF;
    int beta = INF;

    for (const Move& m : moves) {
        board.makeMove(m);
        int score = minimax(board, depth_ - 1, alpha, beta, false, side);
        // No modo Facil, penaliza movimentos que levam a posicoes ja vistas
        // (apareceram >= 1 vez antes) para evitar que a IA fique repetindo
        if (avoidRepetitions_ && board.countRepetitions() >= 2)
            score -= 200;
        board.undoMove(m);
        if (score > bestScore) {
            bestScore = score;
            bestMove = m;
        }
        alpha = std::max(alpha, bestScore);
    }

    found = true;
    return bestMove;
}
