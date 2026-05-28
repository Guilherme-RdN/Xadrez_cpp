#ifndef XADREZ_AVALIADOR_H
#define XADREZ_AVALIADOR_H

#include "Tabuleiro.h"
#include "Types.h"
#include <memory>

namespace xadrez {

// Classe base abstrata: define o contrato de avaliacao de posicao.
class Avaliador {
public:
    virtual ~Avaliador() = default;
    virtual int avaliar(const Tabuleiro& tab, Cor lado) const = 0;
    virtual const char* nome() const = 0;
};

// Estrategia 1: so valor de material.
class AvaliadorMaterial : public Avaliador {
public:
    int avaliar(const Tabuleiro& tab, Cor lado) const override;
    const char* nome() const override { return "Material"; }
};

// Estrategia 2: material + bonus de posicao no tabuleiro.
class AvaliadorPosicional : public Avaliador {
public:
    int avaliar(const Tabuleiro& tab, Cor lado) const override;
    const char* nome() const override { return "Material+Posicao"; }
};

// Estrategia 3: material + bonus por mobilidade (ataques gerados).
class AvaliadorAgressivo : public Avaliador {
public:
    int avaliar(const Tabuleiro& tab, Cor lado) const override;
    const char* nome() const override { return "Material+Mobilidade"; }
};

// Factory: cria o avaliador sem expor as subclasses ao chamador.
class FabricaAvaliador {
public:
    enum class Estrategia { Material, Posicional, Agressivo };
    static std::unique_ptr<Avaliador> criar(Estrategia e);
};

} // namespace xadrez

#endif // XADREZ_AVALIADOR_H
