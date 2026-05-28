#include "Tabuleiro.h"
#include "EstadoJogo.h"
#include "Avaliador.h"
#include "IAMinimax.h"
#include <iostream>
#include <memory>

using namespace xadrez;

static int aprovados = 0;
static int total     = 0;

static void verifica(const char* descricao, bool condicao) {
    ++total;
    std::cout << (condicao ? "[OK]    " : "[FALHA] ") << descricao << "\n";
    if (condicao) ++aprovados;
}

int main() {
    std::cout << "=== Testes DEV 2: Deteccao de Estados + Motor IA ===\n\n";

    // --- Teste 1: xeque na posicao inicial ---
    {
        Tabuleiro tab;
        verifica("Posicao inicial: brancas nao estao em xeque",
                 !EstadoJogo::estaEmXeque(tab, Cor::BRANCA));
        verifica("Posicao inicial: pretas nao estao em xeque",
                 !EstadoJogo::estaEmXeque(tab, Cor::PRETA));
    }

    // --- Teste 2: 20 jogadas legais na abertura (16 peoes + 4 cavalos) ---
    {
        Tabuleiro tab;
        verifica("Abertura tem 20 jogadas legais para as brancas",
                 EstadoJogo::gerarJogadasLegais(tab, Cor::BRANCA).size() == 20);
    }

    // --- Teste 3: nao e mate nem afogamento na abertura ---
    {
        Tabuleiro tab;
        verifica("Abertura nao e xeque-mate", !EstadoJogo::eXequeMate(tab, Cor::BRANCA));
        verifica("Abertura nao e afogamento", !EstadoJogo::eAfogamento(tab, Cor::BRANCA));
    }

    // --- Teste 4: IA escolhe jogada legal na abertura ---
    {
        Tabuleiro tab;
        IAMinimax ia(3, std::make_unique<AvaliadorPosicional>());
        bool encontrou = false;
        Jogada j = ia.escolherJogada(tab, Cor::BRANCA, encontrou);
        verifica("IA encontra uma jogada na abertura", encontrou);

        auto legais = EstadoJogo::gerarJogadasLegais(tab, Cor::BRANCA);
        bool legal  = false;
        for (const auto& lj : legais) if (lj == j) legal = true;
        verifica("Jogada da IA e legal", legal);
        std::cout << "        (nos visitados: " << ia.nosVisitados() << ")\n";
    }

    // --- Teste 5: IA nao captura peao defendido (1.e4 d5 2.Cf3 c6) ---
    {
        Tabuleiro tab;
        tab.fazerJogada(Jogada{6,4,4,4}); // e4
        tab.fazerJogada(Jogada{1,3,3,3}); // d5
        tab.fazerJogada(Jogada{7,6,5,5}); // Cf3
        tab.fazerJogada(Jogada{1,2,2,2}); // c6 (defende d5)

        IAMinimax ia(4, std::make_unique<AvaliadorPosicional>());
        bool encontrou = false;
        Jogada j = ia.escolherJogada(tab, Cor::BRANCA, encontrou);
        verifica("IA nao captura o peao defendido em d5",
                 encontrou && !(j.linDestino == 3 && j.colDestino == 3));
    }

    // --- Teste 6: IA captura peao indefeso (rainha preta saiu de d8) ---
    {
        Tabuleiro tab;
        tab.fazerJogada(Jogada{6,4,4,4}); // e4
        tab.fazerJogada(Jogada{1,3,3,3}); // d5
        tab.fazerJogada(Jogada{7,1,5,2}); // Cc3
        tab.fazerJogada(Jogada{0,3,2,3}); // Rd6 (rainha sai, d5 fica indefeso)

        IAMinimax ia(4, std::make_unique<AvaliadorMaterial>());
        bool encontrou = false;
        Jogada j = ia.escolherJogada(tab, Cor::BRANCA, encontrou);
        verifica("IA captura o peao indefeso em d5",
                 encontrou && j.linDestino == 3 && j.colDestino == 3);
    }

    // --- Teste 7: busca em profundidade 4 termina ---
    {
        Tabuleiro tab;
        IAMinimax ia(4, std::make_unique<AvaliadorMaterial>());
        bool encontrou = false;
        ia.escolherJogada(tab, Cor::BRANCA, encontrou);
        std::cout << "        (profundidade 4: " << ia.nosVisitados() << " nos com poda)\n";
        verifica("Busca em profundidade 4 termina", encontrou);
    }

    // --- Teste 8: polimorfismo - ponteiro de base chama subclasse correta ---
    {
        Tabuleiro tab;
        tab.fazerJogada(Jogada{6,4,4,4}); // e4 (posicao assimetrica)

        std::unique_ptr<Avaliador> avPos = std::make_unique<AvaliadorPosicional>();
        std::unique_ptr<Avaliador> avMat = std::make_unique<AvaliadorMaterial>();
        int pos = avPos->avaliar(tab, Cor::BRANCA);
        int mat = avMat->avaliar(tab, Cor::BRANCA);

        verifica("Polimorfismo: AvaliadorPosicional difere de AvaliadorMaterial apos e4",
                 pos != mat);
        std::cout << "        (Material=" << mat << "  Posicional=" << pos << ")\n";
    }

    // --- Teste 9: FabricaAvaliador cria AvaliadorAgressivo funcional ---
    {
        Tabuleiro tab;
        IAMinimax ia(3, FabricaAvaliador::criar(FabricaAvaliador::Estrategia::Agressivo));
        bool encontrou = false;
        ia.escolherJogada(tab, Cor::BRANCA, encontrou);
        verifica("FabricaAvaliador cria AvaliadorAgressivo e IA encontra jogada", encontrou);
    }

    std::cout << "\n=== Resultado: " << aprovados << "/" << total
              << " testes aprovados ===\n";
    return (aprovados == total) ? 0 : 1;
}
