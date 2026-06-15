#include "Tabuleiro.h"
#include <algorithm>
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

    roqueDisp_[0] = roqueDisp_[1] = roqueDisp_[2] = roqueDisp_[3] = true;
    epLin_ = epCol_ = -1;
    meioLances_ = 0;

    historicoPosicoes_.push_back(chavePosicao());
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
    info.ladoAnterior = ladoAtual_;
    std::copy(roqueDisp_, roqueDisp_ + 4, info.roqueDisp);
    info.epLin      = epLin_;
    info.epCol      = epCol_;
    info.meioLances = meioLances_;

    Peca movendo = grade_[j.linOrigem][j.colOrigem];

    // Detecta en passant: peao captura em diagonal para casa vazia (o alvo)
    bool foiEnPassant = (movendo.tipo == TipoPeca::PEAO) &&
                        (j.colDestino != j.colOrigem) &&
                        grade_[j.linDestino][j.colDestino].vazia();

    // Detecta roque: rei se move 2 colunas
    int coldiff = j.colDestino - j.colOrigem;
    bool foiRoque = (movendo.tipo == TipoPeca::REI) && (coldiff == 2 || coldiff == -2);

    // Salva peca capturada na sua posicao real (en passant difere do destino)
    if (foiEnPassant) {
        int linPeao = j.linOrigem;   // mesma fileira do peao capturante
        int colPeao = j.colDestino;  // mesma coluna do destino
        info.capturada    = grade_[linPeao][colPeao];
        info.linCapturada = linPeao;
        info.colCapturada = colPeao;
        grade_[linPeao][colPeao] = Peca{};
    } else {
        info.capturada    = grade_[j.linDestino][j.colDestino];
        info.linCapturada = j.linDestino;
        info.colCapturada = j.colDestino;
    }
    historico_.push_back(info);

    // Atualiza contador de 50 lances
    if (movendo.tipo == TipoPeca::PEAO || !info.capturada.vazia())
        meioLances_ = 0;
    else
        ++meioLances_;

    // Move a peca (com promocao se aplicavel)
    if (j.promocao != TipoPeca::VAZIA)
        movendo.tipo = j.promocao;
    grade_[j.linDestino][j.colDestino] = movendo;
    grade_[j.linOrigem][j.colOrigem]   = Peca{};

    // Roque: move a torre junto com o rei
    if (foiRoque) {
        int lin = j.linOrigem;
        if (coldiff > 0) {  // rei-lado (O-O): torre de h para f
            grade_[lin][5] = grade_[lin][7];
            grade_[lin][7] = Peca{};
        } else {            // dama-lado (O-O-O): torre de a para d
            grade_[lin][3] = grade_[lin][0];
            grade_[lin][0] = Peca{};
        }
    }

    // Atualiza alvo de en passant (valido so para o proximo lance)
    epLin_ = epCol_ = -1;
    int lindiff = j.linDestino - j.linOrigem;
    if (movendo.tipo == TipoPeca::PEAO && (lindiff == 2 || lindiff == -2)) {
        epLin_ = (j.linOrigem + j.linDestino) / 2;
        epCol_ = j.colOrigem;
    }

    // Atualiza direitos de roque
    if (movendo.tipo == TipoPeca::REI) {
        if (ladoAtual_ == Cor::BRANCA) { roqueDisp_[0] = roqueDisp_[1] = false; }
        else                            { roqueDisp_[2] = roqueDisp_[3] = false; }
    }
    // Torre movida ou capturada
    if (j.linOrigem  == 7 && j.colOrigem  == 7) roqueDisp_[0] = false; // h1
    if (j.linOrigem  == 7 && j.colOrigem  == 0) roqueDisp_[1] = false; // a1
    if (j.linOrigem  == 0 && j.colOrigem  == 7) roqueDisp_[2] = false; // h8
    if (j.linOrigem  == 0 && j.colOrigem  == 0) roqueDisp_[3] = false; // a8
    if (j.linDestino == 7 && j.colDestino == 7) roqueDisp_[0] = false;
    if (j.linDestino == 7 && j.colDestino == 0) roqueDisp_[1] = false;
    if (j.linDestino == 0 && j.colDestino == 7) roqueDisp_[2] = false;
    if (j.linDestino == 0 && j.colDestino == 0) roqueDisp_[3] = false;

    ladoAtual_ = oponente(ladoAtual_);
    historicoPosicoes_.push_back(chavePosicao());
}

