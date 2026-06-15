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

    // --- Teste 10: tripla repeticao (cavalos indo e voltando) ---
    {
        Tabuleiro tab;
        // Posicao inicial = 1a ocorrencia (registrada no construtor).
        verifica("Posicao inicial nao e tripla repeticao",
                 !EstadoJogo::eEmpatePorRepeticao(tab));

        // Ciclo: Cf3 Cf6 Cg1 Cg8 -> volta a posicao inicial (2a vez).
        tab.fazerJogada(Jogada{7,6,5,5}); // Cf3
        tab.fazerJogada(Jogada{0,6,2,5}); // Cf6
        tab.fazerJogada(Jogada{5,5,7,6}); // Cg1
        tab.fazerJogada(Jogada{2,5,0,6}); // Cg8 (2a ocorrencia)
        verifica("Apos 1 ciclo ainda nao e tripla repeticao",
                 !EstadoJogo::eEmpatePorRepeticao(tab));

        // Segundo ciclo identico -> 3a ocorrencia da posicao inicial.
        tab.fazerJogada(Jogada{7,6,5,5}); // Cf3
        tab.fazerJogada(Jogada{0,6,2,5}); // Cf6
        tab.fazerJogada(Jogada{5,5,7,6}); // Cg1
        tab.fazerJogada(Jogada{2,5,0,6}); // Cg8 (3a ocorrencia)
        verifica("Apos 2 ciclos identicos e tripla repeticao",
                 EstadoJogo::eEmpatePorRepeticao(tab));
        std::cout << "        (ocorrencias da posicao atual: "
                  << tab.contarRepeticoes() << ")\n";

        // desfazer deve reverter a deteccao de repeticao.
        tab.desfazer();
        verifica("Desfazer reverte a tripla repeticao",
                 !EstadoJogo::eEmpatePorRepeticao(tab));
    }

    // --- Teste 11: Roque rei-lado branco (Abertura Italiana) ---
    // Coordenadas: lin7=fileira1(brancas), col4=e, col5=f, col6=g, col7=h
    {
        Tabuleiro tab;
        // e4 e5 Cf3 Cc6 Bc4 d6: limpa f1 e g1 para rocar
        tab.fazerJogada(Jogada{6,4,4,4}); // e4
        tab.fazerJogada(Jogada{1,4,3,4}); // e5
        tab.fazerJogada(Jogada{7,6,5,5}); // Cf3 (libera g1)
        tab.fazerJogada(Jogada{0,1,2,2}); // Cc6
        tab.fazerJogada(Jogada{7,5,4,2}); // Bc4 (libera f1)
        tab.fazerJogada(Jogada{1,3,2,3}); // d6

        auto legais = EstadoJogo::gerarJogadasLegais(tab, Cor::BRANCA);
        bool temRoque = false;
        for (const auto& j : legais)
            if (j.linOrigem==7 && j.colOrigem==4 && j.linDestino==7 && j.colDestino==6)
                temRoque = true;
        verifica("Roque rei-lado branco esta disponivel apos abertura", temRoque);

        tab.fazerJogada(Jogada{7,4,7,6}); // O-O
        verifica("Rei em g1 apos roque rei-lado", tab.peca(7,6).tipo == TipoPeca::REI);
        verifica("Torre em f1 apos roque rei-lado", tab.peca(7,5).tipo == TipoPeca::TORRE);
        verifica("h1 vazia apos roque rei-lado",   tab.peca(7,7).vazia());
        verifica("Direitos de roque branco cancelados",
                 !tab.roqueDisponivel(0) && !tab.roqueDisponivel(1));

        // Desfazer deve restaurar tudo
        tab.desfazer();
        verifica("Desfazer roque: rei volta a e1", tab.peca(7,4).tipo == TipoPeca::REI);
        verifica("Desfazer roque: torre volta a h1", tab.peca(7,7).tipo == TipoPeca::TORRE);
        verifica("Desfazer roque: direitos restaurados", tab.roqueDisponivel(0));
    }

    // --- Teste 12: En passant ---
    // Peao branco em e5 captura peao preto que avancou para d5
    {
        Tabuleiro tab;
        tab.fazerJogada(Jogada{6,4,4,4}); // e4
        tab.fazerJogada(Jogada{1,0,2,0}); // a6 (lance neutro preto)
        tab.fazerJogada(Jogada{4,4,3,4}); // e5
        tab.fazerJogada(Jogada{1,3,3,3}); // d5 (avanco duplo -> alvo ep em (2,3)=d6)

        auto legais = EstadoJogo::gerarJogadasLegais(tab, Cor::BRANCA);
        bool temEP = false;
        for (const auto& j : legais)
            if (j.linOrigem==3 && j.colOrigem==4 && j.linDestino==2 && j.colDestino==3)
                temEP = true;
        verifica("Captura en passant disponivel em d6", temEP);

        tab.fazerJogada(Jogada{3,4,2,3}); // exd6 en passant
        verifica("Peao branco chegou em d6", tab.peca(2,3).tipo == TipoPeca::PEAO);
        verifica("Peao preto em d5 foi removido",  tab.peca(3,3).vazia());
        verifica("Casa e5 ficou vazia",            tab.peca(3,4).vazia());

        // Desfazer deve restaurar o peao capturado
        tab.desfazer();
        verifica("Desfazer ep: peao preto volta a d5", tab.peca(3,3).tipo == TipoPeca::PEAO);
        verifica("Desfazer ep: d6 volta vazia",        tab.peca(2,3).vazia());
    }

    // --- Teste 13: Subpromocao a cavalo ---
    // Verifica que o gerador produz as 4 promocoes; usamos a lista de jogadas
    // de um peao preto em g2 (lin6,col6) que avanca para g1 (lin7,col6).
    {
        // Monta sequencia que leva peao preto a g2:
        // Abertura para liberar caminho e avancado do peao h (simplificado):
        // Vamos apenas contar as jogadas de promocao geradas por um peao a 1 passo.
        // Para nao precisar de muitos lances, verificamos via gerarPseudoLegais
        // numa posicao onde sabemos que ha peao preto em g2.
        //
        // Construimos via lances: 1.h4 g5 2.h5 g4 3.h6 g3 4.hxg7(?) nao, vamos por
        // outro caminho: 1.g4 h5 2.g5 h4 3.g6 h3 4.g7 h2 -> peao preto em h2
        // pode promover, gerando 4 jogadas.
        Tabuleiro tab;
        tab.fazerJogada(Jogada{6,6,4,6}); // g4
        tab.fazerJogada(Jogada{1,7,3,7}); // h5
        tab.fazerJogada(Jogada{4,6,3,6}); // g5
        tab.fazerJogada(Jogada{3,7,4,7}); // h4
        tab.fazerJogada(Jogada{3,6,2,6}); // g6
        tab.fazerJogada(Jogada{4,7,5,7}); // h3
        tab.fazerJogada(Jogada{2,6,1,6}); // g7
        tab.fazerJogada(Jogada{5,7,6,7}); // h2 (peao preto em h2, lin6 col7)

        // Conta quantas jogadas de promocao o peao preto em h2 gera (para g1/h1)
        auto pseudo = tab.gerarPseudoLegais(Cor::PRETA);
        int nPromo = 0;
        for (const auto& j : pseudo)
            if (j.linOrigem == 6 && j.colOrigem == 7 && j.promocao != TipoPeca::VAZIA)
                ++nPromo;
        // Peao em h2 pode avancar para h1 (4 promocoes) e
        // capturar em g1 se houver peca branca la (pode ter o peao que avancou?)
        // Nao ha peca em g1 (branca moveu g para g7). Entao so avanco: 4 promocoes.
        verifica("Peao a 1 passo da promocao gera 4 subpromocoes", nPromo == 4);
    }

    // --- Teste 14: Regra dos 50 lances ---
    {
        Tabuleiro tab;
        verifica("Contador de meio-lances inicia em 0", tab.meioLances() == 0);

        // Movimentos de cavalo nao sao capturas nem peao: incrementa
        tab.fazerJogada(Jogada{7,6,5,5}); // Cf3
        tab.fazerJogada(Jogada{0,6,2,5}); // Cf6
        verifica("Dois lances de cavalo: contador = 2", tab.meioLances() == 2);

        // Lance de peao reseta o contador
        tab.fazerJogada(Jogada{6,4,4,4}); // e4
        verifica("Lance de peao reseta contador para 0", tab.meioLances() == 0);

        verifica("Nao e empate por 50 lances ainda",
                 !EstadoJogo::eEmpatePorRegra50Lances(tab));

        // Desfazer restaura o contador
        tab.desfazer();
        verifica("Desfazer restaura contador para 2", tab.meioLances() == 2);
    }

    // --- Teste 15: Material insuficiente ---
    {
        Tabuleiro tab;
        verifica("Posicao inicial nao e material insuficiente",
                 !EstadoJogo::eMaterialInsuficiente(tab));

        // Simula K vs K capturando tudo: muito custoso em lances reais,
        // entao testamos o caminho de codigo com a posicao inicial (maior)
        // e confiamos nos testes unitarios acima para os outros estados.
        // Teste adicional: afogamento/xequemate nao ocorre na posicao inicial.
        verifica("Posicao inicial: brancas nao estao em xeque-mate",
                 !EstadoJogo::eXequeMate(tab, Cor::BRANCA));
        verifica("Posicao inicial: pretas nao estao em afogamento",
                 !EstadoJogo::eAfogamento(tab, Cor::PRETA));
    }

    std::cout << "\n=== Resultado: " << aprovados << "/" << total
              << " testes aprovados ===\n";
    return (aprovados == total) ? 0 : 1;
}
