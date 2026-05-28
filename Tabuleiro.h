#ifndef XADREZ_TABULEIRO_H
#define XADREZ_TABULEIRO_H

#include "Types.h"
#include <vector>
#include <string>

namespace xadrez {

class Tabuleiro {
public:
    Tabuleiro();

    Peca peca(int lin, int col) const;
    bool ocupada(int lin, int col) const;
    bool dentroLimites(int lin, int col) const;
    Cor  ladoAtual() const { return ladoAtual_; }

    void fazerJogada(const Jogada& j);
    void desfazer();

    std::vector<Jogada> gerarPseudoLegais(Cor lado) const;
    bool encontrarRei(Cor lado, int& lin, int& col) const;

    std::string texto() const;

private:
    struct InfoDesfazer {
        Jogada jogada;
        Peca   capturada;
        Cor    ladoAnterior;
    };

    Peca grade_[8][8];
    Cor  ladoAtual_ = Cor::BRANCA;
    std::vector<InfoDesfazer> historico_;

    void adicionarDeslizantes(int lin, int col, const int dirs[][2], int n,
                               std::vector<Jogada>& saida) const;
    void adicionarSaltos(int lin, int col, const int passos[][2], int n,
                          std::vector<Jogada>& saida) const;
    void adicionarPeao(int lin, int col, std::vector<Jogada>& saida) const;
};

} // namespace xadrez

#endif // XADREZ_TABULEIRO_H
