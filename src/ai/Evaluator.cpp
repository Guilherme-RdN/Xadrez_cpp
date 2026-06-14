#include "Evaluator.h"

static int pieceValue(PieceType type) {
    switch (type) {
        case PieceType::PAWN:   return 100;
        case PieceType::KNIGHT: return 320;
        case PieceType::BISHOP: return 330;
        case PieceType::ROOK:   return 500;
        case PieceType::QUEEN:  return 900;
        case PieceType::KING:   return 20000;
        default:                return 0;
    }
}

int MaterialEvaluator::evaluate(const Board& board, Color side) const {
    int score = 0;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            Piece p = board.getPiece(r, c);
            if (p.isEmpty()) continue;
            int v = pieceValue(p.type);
            score += (p.color == side) ? v : -v;
        }
    return score;
}

static const int knightTable[8][8] = {
    {-50, -40, -30, -30, -30, -30, -40, -50},
    {-40, -20,   0,   0,   0,   0, -20, -40},
    {-30,   0,  10,  15,  15,  10,   0, -30},
    {-30,   5,  15,  20,  20,  15,   5, -30},
    {-30,   0,  15,  20,  20,  15,   0, -30},
    {-30,   5,  10,  15,  15,  10,   5, -30},
    {-40, -20,   0,   5,   5,   0, -20, -40},
    {-50, -40, -30, -30, -30, -30, -40, -50}
};

static const int pawnTable[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0},
    { 50, 50, 50, 50, 50, 50, 50, 50},
    { 10, 10, 20, 30, 30, 20, 10, 10},
    {  5,  5, 10, 25, 25, 10,  5,  5},
    {  0,  0,  0, 20, 20,  0,  0,  0},
    {  5, -5,-10,  0,  0,-10, -5,  5},
    {  5, 10, 10,-20,-20, 10, 10,  5},
    {  0,  0,  0,  0,  0,  0,  0,  0}
};

static int positionBonus(PieceType type, Color color, int r, int c) {
    int row = (color == Color::WHITE) ? r : (7 - r);
    switch (type) {
        case PieceType::KNIGHT: return knightTable[row][c];
        case PieceType::PAWN:   return pawnTable[row][c];
        default:                return 0;
    }
}

int PositionalEvaluator::evaluate(const Board& board, Color side) const {
    int score = 0;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            Piece p = board.getPiece(r, c);
            if (p.isEmpty()) continue;
            int v = pieceValue(p.type) + positionBonus(p.type, p.color, r, c);
            score += (p.color == side) ? v : -v;
        }
    return score;
}

int AggressiveEvaluator::evaluate(const Board& board, Color side) const {
    int score = 0;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            Piece p = board.getPiece(r, c);
            if (p.isEmpty()) continue;
            int v = pieceValue(p.type);
            score += (p.color == side) ? v : -v;
        }
    int mobility = static_cast<int>(board.generatePseudoLegalMoves(side).size()) -
                   static_cast<int>(board.generatePseudoLegalMoves(opponent(side)).size());
    return score + 2 * mobility;
}

std::unique_ptr<Evaluator> EvaluatorFactory::create(Strategy strategy) {
    switch (strategy) {
        case Strategy::Material:   return std::make_unique<MaterialEvaluator>();
        case Strategy::Positional: return std::make_unique<PositionalEvaluator>();
        case Strategy::Aggressive: return std::make_unique<AggressiveEvaluator>();
    }
    return std::make_unique<PositionalEvaluator>();
}
