# Xadrez com IA em C++

Jogo de xadrez em C++17 com interface de linha de comando e adversário controlado por
Inteligência Artificial (Minimax com poda Alpha-Beta).

## Estrutura

```
include/        headers públicos (Board, GameState, Evaluator, MinimaxAI, AIFactory, CLI)
src/board/      representação e estado do tabuleiro + geração de movimentos
src/game/       detecção de estados (xeque, xeque-mate, afogamento)
src/ai/         motor Minimax + avaliadores + fábrica de dificuldade
src/ui/         interface CLI
src/main.cpp    ponto de entrada
tests/          testes
```

## Como compilar

### Com CMake

```bash
cmake -S . -B build
cmake --build build
```

Gera dois executáveis em `build/`: `xadrez` (o jogo) e `tests` (os testes).

### Sem CMake (g++ direto)

```bash
g++ -std=c++17 -Iinclude src/board/Board.cpp src/game/GameState.cpp \
    src/ai/Evaluator.cpp src/ai/MinimaxAI.cpp src/ai/AIFactory.cpp \
    src/ui/CLI.cpp src/main.cpp -o xadrez

g++ -std=c++17 -Iinclude src/board/Board.cpp src/game/GameState.cpp \
    src/ai/Evaluator.cpp src/ai/MinimaxAI.cpp src/ai/AIFactory.cpp \
    src/ui/CLI.cpp tests/tests.cpp -o tests
```

## Como jogar

```bash
./xadrez
```

Escolha a dificuldade (Fácil / Médio / Difícil) e a cor. Os lances são digitados em
notação de casas, por exemplo `e2 e4`. Para roque, mova o rei duas casas (`e1 g1`).
Digite `sair` para encerrar.

## Dificuldades

| Nível   | Profundidade | Avaliador  |
|---------|--------------|------------|
| Fácil   | 2            | Material   |
| Médio   | 3            | Posicional |
| Difícil | 4            | Posicional |

A explicação completa do código está em [DOCUMENTACAO.md](DOCUMENTACAO.md).
