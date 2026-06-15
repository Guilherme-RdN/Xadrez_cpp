#include "EstadoJogo.h"

namespace xadrez {

bool EstadoJogo::casaEstaAtacada(const Tabuleiro& tab, int lin, int col, Cor atacante) {
    for (const Jogada& j : tab.gerarPseudoLegais(atacante)) {
        // Roque nao e um ataque: o rei nao "ataca" g1/g8/c1/c8 ao rocar
        int coldiff = j.colDestino - j.colOrigem;
        if (tab.peca(j.linOrigem, j.colOrigem).tipo == TipoPeca::REI &&
            (coldiff == 2 || coldiff == -2))
            continue;
        if (j.linDestino == lin && j.colDestino == col)
            return true;
    }
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
    bool emXeque = estaEmXeque(tab, lado);

    for (const Jogada& j : tab.gerarPseudoLegais(lado)) {
        // Roque precisa de verificacoes extras alem do make/unmake normal
        Peca p = tab.peca(j.linOrigem, j.colOrigem);
        int coldiff = j.colDestino - j.colOrigem;
        bool ehRoque = (p.tipo == TipoPeca::REI) && (coldiff == 2 || coldiff == -2);

        if (ehRoque) {
            if (emXeque) continue; // nao pode rocar saindo de xeque
            // A casa de passagem do rei nao pode estar atacada
            int dir = (coldiff > 0) ? 1 : -1;
            if (casaEstaAtacada(tab, j.linOrigem, j.colOrigem + dir, oponente(lado)))
                continue;
        }

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

bool EstadoJogo::eEmpatePorRepeticao(const Tabuleiro& tab) {
    return tab.eRepeticaoTripla();
}

bool EstadoJogo::eEmpatePorRegra50Lances(const Tabuleiro& tab) {
    return tab.meioLances() >= 100;
}

// Material insuficiente para dar mate (FIDE Art. 5.2b):
// K vs K, K+menor vs K, K+B vs K+B (mesma cor). Simplificacao pratica:
// K+menor vs K e K vs K sao os casos impossiveis com qualquer jogo.
bool EstadoJogo::eMaterialInsuficiente(const Tabuleiro& tab) {
    int  brancasMenores = 0, pretasMenores = 0;
    bool brancasMaior   = false, pretasMaior = false;

    for (int l = 0; l < 8; ++l) {
        for (int c = 0; c < 8; ++c) {
            const Peca& p = tab.peca(l, c);
            if (p.vazia() || p.tipo == TipoPeca::REI) continue;
            bool maior = (p.tipo == TipoPeca::PEAO  ||
                          p.tipo == TipoPeca::TORRE  ||
                          p.tipo == TipoPeca::RAINHA);
            if (p.cor == Cor::BRANCA) {
                if (maior) brancasMaior = true; else ++brancasMenores;
            } else {
                if (maior) pretasMaior  = true; else ++pretasMenores;
            }
        }
    }

    if (brancasMaior || pretasMaior) return false;
    if (brancasMenores == 0 && pretasMenores == 0) return true; // K vs K
    if (brancasMenores == 1 && pretasMenores == 0) return true; // K+menor vs K
    if (brancasMenores == 0 && pretasMenores == 1) return true; // K vs K+menor
    return false;
}

} // namespace xadrez
