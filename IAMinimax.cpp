#include "IAMinimax.h"
#include "EstadoJogo.h"
#include <algorithm>

namespace xadrez {

static const int INFINITO = 1'000'000;
static const int MATE     =   100'000;

IAMinimax::IAMinimax(int profundidade, std::unique_ptr<Avaliador> avaliador)
    : profundidade_(profundidade), avaliador_(std::move(avaliador)) {}

int IAMinimax::minimax(Tabuleiro& tab, int prof, int alpha, int beta,
                       bool maximizando, Cor ladoRaiz) {
    ++nos_;

    Cor ladoAtual = maximizando ? ladoRaiz : oponente(ladoRaiz);
    std::vector<Jogada> jogadas = EstadoJogo::gerarJogadasLegais(tab, ladoAtual);

    if (jogadas.empty()) {
        if (EstadoJogo::estaEmXeque(tab, ladoAtual))
            return maximizando ? (-MATE - prof) : (MATE + prof);
        return 0; // afogamento
    }

    if (prof == 0)
        return avaliador_->avaliar(tab, ladoRaiz);

    // Capturas primeiro: melhora a eficiencia da poda
    std::sort(jogadas.begin(), jogadas.end(),
        [&tab](const Jogada& a, const Jogada& b) {
            return !tab.peca(a.linDestino, a.colDestino).vazia() >
                   !tab.peca(b.linDestino, b.colDestino).vazia();
        });

    if (maximizando) {
        int melhor = -INFINITO;
        for (const Jogada& j : jogadas) {
            tab.fazerJogada(j);
            melhor = std::max(melhor, minimax(tab, prof - 1, alpha, beta, false, ladoRaiz));
            tab.desfazer();
            alpha = std::max(alpha, melhor);
            if (beta <= alpha) break;
        }
        return melhor;
    } else {
        int melhor = INFINITO;
        for (const Jogada& j : jogadas) {
            tab.fazerJogada(j);
            melhor = std::min(melhor, minimax(tab, prof - 1, alpha, beta, true, ladoRaiz));
            tab.desfazer();
            beta = std::min(beta, melhor);
            if (beta <= alpha) break;
        }
        return melhor;
    }
}

Jogada IAMinimax::escolherJogada(Tabuleiro& tab, Cor lado, bool& encontrou) {
    nos_ = 0;
    std::vector<Jogada> jogadas = EstadoJogo::gerarJogadasLegais(tab, lado);

    if (jogadas.empty()) {
        encontrou = false;
        return Jogada{};
    }

    Jogada melhorJogada    = jogadas.front();
    int    melhorPontuacao = -INFINITO;
    int    alpha           = -INFINITO;
    int    beta            =  INFINITO;

    for (const Jogada& j : jogadas) {
        tab.fazerJogada(j);
        int pontos = minimax(tab, profundidade_ - 1, alpha, beta, false, lado);
        tab.desfazer();
        if (pontos > melhorPontuacao) {
            melhorPontuacao = pontos;
            melhorJogada    = j;
        }
        alpha = std::max(alpha, melhorPontuacao);
    }

    encontrou = true;
    return melhorJogada;
}

} // namespace xadrez
