#include "../../include/Board.h"
#include <string>

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

    posHistory_.push_back(positionKey());
    simplePosHistory_.push_back(simplePositionKey());
}

// Retorna 'true' se a casa não estiver vazia
bool Board::isOccupied(int row, int col) const {
    return board[row][col].type != PieceType::NONE;
}

// Retorna a peça de uma determinada casa
Piece Board::getPiece(int row, int col) const {
    return board[row][col];
}

bool Board::inBounds(int row, int col) const {
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

void Board::makeMove(Move move) {
    Undo u;
    u.move = move;
    u.prevSide = sideToMove_;
    for (int i = 0; i < 3; i++) { u.ck[i] = castleKingSide[i]; u.cq[i] = castleQueenSide[i]; }
    u.epR = epRow;
    u.epC = epCol;

    Piece moving = board[move.fromRow][move.fromCol];

    if (move.type == MoveType::EnPassant) {
        u.captured = board[move.fromRow][move.toCol];
        board[move.fromRow][move.toCol] = {PieceType::NONE, Color::NONE};
    } else {
        u.captured = board[move.toRow][move.toCol];
    }
    history.push_back(u);

    epRow = -1;
    epCol = -1;

    board[move.toRow][move.toCol] = moving;
    board[move.fromRow][move.fromCol] = {PieceType::NONE, Color::NONE};

    if (move.promotion != PieceType::NONE)
        board[move.toRow][move.toCol].type = move.promotion;

    if (move.type == MoveType::Castle) {
        int row = move.fromRow;
        if (move.toCol == 6) {
            board[row][5] = board[row][7];
            board[row][7] = {PieceType::NONE, Color::NONE};
        } else if (move.toCol == 2) {
            board[row][3] = board[row][0];
            board[row][0] = {PieceType::NONE, Color::NONE};
        }
    }

    int diff = move.toRow - move.fromRow;
    if (moving.type == PieceType::PAWN && (diff == 2 || diff == -2)) {
        epRow = (move.fromRow + move.toRow) / 2;
        epCol = move.fromCol;
    }

    int idx = static_cast<int>(moving.color);
    if (moving.type == PieceType::KING) {
        castleKingSide[idx] = false;
        castleQueenSide[idx] = false;
    }
    if (moving.type == PieceType::ROOK) {
        if (move.fromCol == 0) castleQueenSide[idx] = false;
        if (move.fromCol == 7) castleKingSide[idx] = false;
    }
    if (move.toRow == 0 && move.toCol == 0) castleQueenSide[static_cast<int>(Color::BLACK)] = false;
    if (move.toRow == 0 && move.toCol == 7) castleKingSide[static_cast<int>(Color::BLACK)] = false;
    if (move.toRow == 7 && move.toCol == 0) castleQueenSide[static_cast<int>(Color::WHITE)] = false;
    if (move.toRow == 7 && move.toCol == 7) castleKingSide[static_cast<int>(Color::WHITE)] = false;

    sideToMove_ = opponent(sideToMove_);
    posHistory_.push_back(positionKey());
    simplePosHistory_.push_back(simplePositionKey());
}

void Board::undoMove(Move) {
    if (history.empty()) return;
    if (!posHistory_.empty())       posHistory_.pop_back();
    if (!simplePosHistory_.empty()) simplePosHistory_.pop_back();
    Undo u = history.back();
    history.pop_back();
    Move m = u.move;

    Piece moved = board[m.toRow][m.toCol];
    if (m.promotion != PieceType::NONE)
        moved.type = PieceType::PAWN;

    board[m.fromRow][m.fromCol] = moved;

    if (m.type == MoveType::EnPassant) {
        board[m.toRow][m.toCol] = {PieceType::NONE, Color::NONE};
        board[m.fromRow][m.toCol] = u.captured;
    } else {
        board[m.toRow][m.toCol] = u.captured;
    }

    if (m.type == MoveType::Castle) {
        int row = m.fromRow;
        if (m.toCol == 6) {
            board[row][7] = board[row][5];
            board[row][5] = {PieceType::NONE, Color::NONE};
        } else if (m.toCol == 2) {
            board[row][0] = board[row][3];
            board[row][3] = {PieceType::NONE, Color::NONE};
        }
    }

    for (int i = 0; i < 3; i++) { castleKingSide[i] = u.ck[i]; castleQueenSide[i] = u.cq[i]; }
    epRow = u.epR;
    epCol = u.epC;
    sideToMove_ = u.prevSide;
}

bool Board::findKing(Color color, int& row, int& col) const {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (board[r][c].type == PieceType::KING && board[r][c].color == color) {
                row = r;
                col = c;
                return true;
            }
    return false;
}

void Board::addSliding(int r, int c, const int dirs[][2], int n, std::vector<Move>& out) const {
    Color me = board[r][c].color;
    for (int d = 0; d < n; d++) {
        int nr = r + dirs[d][0];
        int nc = c + dirs[d][1];
        while (inBounds(nr, nc)) {
            Piece target = board[nr][nc];
            if (target.isEmpty()) {
                out.push_back(Move{r, c, nr, nc});
            } else {
                if (target.color != me) out.push_back(Move{r, c, nr, nc});
                break;
            }
            nr += dirs[d][0];
            nc += dirs[d][1];
        }
    }
}

void Board::addSteps(int r, int c, const int steps[][2], int n, std::vector<Move>& out) const {
    Color me = board[r][c].color;
    for (int s = 0; s < n; s++) {
        int nr = r + steps[s][0];
        int nc = c + steps[s][1];
        if (!inBounds(nr, nc)) continue;
        Piece target = board[nr][nc];
        if (target.isEmpty() || target.color != me) out.push_back(Move{r, c, nr, nc});
    }
}

void Board::addPawn(int r, int c, std::vector<Move>& out) const {
    Color me = board[r][c].color;
    int dir = (me == Color::WHITE) ? -1 : 1;
    int startRow = (me == Color::WHITE) ? 6 : 1;
    int promoRow = (me == Color::WHITE) ? 0 : 7;

    int oneR = r + dir;
    if (inBounds(oneR, c) && board[oneR][c].isEmpty()) {
        if (oneR == promoRow) {
            out.push_back(Move{r, c, oneR, c, PieceType::QUEEN});
            out.push_back(Move{r, c, oneR, c, PieceType::ROOK});
            out.push_back(Move{r, c, oneR, c, PieceType::BISHOP});
            out.push_back(Move{r, c, oneR, c, PieceType::KNIGHT});
        } else {
            out.push_back(Move{r, c, oneR, c});
            int twoR = r + 2 * dir;
            if (r == startRow && board[twoR][c].isEmpty())
                out.push_back(Move{r, c, twoR, c});
        }
    }

    for (int dc = -1; dc <= 1; dc += 2) {
        int nc = c + dc;
        if (!inBounds(oneR, nc)) continue;
        Piece target = board[oneR][nc];
        if (!target.isEmpty() && target.color != me) {
            if (oneR == promoRow) {
                out.push_back(Move{r, c, oneR, nc, PieceType::QUEEN});
                out.push_back(Move{r, c, oneR, nc, PieceType::ROOK});
                out.push_back(Move{r, c, oneR, nc, PieceType::BISHOP});
                out.push_back(Move{r, c, oneR, nc, PieceType::KNIGHT});
            } else {
                out.push_back(Move{r, c, oneR, nc});
            }
        } else if (oneR == epRow && nc == epCol) {
            out.push_back(Move{r, c, oneR, nc, PieceType::NONE, MoveType::EnPassant});
        }
    }
}

void Board::addCastles(int r, int c, std::vector<Move>& out) const {
    Color me = board[r][c].color;
    int idx = static_cast<int>(me);
    if (c != 4) return;

    if (castleKingSide[idx] && board[r][5].isEmpty() && board[r][6].isEmpty() &&
        board[r][7].type == PieceType::ROOK && board[r][7].color == me) {
        out.push_back(Move{r, 4, r, 6, PieceType::NONE, MoveType::Castle});
    }
    if (castleQueenSide[idx] && board[r][1].isEmpty() && board[r][2].isEmpty() &&
        board[r][3].isEmpty() && board[r][0].type == PieceType::ROOK && board[r][0].color == me) {
        out.push_back(Move{r, 4, r, 2, PieceType::NONE, MoveType::Castle});
    }
}

std::vector<Move> Board::generatePseudoLegalMoves(Color color) const {
    std::vector<Move> moves;
    moves.reserve(64);

    static const int rookDirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    static const int bishopDirs[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    static const int knightSteps[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
    static const int kingSteps[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Piece p = board[r][c];
            if (p.isEmpty() || p.color != color) continue;

            switch (p.type) {
                case PieceType::PAWN:   addPawn(r, c, moves); break;
                case PieceType::KNIGHT: addSteps(r, c, knightSteps, 8, moves); break;
                case PieceType::BISHOP: addSliding(r, c, bishopDirs, 4, moves); break;
                case PieceType::ROOK:   addSliding(r, c, rookDirs, 4, moves); break;
                case PieceType::QUEEN:
                    addSliding(r, c, rookDirs, 4, moves);
                    addSliding(r, c, bishopDirs, 4, moves);
                    break;
                case PieceType::KING:
                    addSteps(r, c, kingSteps, 8, moves);
                    addCastles(r, c, moves);
                    break;
                default: break;
            }
        }
    }
    return moves;
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

std::string Board::positionKey() const {
    std::string key;
    key.reserve(70);
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            const Piece& p = board[r][c];
            if (p.isEmpty()) { key.push_back('.'); continue; }
            char ch;
            switch (p.type) {
                case PieceType::PAWN:   ch = 'P'; break;
                case PieceType::ROOK:   ch = 'R'; break;
                case PieceType::KNIGHT: ch = 'N'; break;
                case PieceType::BISHOP: ch = 'B'; break;
                case PieceType::QUEEN:  ch = 'Q'; break;
                case PieceType::KING:   ch = 'K'; break;
                default:                ch = '.'; break;
            }
            if (p.color == Color::BLACK) ch = static_cast<char>(ch + 32);
            key.push_back(ch);
        }
    key.push_back(sideToMove_ == Color::WHITE ? 'w' : 'b');
    int wi = static_cast<int>(Color::WHITE), bi = static_cast<int>(Color::BLACK);
    key.push_back(castleKingSide[wi]  ? 'K' : '-');
    key.push_back(castleQueenSide[wi] ? 'Q' : '-');
    key.push_back(castleKingSide[bi]  ? 'k' : '-');
    key.push_back(castleQueenSide[bi] ? 'q' : '-');
    key.push_back(epCol >= 0 ? static_cast<char>('a' + epCol) : '-');
    return key;
}

int Board::countRepetitions() const {
    if (posHistory_.empty()) return 0;
    const std::string& cur = posHistory_.back();
    int n = 0;
    for (const std::string& k : posHistory_)
        if (k == cur) ++n;
    return n;
}

// Chave simplificada: apenas 64 casas + lado a mover.
// Ignora direitos de roque e en passant para que o vai-e-vem seja
// detectado mesmo depois que esses direitos foram perdidos.
std::string Board::simplePositionKey() const {
    std::string key;
    key.reserve(65);
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            const Piece& p = board[r][c];
            if (p.isEmpty()) { key.push_back('.'); continue; }
            char ch;
            switch (p.type) {
                case PieceType::PAWN:   ch = 'P'; break;
                case PieceType::ROOK:   ch = 'R'; break;
                case PieceType::KNIGHT: ch = 'N'; break;
                case PieceType::BISHOP: ch = 'B'; break;
                case PieceType::QUEEN:  ch = 'Q'; break;
                case PieceType::KING:   ch = 'K'; break;
                default:                ch = '.'; break;
            }
            if (p.color == Color::BLACK) ch = static_cast<char>(ch + 32);
            key.push_back(ch);
        }
    key.push_back(sideToMove_ == Color::WHITE ? 'w' : 'b');
    return key;
}

int Board::countSimpleRepetitions() const {
    if (simplePosHistory_.empty()) return 0;
    const std::string& cur = simplePosHistory_.back();
    int n = 0;
    for (const std::string& k : simplePosHistory_)
        if (k == cur) ++n;
    return n;
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
