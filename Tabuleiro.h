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

    // Direitos de roque: [0]=branca-rei, [1]=branca-dama, [2]=preta-rei, [3]=preta-dama
    bool roqueDisponivel(int idx) const { return roqueDisp_[idx]; }
    // Alvo de en passant (-1 = nenhum)
    int  enPassantLin() const { return epLin_; }
    int  enPassantCol() const { return epCol_; }
    // Contador de meios-lances para a regra dos 50 lances
    int  meioLances() const { return meioLances_; }

    void fazerJogada(const Jogada& j);
    void desfazer();

    std::vector<Jogada> gerarPseudoLegais(Cor lado) const;
    bool encontrarRei(Cor lado, int& lin, int& col) const;

    // Tripla repeticao
    int  contarRepeticoes() const;
    bool eRepeticaoTripla() const { return contarRepeticoes() >= 3; }

    std::string texto() const;

private:
    struct InfoDesfazer {
        Jogada jogada;
        Peca   capturada;
        int    linCapturada;  // posicao real da peca capturada (difere para en passant)
        int    colCapturada;
        Cor    ladoAnterior;
        bool   roqueDisp[4]; // direitos de roque anteriores
        int    epLin;        // alvo en passant anterior
        int    epCol;
        int    meioLances;   // contador antes da jogada
    };

    Peca grade_[8][8];
    Cor  ladoAtual_  = Cor::BRANCA;
    bool roqueDisp_[4] = {true, true, true, true};
    int  epLin_      = -1;
    int  epCol_      = -1;
    int  meioLances_ = 0;

    std::vector<InfoDesfazer> historico_;
    std::vector<std::string>  historicoPosicoes_;

    std::string chavePosicao() const;

    void adicionarDeslizantes(int lin, int col, const int dirs[][2], int n,
                               std::vector<Jogada>& saida) const;
    void adicionarSaltos(int lin, int col, const int passos[][2], int n,
                          std::vector<Jogada>& saida) const;
    void adicionarPeao(int lin, int col, std::vector<Jogada>& saida) const;
    void adicionarRoque(Cor lado, std::vector<Jogada>& saida) const;
};

} // namespace xadrez

#endif // XADREZ_TABULEIRO_H