void Tabuleiro::desfazer() {
    if (historico_.empty()) return;

    if (!historicoPosicoes_.empty())
        historicoPosicoes_.pop_back();

    InfoDesfazer info = historico_.back();
    historico_.pop_back();
    const Jogada& j = info.jogada;

    // Determina peca que foi movida (antes de restaurar)
    Peca movida = grade_[j.linDestino][j.colDestino];
    if (j.promocao != TipoPeca::VAZIA)
        movida.tipo = TipoPeca::PEAO;

    int coldiff = j.colDestino - j.colOrigem;
    bool foiRoque = (movida.tipo == TipoPeca::REI) && (coldiff == 2 || coldiff == -2);

    // Restaura peca movida na origem e limpa o destino
    grade_[j.linOrigem][j.colOrigem]             = movida;
    grade_[j.linDestino][j.colDestino]           = Peca{};
    // Restaura peca capturada (posicao real, suporta en passant)
    grade_[info.linCapturada][info.colCapturada] = info.capturada;

    // Restaura torre se foi roque
    if (foiRoque) {
        int lin = j.linOrigem;
        if (coldiff > 0) {  // rei-lado: torre estava em f, volta para h
            grade_[lin][7] = grade_[lin][5];
            grade_[lin][5] = Peca{};
        } else {            // dama-lado: torre estava em d, volta para a
            grade_[lin][0] = grade_[lin][3];
            grade_[lin][3] = Peca{};
        }
    }

    ladoAtual_  = info.ladoAnterior;
    std::copy(info.roqueDisp, info.roqueDisp + 4, roqueDisp_);
    epLin_      = info.epLin;
    epCol_      = info.epCol;
    meioLances_ = info.meioLances;
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
    Cor eu        = grade_[lin][col].cor;
    int dir       = (eu == Cor::BRANCA) ? -1 : +1;
    int linInicio = (eu == Cor::BRANCA) ? 6 : 1;
    int linPromo  = (eu == Cor::BRANCA) ? 0 : 7;

    // Gera o movimento com todas as 4 subpromocoes, ou simples se nao promover
    auto empurrar = [&](int nl, int nc) {
        if (nl == linPromo) {
            for (TipoPeca promo : {TipoPeca::RAINHA, TipoPeca::TORRE,
                                   TipoPeca::BISPO,  TipoPeca::CAVALO})
                saida.push_back(Jogada{lin, col, nl, nc, promo});
        } else {
            saida.push_back(Jogada{lin, col, nl, nc});
        }
    };

    int umaLinha = lin + dir;
    if (dentroLimites(umaLinha, col) && grade_[umaLinha][col].vazia()) {
        empurrar(umaLinha, col);
        int duasLinhas = lin + 2 * dir;
        if (lin == linInicio && dentroLimites(duasLinhas, col) &&
            grade_[duasLinhas][col].vazia())
            saida.push_back(Jogada{lin, col, duasLinhas, col});
    }

    for (int dc = -1; dc <= 1; dc += 2) {
        int nc = col + dc;
        if (!dentroLimites(umaLinha, nc)) continue;
        const Peca& alvo = grade_[umaLinha][nc];
        if (!alvo.vazia() && alvo.cor != eu) {
            empurrar(umaLinha, nc);
        } else if (umaLinha == epLin_ && nc == epCol_) {
            // En passant: destino e a casa alvo (sempre fora da fila de promocao)
            saida.push_back(Jogada{lin, col, umaLinha, nc});
        }
    }
}

