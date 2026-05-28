#ifndef XADREZ_TYPES_H
#define XADREZ_TYPES_H

#include <cstdint>

namespace xadrez {

enum class Cor : std::int8_t {
    BRANCA = 0,
    PRETA  = 1,
    NENHUMA = 2
};

enum class TipoPeca : std::int8_t {
    VAZIA  = 0,
    PEAO   = 1,
    CAVALO = 2,
    BISPO  = 3,
    TORRE  = 4,
    RAINHA = 5,
    REI    = 6
};

inline Cor oponente(Cor c) {
    return (c == Cor::BRANCA) ? Cor::PRETA : Cor::BRANCA;
}

struct Peca {
    TipoPeca tipo = TipoPeca::VAZIA;
    Cor      cor  = Cor::NENHUMA;

    bool vazia() const { return tipo == TipoPeca::VAZIA; }
};

struct Jogada {
    int      linOrigem  = 0;
    int      colOrigem  = 0;
    int      linDestino = 0;
    int      colDestino = 0;
    TipoPeca promocao   = TipoPeca::VAZIA;

    bool operator==(const Jogada& outra) const {
        return linOrigem  == outra.linOrigem  &&
               colOrigem  == outra.colOrigem  &&
               linDestino == outra.linDestino &&
               colDestino == outra.colDestino &&
               promocao   == outra.promocao;
    }
};

} // namespace xadrez

#endif // XADREZ_TYPES_H
