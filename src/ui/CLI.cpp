#include "CLI.h"
#include "GameState.h"
#include "MinimaxAI.h"
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

Difficulty CLI::chooseDifficulty() const {
    cout << "Escolha a dificuldade:\n";
    cout << "  1 - Facil\n  2 - Medio\n  3 - Dificil\n";
    while (true) {
        cout << "Opcao: ";
        string line;
        if (!getline(cin, line)) return Difficulty::Medium;
        if (line == "1") return Difficulty::Easy;
        if (line == "2") return Difficulty::Medium;
        if (line == "3") return Difficulty::Hard;
        cout << "Opcao invalida.\n";
    }
}

Color CLI::chooseColor() const {
    cout << "Jogar com qual cor? (b = brancas, p = pretas): ";
    string line;
    if (!getline(cin, line)) return Color::WHITE;
    if (!line.empty() && (line[0] == 'p' || line[0] == 'P')) return Color::BLACK;
    return Color::WHITE;
}

bool CLI::parseSquares(const string& line, int& fr, int& fc, int& tr, int& tc) const {
    istringstream iss(line);
    string from, to;
    if (!(iss >> from >> to)) return false;
    if (from.size() < 2 || to.size() < 2) return false;

    fc = from[0] - 'a';
    fr = 8 - (from[1] - '0');
    tc = to[0] - 'a';
    tr = 8 - (to[1] - '0');

    return fc >= 0 && fc < 8 && fr >= 0 && fr < 8 && tc >= 0 && tc < 8 && tr >= 0 && tr < 8;
}

PieceType CLI::askPromotion() const {
    cout << "Promover para (Q/R/B/N): ";
    string line;
    getline(cin, line);
    if (line.empty()) return PieceType::QUEEN;
    switch (line[0]) {
        case 'R': case 'r': return PieceType::ROOK;
        case 'B': case 'b': return PieceType::BISHOP;
        case 'N': case 'n': return PieceType::KNIGHT;
        default:            return PieceType::QUEEN;
    }
}

string CLI::squareName(int row, int col) const {
    string s;
    s += static_cast<char>('a' + col);
    s += static_cast<char>('0' + (8 - row));
    return s;
}

void CLI::playGame(Difficulty difficulty, Color humanColor) {
    Board board;
    MinimaxAI ai = AIFactory::create(difficulty);

    cout << "\nDificuldade: " << AIFactory::name(difficulty)
         << " (profundidade " << ai.depth() << ", avaliador " << ai.evaluatorName() << ")\n";
    cout << "Comandos: 'e2 e4' para mover, 'sair' para encerrar.\n";

    while (true) {
        board.printBoard();
        Color side = board.sideToMove();

        vector<Move> legal = GameState::generateLegalMoves(board, side);
        if (legal.empty()) {
            if (GameState::isInCheck(board, side))
                cout << "Xeque-mate! " << (side == Color::WHITE ? "Pretas" : "Brancas") << " vencem.\n";
            else
                cout << "Afogamento! Empate.\n";
            return;
        }
        if (GameState::isInCheck(board, side))
            cout << "Xeque!\n";

        if (side == humanColor) {
            cout << "Seu lance: ";
            string line;
            if (!getline(cin, line)) return;
            if (line == "sair" || line == "quit") return;

            int fr, fc, tr, tc;
            if (!parseSquares(line, fr, fc, tr, tc)) {
                cout << "Formato invalido. Exemplo: e2 e4\n";
                continue;
            }

            vector<Move> matches;
            for (const Move& m : legal)
                if (m.fromRow == fr && m.fromCol == fc && m.toRow == tr && m.toCol == tc)
                    matches.push_back(m);

            if (matches.empty()) {
                cout << "Lance ilegal.\n";
                continue;
            }

            Move chosen = matches.front();
            if (matches.size() > 1) {
                PieceType promo = askPromotion();
                for (const Move& m : matches)
                    if (m.promotion == promo) chosen = m;
            }
            board.makeMove(chosen);
        } else {
            bool found = false;
            Move m = ai.chooseMove(board, side, found);
            if (!found) return;
            cout << "IA joga " << squareName(m.fromRow, m.fromCol) << " "
                 << squareName(m.toRow, m.toCol)
                 << "  (" << ai.nodesVisited() << " nos)\n";
            board.makeMove(m);
        }
    }
}

bool CLI::askPlayAgain() const {
    cout << "Jogar de novo? (s/n): ";
    string line;
    if (!getline(cin, line)) return false;
    return !line.empty() && (line[0] == 's' || line[0] == 'S');
}

void CLI::run() {
    cout << "=== Xadrez com IA (C++) ===\n\n";
    do {
        Difficulty difficulty = chooseDifficulty();
        Color humanColor = chooseColor();
        playGame(difficulty, humanColor);
    } while (askPlayAgain());
    cout << "Ate logo!\n";
}
