#ifndef XADREZ_IA_MINIMAX_H
#define XADREZ_IA_MINIMAX_H

#include "Tabuleiro.h"
#include "Avaliador.h"
#include "Types.h"
#include <memory>

namespace xadrez {

// Motor de busca: Minimax com poda Alpha-Beta.
class IAMinimax {
public:
    IAMinimax(int profundidade, std::unique_ptr<Avaliador> avaliador);

    Jogada escolherJogada(Tabuleiro& tab, Cor lado, bool& encontrou);
    long nosVisitados() const { return nos_; }

private:
    int                        profundidade_;
    std::unique_ptr<Avaliador> avaliador_;
    long                       nos_ = 0;

    int minimax(Tabuleiro& tab, int prof, int alpha, int beta,
                bool maximizando, Cor ladoRaiz);
};

} // namespace xadrez

#endif // XADREZ_IA_MINIMAX_H
