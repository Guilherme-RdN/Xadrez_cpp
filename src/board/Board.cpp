#include "../../include/Board.h"

using namespace std;

Board::Board() {
    // 1. Limpa o tabuleiro inteiro
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = {PieceType::NONE, Color::NONE};
        }
    }

    // 2. Coloca os Peões
    for (int j = 0; j < 8; j++) {
        board[1][j] = {PieceType::PAWN, Color::BLACK};
        board[6][j] = {PieceType::PAWN, Color::WHITE};
    }

    // 3. Coloca as Torres, Cavalos, Bispos, Rainhas e Reis
    PieceType backRank[8] = {PieceType::ROOK, PieceType::KNIGHT, PieceType::BISHOP, PieceType::QUEEN, PieceType::KING, PieceType::BISHOP, PieceType::KNIGHT, PieceType::ROOK};
    
    for (int j = 0; j < 8; j++) {
        board[0][j] = {backRank[j], Color::BLACK};
        board[7][j] = {backRank[j], Color::WHITE};
    }
}

// Retorna 'true' se a casa não estiver vazia
bool Board::isOccupied(int row, int col) const {
    return board[row][col].type != PieceType::NONE;
}

// Retorna a peça de uma determinada casa
Piece Board::getPiece(int row, int col) const {
    return board[row][col];
}

// Esqueletos dos movimentos (serão preenchidos nas próximas fases)
void Board::makeMove(Move move) {
    // Pega a peça da origem e coloca no destino
    board[move.toRow][move.toCol] = board[move.fromRow][move.fromCol];
    // Esvazia a origem
    board[move.fromRow][move.fromCol] = {PieceType::NONE, Color::NONE};
}

void Board::undoMove(Move move) {
    // Lógica para desfazer será feita depois
}

// Transforma o Enum em uma letra para imprimir na tela
char Board::getPieceChar(Piece p) const {
    if (p.type == PieceType::NONE) return '.';
    
    char c;
    switch (p.type) {
        case PieceType::PAWN:   c = 'p'; break;
        case PieceType::ROOK:   c = 'r'; break;
        case PieceType::KNIGHT: c = 'n'; break;
        case PieceType::BISHOP: c = 'b'; break;
        case PieceType::QUEEN:  c = 'q'; break;
        case PieceType::KING:   c = 'k'; break;
        default: return '.';
    }
    // Maiúsculas para brancas, minúsculas para pretas
    return (p.color == Color::WHITE) ? toupper(c) : c;
}

// Imprime o tabuleiro
void Board::printBoard() const {
    cout << "\n   a b c d e f g h\n";
    for (int i = 0; i < 8; i++) {
        cout << 8 - i << " "; // Notação lateral (1 a 8)
        for (int j = 0; j < 8; j++) {
            cout << " " << getPieceChar(board[i][j]);
        }
        cout << "  " << 8 - i << "\n";
    }
    cout << "   a b c d e f g h\n\n";
}