void Tabuleiro::adicionarRoque(Cor lado, std::vector<Jogada>& saida) const {
    int lin  = (lado == Cor::BRANCA) ? 7 : 0;
    int idxK = (lado == Cor::BRANCA) ? 0 : 2;
    int idxQ = (lado == Cor::BRANCA) ? 1 : 3;

    if (grade_[lin][4].tipo != TipoPeca::REI || grade_[lin][4].cor != lado) return;

    // Roque rei-lado (O-O): casas f e g devem estar livres
    if (roqueDisp_[idxK] &&
        grade_[lin][5].vazia() && grade_[lin][6].vazia() &&
        grade_[lin][7].tipo == TipoPeca::TORRE && grade_[lin][7].cor == lado)
        saida.push_back(Jogada{lin, 4, lin, 6});

    // Roque dama-lado (O-O-O): casas b, c e d devem estar livres
    if (roqueDisp_[idxQ] &&
        grade_[lin][3].vazia() && grade_[lin][2].vazia() && grade_[lin][1].vazia() &&
        grade_[lin][0].tipo == TipoPeca::TORRE && grade_[lin][0].cor == lado)
        saida.push_back(Jogada{lin, 4, lin, 2});
}

std::vector<Jogada> Tabuleiro::gerarPseudoLegais(Cor lado) const {
    std::vector<Jogada> jogadas;
    jogadas.reserve(80);

    static const int dirsTorre[4][2]    = {{-1,0},{1,0},{0,-1},{0,1}};
    static const int dirsBispo[4][2]    = {{-1,-1},{-1,1},{1,-1},{1,1}};
    static const int passosCavalo[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},
                                            {1,-2},{1,2},{2,-1},{2,1}};
    static const int passosRei[8][2]    = {{-1,-1},{-1,0},{-1,1},{0,-1},
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
                    adicionarDeslizantes(l, c, dirsTorre, 4, jogadas);
                    adicionarDeslizantes(l, c, dirsBispo, 4, jogadas);
                    break;
                case TipoPeca::REI: adicionarSaltos(l, c, passosRei, 8, jogadas); break;
                default: break;
            }
        }
    }
    adicionarRoque(lado, jogadas);
    return jogadas;
}

// Chave compacta: 64 casas + lado + direitos de roque + coluna en passant.
// Todos esses fatores determinam se duas posicoes sao identicas para a regra FIDE.
std::string Tabuleiro::chavePosicao() const {
    static const char glifo[7] = {'.', 'p', 'n', 'b', 'r', 'q', 'k'};
    std::string chave;
    chave.reserve(70);
    for (int l = 0; l < 8; ++l)
        for (int c = 0; c < 8; ++c) {
            const Peca& p = grade_[l][c];
            char ch = glifo[static_cast<int>(p.tipo)];
            if (p.cor == Cor::BRANCA) ch = static_cast<char>(ch - ('a' - 'A'));
            chave.push_back(ch);
        }
    chave.push_back(ladoAtual_ == Cor::BRANCA ? 'w' : 'b');
    chave.push_back(roqueDisp_[0] ? 'K' : '-');
    chave.push_back(roqueDisp_[1] ? 'Q' : '-');
    chave.push_back(roqueDisp_[2] ? 'k' : '-');
    chave.push_back(roqueDisp_[3] ? 'q' : '-');
    chave.push_back(epCol_ >= 0 ? static_cast<char>('a' + epCol_) : '-');
    return chave;
}

int Tabuleiro::contarRepeticoes() const {
    if (historicoPosicoes_.empty()) return 0;
    const std::string& atual = historicoPosicoes_.back();
    int n = 0;
    for (const std::string& k : historicoPosicoes_)
        if (k == atual) ++n;
    return n;
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
