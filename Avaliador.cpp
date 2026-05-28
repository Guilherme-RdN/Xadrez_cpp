#include "Avaliador.h"

namespace xadrez {

// Valores em centipawns (centesimos de peao).
static int valorPeca(TipoPeca tipo) {
    switch (tipo) {
        case TipoPeca::PEAO:   return 100;
        case TipoPeca::CAVALO: return 320;
        case TipoPeca::BISPO:  return 330;
        case TipoPeca::TORRE:  return 500;
        case TipoPeca::RAINHA: return 900;
        case TipoPeca::REI:    return 20000;
        default:               return 0;
    }
}

int AvaliadorMaterial::avaliar(const Tabuleiro& tab, Cor lado) const {
    int pontuacao = 0;
    for (int lin = 0; lin < 8; ++lin)
        for (int col = 0; col < 8; ++col) {
            const Peca& p = tab.peca(lin, col);
            if (p.vazia()) continue;
            int v = valorPeca(p.tipo);
            pontuacao += (p.cor == lado) ? v : -v;
        }
    return pontuacao;
}

// Piece-square tables escritas do ponto de vista das brancas (lin 0 = fileira 8).
// Para as pretas, espelhamos a linha em bonusPosicao().
static const int tabelaCavalo[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

static const int tabelaPeao[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0},
    { 50, 50, 50, 50, 50, 50, 50, 50},
    { 10, 10, 20, 30, 30, 20, 10, 10},
    {  5,  5, 10, 25, 25, 10,  5,  5},
    {  0,  0,  0, 20, 20,  0,  0,  0},
    {  5, -5,-10,  0,  0,-10, -5,  5},
    {  5, 10, 10,-20,-20, 10, 10,  5},
    {  0,  0,  0,  0,  0,  0,  0,  0}
};

static int bonusPosicao(TipoPeca tipo, Cor cor, int lin, int col) {
    int fileira = (cor == Cor::BRANCA) ? lin : (7 - lin);
    switch (tipo) {
        case TipoPeca::CAVALO: return tabelaCavalo[fileira][col];
        case TipoPeca::PEAO:   return tabelaPeao[fileira][col];
        default:               return 0;
    }
}

int AvaliadorPosicional::avaliar(const Tabuleiro& tab, Cor lado) const {
    int pontuacao = 0;
    for (int lin = 0; lin < 8; ++lin)
        for (int col = 0; col < 8; ++col) {
            const Peca& p = tab.peca(lin, col);
            if (p.vazia()) continue;
            int v = valorPeca(p.tipo) + bonusPosicao(p.tipo, p.cor, lin, col);
            pontuacao += (p.cor == lado) ? v : -v;
        }
    return pontuacao;
}

int AvaliadorAgressivo::avaliar(const Tabuleiro& tab, Cor lado) const {
    int pontuacao = 0;
    for (int lin = 0; lin < 8; ++lin)
        for (int col = 0; col < 8; ++col) {
            const Peca& p = tab.peca(lin, col);
            if (p.vazia()) continue;
            pontuacao += (p.cor == lado) ? valorPeca(p.tipo) : -valorPeca(p.tipo);
        }
    Cor adv = oponente(lado);
    int mob = static_cast<int>(tab.gerarPseudoLegais(lado).size())
            - static_cast<int>(tab.gerarPseudoLegais(adv).size());
    return pontuacao + 2 * mob;
}

std::unique_ptr<Avaliador> FabricaAvaliador::criar(Estrategia e) {
    switch (e) {
        case Estrategia::Material:   return std::make_unique<AvaliadorMaterial>();
        case Estrategia::Posicional: return std::make_unique<AvaliadorPosicional>();
        case Estrategia::Agressivo:  return std::make_unique<AvaliadorAgressivo>();
    }
    return std::make_unique<AvaliadorPosicional>();
}

} // namespace xadrez
