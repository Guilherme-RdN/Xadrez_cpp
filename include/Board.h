#pragma once
#include <iostream>

// Enums para o tipo de peça e cor, conforme o roteiro
enum class PieceType { NONE, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };
enum class Color { NONE, WHITE, BLACK };

// Struct que junta o tipo da peça e a cor na mesma casa
struct Piece {
    PieceType type;
    Color color;
};

// Struct para registrar os movimentos
struct Move {
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;
};

class Board {
private:
    // Array 8x8 representando o tabuleiro
    Piece board[8][8];

    // Função auxiliar para imprimir o caractere correto da peça
    char getPieceChar(Piece p) const;

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
};