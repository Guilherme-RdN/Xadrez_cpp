#include "EstadoJogo.h"

namespace xadrez {

bool EstadoJogo::casaEstaAtacada(const Tabuleiro& tab, int lin, int col, Cor atacante) {
    for (const Jogada& j : tab.gerarPseudoLegais(atacante))
        if (j.linDestino == lin && j.colDestino == col)
            return true;
    return false;
}

bool EstadoJogo::estaEmXeque(const Tabuleiro& tab, Cor lado) {
    int linRei, colRei;
    if (!tab.encontrarRei(lado, linRei, colRei))
        return false;
    return casaEstaAtacada(tab, linRei, colRei, oponente(lado));
}

std::vector<Jogada> EstadoJogo::gerarJogadasLegais(Tabuleiro& tab, Cor lado) {
    std::vector<Jogada> legais;
    for (const Jogada& j : tab.gerarPseudoLegais(lado)) {
        tab.fazerJogada(j);
        if (!estaEmXeque(tab, lado))
            legais.push_back(j);
        tab.desfazer();
    }
    return legais;
}

bool EstadoJogo::eXequeMate(Tabuleiro& tab, Cor lado) {
    return estaEmXeque(tab, lado) && gerarJogadasLegais(tab, lado).empty();
}

bool EstadoJogo::eAfogamento(Tabuleiro& tab, Cor lado) {
    return !estaEmXeque(tab, lado) && gerarJogadasLegais(tab, lado).empty();
}

} // namespace xadrez
