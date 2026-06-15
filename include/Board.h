#pragma once
#include <iostream>
#include <string>
#include <vector>

// Enums para o tipo de peça e cor, conforme o roteiro
enum class PieceType { NONE, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };
enum class Color { NONE, WHITE, BLACK };

inline Color opponent(Color c) {
    return (c == Color::WHITE) ? Color::BLACK : Color::WHITE;
}

// Struct que junta o tipo da peça e a cor na mesma casa
struct Piece {
    PieceType type = PieceType::NONE;
    Color color = Color::NONE;

    bool isEmpty() const { return type == PieceType::NONE; }
};

enum class MoveType { Normal, Castle, EnPassant };

// Struct para registrar os movimentos
struct Move {
    int fromRow = 0;
    int fromCol = 0;
    int toRow = 0;
    int toCol = 0;
    PieceType promotion = PieceType::NONE;
    MoveType type = MoveType::Normal;

    bool operator==(const Move& other) const {
        return fromRow == other.fromRow && fromCol == other.fromCol &&
               toRow == other.toRow && toCol == other.toCol &&
               promotion == other.promotion;
    }
};

class Board {
private:
    // Array 8x8 representando o tabuleiro
    Piece board[8][8];

    Color sideToMove_ = Color::WHITE;

    bool castleKingSide[3] = {false, true, true};
    bool castleQueenSide[3] = {false, true, true};
    int epRow = -1;
    int epCol = -1;

    struct Undo {
        Move move;
        Piece captured;
        Color prevSide;
        bool ck[3];
        bool cq[3];
        int epR;
        int epC;
    };
    std::vector<Undo> history;

    // Função auxiliar para imprimir o caractere correto da peça
    char getPieceChar(Piece p) const;

    void addSliding(int r, int c, const int dirs[][2], int n, std::vector<Move>& out) const;
    void addSteps(int r, int c, const int steps[][2], int n, std::vector<Move>& out) const;
    void addPawn(int r, int c, std::vector<Move>& out) const;
    void addCastles(int r, int c, std::vector<Move>& out) const;

public:
    // Construtor que arruma as peças na posição inicial
    Board();

    // Métodos obrigatórios da Fase 1
    void makeMove(Move move);
    void undoMove(Move move);
    bool isOccupied(int row, int col) const;
    Piece getPiece(int row, int col) const;

    // Impressão com notação algébrica
    void printBoard() const;

    // Extensões da Fase 2 (geração de movimentos e estado de turno)
    Color sideToMove() const { return sideToMove_; }
    bool inBounds(int row, int col) const;
    bool findKing(Color color, int& row, int& col) const;
    std::vector<Move> generatePseudoLegalMoves(Color color) const;

    // Rastreio de posicoes para repeticao
    std::string positionKey() const;
    int countRepetitions() const;

private:
    std::vector<std::string> posHistory_;
};
