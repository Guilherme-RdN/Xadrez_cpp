#ifndef XADREZ_ESTADO_JOGO_H
#define XADREZ_ESTADO_JOGO_H

#include "Tabuleiro.h"
#include "Types.h"
#include <vector>

namespace xadrez {

// Deteccao de estados do jogo. Metodos estaticos: nao guarda estado proprio.
class EstadoJogo {
public:
    static bool casaEstaAtacada(const Tabuleiro& tab, int lin, int col, Cor atacante);
    static bool estaEmXeque(const Tabuleiro& tab, Cor lado);
    static std::vector<Jogada> gerarJogadasLegais(Tabuleiro& tab, Cor lado);
    static bool eXequeMate(Tabuleiro& tab, Cor lado);
    static bool eAfogamento(Tabuleiro& tab, Cor lado);

    // Empates
    static bool eEmpatePorRepeticao(const Tabuleiro& tab);
    static bool eEmpatePorRegra50Lances(const Tabuleiro& tab);
    static bool eMaterialInsuficiente(const Tabuleiro& tab);
};

} // namespace xadrez

#endif // XADREZ_ESTADO_JOGO_H
