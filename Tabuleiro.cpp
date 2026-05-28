#include "Tabuleiro.h"
#include <sstream>

// Coordenadas: lin 0 = fileira 8 (pretas), lin 7 = fileira 1 (brancas).
// Brancas avancam para cima (lin diminui), pretas para baixo.

namespace xadrez {

Tabuleiro::Tabuleiro() {
    for (int l = 0; l < 8; ++l)
        for (int c = 0; c < 8; ++c)
            grade_[l][c] = Peca{};

    const TipoPeca fundo[8] = {
        TipoPeca::TORRE, TipoPeca::CAVALO, TipoPeca::BISPO, TipoPeca::RAINHA,
        TipoPeca::REI,   TipoPeca::BISPO,  TipoPeca::CAVALO, TipoPeca::TORRE
    };

    for (int c = 0; c < 8; ++c) {
        grade_[0][c] = Peca{fundo[c],        Cor::PRETA};
        grade_[1][c] = Peca{TipoPeca::PEAO,  Cor::PRETA};
        grade_[6][c] = Peca{TipoPeca::PEAO,  Cor::BRANCA};
        grade_[7][c] = Peca{fundo[c],        Cor::BRANCA};
    }
}

bool Tabuleiro::dentroLimites(int lin, int col) const {
    return lin >= 0 && lin < 8 && col >= 0 && col < 8;
}

Peca Tabuleiro::peca(int lin, int col) const {
    return grade_[lin][col];
}

bool Tabuleiro::ocupada(int lin, int col) const {
    return !grade_[lin][col].vazia();
}

void Tabuleiro::fazerJogada(const Jogada& j) {
    InfoDesfazer info;
    info.jogada       = j;
    info.capturada    = grade_[j.linDestino][j.colDestino];
    info.ladoAnterior = ladoAtual_;
    historico_.push_back(info);

    Peca movendo = grade_[j.linOrigem][j.colOrigem];
    if (j.promocao != TipoPeca::VAZIA)
        movendo.tipo = j.promocao;

    grade_[j.linDestino][j.colDestino] = movendo;
    grade_[j.linOrigem][j.colOrigem]   = Peca{};
    ladoAtual_ = oponente(ladoAtual_);
}

void Tabuleiro::desfazer() {
    if (historico_.empty()) return;

    InfoDesfazer info = historico_.back();
    historico_.pop_back();
    const Jogada& j = info.jogada;

    Peca movida = grade_[j.linDestino][j.colDestino];
    if (j.promocao != TipoPeca::VAZIA)
        movida.tipo = TipoPeca::PEAO;

    grade_[j.linOrigem][j.colOrigem]   = movida;
    grade_[j.linDestino][j.colDestino] = info.capturada;
    ladoAtual_ = info.ladoAnterior;
}

bool Tabuleiro::encontrarRei(Cor lado, int& lin, int& col) const {
    for (int l = 0; l < 8; ++l)
        for (int c = 0; c < 8; ++c) {
            const Peca& p = grade_[l][c];
            if (p.tipo == TipoPeca::REI && p.cor == lado) {
                lin = l;
                col = c;
                return true;
            }
        }
    return false;
}

void Tabuleiro::adicionarDeslizantes(int lin, int col, const int dirs[][2], int n,
                                      std::vector<Jogada>& saida) const {
    Cor eu = grade_[lin][col].cor;
    for (int d = 0; d < n; ++d) {
        int nl = lin + dirs[d][0];
        int nc = col + dirs[d][1];
        while (dentroLimites(nl, nc)) {
            const Peca& alvo = grade_[nl][nc];
            if (alvo.vazia()) {
                saida.push_back(Jogada{lin, col, nl, nc});
            } else {
                if (alvo.cor != eu)
                    saida.push_back(Jogada{lin, col, nl, nc});
                break;
            }
            nl += dirs[d][0];
            nc += dirs[d][1];
        }
    }
}

void Tabuleiro::adicionarSaltos(int lin, int col, const int passos[][2], int n,
                                 std::vector<Jogada>& saida) const {
    Cor eu = grade_[lin][col].cor;
    for (int s = 0; s < n; ++s) {
        int nl = lin + passos[s][0];
        int nc = col + passos[s][1];
        if (!dentroLimites(nl, nc)) continue;
        const Peca& alvo = grade_[nl][nc];
        if (alvo.vazia() || alvo.cor != eu)
            saida.push_back(Jogada{lin, col, nl, nc});
    }
}

void Tabuleiro::adicionarPeao(int lin, int col, std::vector<Jogada>& saida) const {
    Cor eu       = grade_[lin][col].cor;
    int dir      = (eu == Cor::BRANCA) ? -1 : +1;
    int linInicio = (eu == Cor::BRANCA) ? 6 : 1;
    int linPromo  = (eu == Cor::BRANCA) ? 0 : 7;

    auto empurrar = [&](int nl, int nc) {
        if (nl == linPromo)
            saida.push_back(Jogada{lin, col, nl, nc, TipoPeca::RAINHA});
        else
            saida.push_back(Jogada{lin, col, nl, nc});
    };

    int umaLinha = lin + dir;
    if (dentroLimites(umaLinha, col) && grade_[umaLinha][col].vazia()) {
        empurrar(umaLinha, col);
        int duasLinhas = lin + 2 * dir;
        if (lin == linInicio && grade_[duasLinhas][col].vazia())
            saida.push_back(Jogada{lin, col, duasLinhas, col});
    }

    for (int dc = -1; dc <= 1; dc += 2) {
        int nc = col + dc;
        if (!dentroLimites(umaLinha, nc)) continue;
        const Peca& alvo = grade_[umaLinha][nc];
        if (!alvo.vazia() && alvo.cor != eu)
            empurrar(umaLinha, nc);
    }
}

std::vector<Jogada> Tabuleiro::gerarPseudoLegais(Cor lado) const {
    std::vector<Jogada> jogadas;
    jogadas.reserve(64);

    static const int dirsTorre[4][2]   = {{-1,0},{1,0},{0,-1},{0,1}};
    static const int dirsBispo[4][2]   = {{-1,-1},{-1,1},{1,-1},{1,1}};
    static const int passosCavalo[8][2]= {{-2,-1},{-2,1},{-1,-2},{-1,2},
                                           {1,-2},{1,2},{2,-1},{2,1}};
    static const int passosRei[8][2]   = {{-1,-1},{-1,0},{-1,1},{0,-1},
                                           {0,1},{1,-1},{1,0},{1,1}};

    for (int l = 0; l < 8; ++l) {
        for (int c = 0; c < 8; ++c) {
            const Peca& p = grade_[l][c];
            if (p.vazia() || p.cor != lado) continue;

            switch (p.tipo) {
                case TipoPeca::PEAO:   adicionarPeao(l, c, jogadas); break;
                case TipoPeca::CAVALO: adicionarSaltos(l, c, passosCavalo, 8, jogadas); break;
                case TipoPeca::BISPO:  adicionarDeslizantes(l, c, dirsBispo, 4, jogadas); break;
                case TipoPeca::TORRE:  adicionarDeslizantes(l, c, dirsTorre, 4, jogadas); break;
                case TipoPeca::RAINHA:
                    adicionarDeslizantes(l, c, dirsTorre,  4, jogadas);
                    adicionarDeslizantes(l, c, dirsBispo, 4, jogadas);
                    break;
                case TipoPeca::REI:    adicionarSaltos(l, c, passosRei, 8, jogadas); break;
                default: break;
            }
        }
    }
    return jogadas;
}

std::string Tabuleiro::texto() const {
    static const char* glifo[7] = {".", "P", "C", "B", "T", "R", "K"};
    std::ostringstream os;
    os << "  a b c d e f g h\n";
    for (int l = 0; l < 8; ++l) {
        os << (8 - l) << ' ';
        for (int c = 0; c < 8; ++c) {
            const Peca& p = grade_[l][c];
            char ch = glifo[static_cast<int>(p.tipo)][0];
            if (p.cor == Cor::PRETA) ch = static_cast<char>(ch + ('a' - 'A'));
            os << ch << ' ';
        }
        os << (8 - l) << '\n';
    }
    os << "  a b c d e f g h\n";
    return os.str();
}

} // namespace xadrez
