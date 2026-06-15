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

    // Modo Facil: filtra lancamentos que revisitam posicoes ja vistas
    // (chave simplificada: so pecas + lado, sem roque/en passant).
    // Isso impede vai-e-vem mesmo quando os direitos de roque mudaram.
    // O filtro so remove; se todos os lancamentos forem repeticao, mantemos todos.
    if (avoidRepetitions_) {
        std::vector<Move> fresh;
        for (const Move& m : moves) {
            board.makeMove(m);
            bool repeated = board.countSimpleRepetitions() >= 2;
            board.undoMove(m);
            if (!repeated) fresh.push_back(m);
        }
        if (!fresh.empty()) moves = fresh;
    }

    for (const Move& m : moves) {
        board.makeMove(m);
        int score = minimax(board, depth_ - 1, alpha, beta, false, side);
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
