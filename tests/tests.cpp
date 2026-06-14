#include "Board.h"
#include "GameState.h"
#include "Evaluator.h"
#include "MinimaxAI.h"
#include "AIFactory.h"
#include <iostream>
#include <memory>
#include <vector>

static int passed = 0;
static int total = 0;

static void check(const char* what, bool cond) {
    total++;
    std::cout << (cond ? "[OK]    " : "[FALHA] ") << what << "\n";
    if (cond) passed++;
}

static bool sameBoard(const Board& a, const Board& b) {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            Piece pa = a.getPiece(r, c);
            Piece pb = b.getPiece(r, c);
            if (pa.type != pb.type || pa.color != pb.color) return false;
        }
    return a.sideToMove() == b.sideToMove();
}

int main() {
    std::cout << "=== Testes: Xadrez com IA ===\n\n";

    {
        Board b;
        check("Posicao inicial: brancas nao estao em xeque", !GameState::isInCheck(b, Color::WHITE));
        check("Posicao inicial: pretas nao estao em xeque", !GameState::isInCheck(b, Color::BLACK));
    }

    {
        Board b;
        check("Abertura tem 20 jogadas legais para as brancas",
              GameState::generateLegalMoves(b, Color::WHITE).size() == 20);
    }

    {
        Board b;
        check("Abertura nao e xeque-mate", !GameState::isCheckmate(b, Color::WHITE));
        check("Abertura nao e afogamento", !GameState::isStalemate(b, Color::WHITE));
    }

    {
        Board b;
        MinimaxAI ai(3, EvaluatorFactory::create(EvaluatorFactory::Strategy::Positional));
        bool found = false;
        Move m = ai.chooseMove(b, Color::WHITE, found);
        check("IA encontra uma jogada na abertura", found);
        bool legal = false;
        for (const Move& lm : GameState::generateLegalMoves(b, Color::WHITE))
            if (lm == m) legal = true;
        check("Jogada da IA e legal", legal);
        std::cout << "        (nos visitados: " << ai.nodesVisited() << ")\n";
    }

    {
        Board b;
        b.makeMove(Move{6, 4, 4, 4});
        b.makeMove(Move{1, 5, 3, 5});
        MinimaxAI ai(3, EvaluatorFactory::create(EvaluatorFactory::Strategy::Material));
        bool found = false;
        Move m = ai.chooseMove(b, Color::WHITE, found);
        check("IA captura o peao indefeso (exf5)", found && m.toRow == 3 && m.toCol == 5);
    }

    {
        Board b;
        b.makeMove(Move{6, 4, 4, 4});
        b.makeMove(Move{1, 0, 2, 0});
        b.makeMove(Move{7, 3, 3, 7});
        b.makeMove(Move{2, 0, 3, 0});
        MinimaxAI ai(2, EvaluatorFactory::create(EvaluatorFactory::Strategy::Material));
        bool found = false;
        Move m = ai.chooseMove(b, Color::WHITE, found);
        bool suicide = (m.fromRow == 3 && m.fromCol == 7 && m.toRow == 1 && m.toCol == 5);
        check("IA evita sacrificar a dama em f7 (defendida pelo rei)", found && !suicide);
    }

    {
        Board b;
        MinimaxAI ai(4, EvaluatorFactory::create(EvaluatorFactory::Strategy::Material));
        bool found = false;
        ai.chooseMove(b, Color::WHITE, found);
        std::cout << "        (profundidade 4: " << ai.nodesVisited() << " nos com poda)\n";
        check("Busca em profundidade 4 termina", found);
    }

    {
        Board b;
        b.makeMove(Move{6, 4, 4, 4});
        std::unique_ptr<Evaluator> pos = std::make_unique<PositionalEvaluator>();
        std::unique_ptr<Evaluator> mat = std::make_unique<MaterialEvaluator>();
        check("Polimorfismo: avaliador posicional difere do material apos e4",
              pos->evaluate(b, Color::WHITE) != mat->evaluate(b, Color::WHITE));
    }

    {
        Board b;
        MinimaxAI ai(3, EvaluatorFactory::create(EvaluatorFactory::Strategy::Aggressive));
        bool found = false;
        ai.chooseMove(b, Color::WHITE, found);
        check("EvaluatorFactory cria avaliador agressivo e IA joga", found);
    }

    {
        Board b;
        b.makeMove(Move{7, 6, 5, 5});
        b.makeMove(Move{0, 6, 2, 5});
        b.makeMove(Move{7, 5, 3, 1});
        b.makeMove(Move{0, 5, 4, 1});
        bool hasCastle = false;
        for (const Move& m : GameState::generateLegalMoves(b, Color::WHITE))
            if (m.type == MoveType::Castle && m.toCol == 6) hasCastle = true;
        check("Roque pequeno disponivel com o caminho livre", hasCastle);
    }

    {
        Board b;
        bool hasCastle = false;
        for (const Move& m : GameState::generateLegalMoves(b, Color::WHITE))
            if (m.type == MoveType::Castle) hasCastle = true;
        check("Sem roque na abertura (caminho bloqueado)", !hasCastle);
    }

    {
        Board b;
        b.makeMove(Move{7, 6, 5, 5});
        b.makeMove(Move{0, 1, 5, 4});
        b.makeMove(Move{7, 5, 3, 1});
        b.makeMove(Move{1, 7, 2, 7});

        bool pseudoCastle = false;
        for (const Move& m : b.generatePseudoLegalMoves(Color::WHITE))
            if (m.type == MoveType::Castle && m.toCol == 6) pseudoCastle = true;

        bool legalCastle = false;
        for (const Move& m : GameState::generateLegalMoves(b, Color::WHITE))
            if (m.type == MoveType::Castle && m.toCol == 6) legalCastle = true;

        check("Rei nao esta em xeque nessa posicao", !GameState::isInCheck(b, Color::WHITE));
        check("Roque e pseudo-legal mas rejeitado (rei passaria por f1 atacada)",
              pseudoCastle && !legalCastle);
    }

    {
        Board b;
        b.makeMove(Move{6, 4, 4, 4});
        b.makeMove(Move{1, 0, 2, 0});
        b.makeMove(Move{4, 4, 3, 4});
        b.makeMove(Move{1, 3, 3, 3});

        Move ep{};
        bool hasEp = false;
        for (const Move& m : GameState::generateLegalMoves(b, Color::WHITE))
            if (m.type == MoveType::EnPassant) { ep = m; hasEp = true; }
        check("En passant disponivel apos avanco duplo adversario", hasEp);

        if (hasEp) {
            Board before = b;
            b.makeMove(ep);
            bool captured = b.getPiece(3, 3).isEmpty() && b.getPiece(2, 3).type == PieceType::PAWN;
            check("En passant remove o peao capturado", captured);
            b.undoMove(ep);
            check("makeMove + undoMove restaura o tabuleiro (en passant)", sameBoard(before, b));
        }
    }

    {
        Board b;
        b.makeMove(Move{6, 0, 1, 0});
        b.makeMove(Move{0, 0, 3, 7});
        int promos = 0;
        Move knightPromo{};
        for (const Move& m : GameState::generateLegalMoves(b, Color::WHITE))
            if (m.fromRow == 1 && m.fromCol == 0 && m.toCol == 0 && m.promotion != PieceType::NONE) {
                promos++;
                if (m.promotion == PieceType::KNIGHT) knightPromo = m;
            }
        check("Promocao gera 4 opcoes (D/T/B/C)", promos == 4);
        b.makeMove(knightPromo);
        check("Promover para cavalo coloca um cavalo na ultima fileira",
              b.getPiece(0, 0).type == PieceType::KNIGHT && b.getPiece(0, 0).color == Color::WHITE);
    }

    {
        MinimaxAI easy = AIFactory::create(Difficulty::Easy);
        MinimaxAI medium = AIFactory::create(Difficulty::Medium);
        MinimaxAI hard = AIFactory::create(Difficulty::Hard);
        check("Dificuldade Facil = profundidade 2", easy.depth() == 2);
        check("Dificuldade Medio = profundidade 3", medium.depth() == 3);
        check("Dificuldade Dificil = profundidade 4", hard.depth() == 4);

        Board b;
        bool found = false;
        easy.chooseMove(b, Color::WHITE, found);
        check("IA Facil escolhe uma jogada", found);
    }

    std::cout << "\n=== Resultado: " << passed << "/" << total << " testes aprovados ===\n";
    return (passed == total) ? 0 : 1;
}
