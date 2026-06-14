# Xadrez com IA em C++

Jogo de xadrez com interface gráfica em C++17. Adversário controlado por
Inteligência Artificial (Minimax com poda Alpha-Beta).

## Estrutura

```
include/        headers públicos (Board, GameState, Evaluator, MinimaxAI, AIFactory, GUI)
src/board/      representação e estado do tabuleiro + geração de movimentos
src/game/       detecção de estados (xeque, xeque-mate, afogamento)
src/ai/         motor Minimax + avaliadores + fábrica de dificuldade
src/ui/         interface gráfica (SFML 3)
src/main.cpp    ponto de entrada
tests/          testes unitários
```

## Dependências

- C++17
- [SFML 3](https://www.sfml-dev.org/) — interface gráfica

### Instalar SFML (Windows com MSYS2)

```bash
pacman -S mingw-w64-x86_64-sfml
```

## Como compilar

```bash
g++ -std=c++17 -Iinclude -I"C:/msys64/mingw64/include" \
    src/board/Board.cpp src/game/GameState.cpp \
    src/ai/Evaluator.cpp src/ai/MinimaxAI.cpp src/ai/AIFactory.cpp \
    src/ui/GUI.cpp src/main.cpp \
    -L"C:/msys64/mingw64/lib" -lsfml-graphics -lsfml-window -lsfml-system \
    -o xadrez.exe
```

### Compilar os testes (sem SFML)

```bash
g++ -std=c++17 -Iinclude src/board/Board.cpp src/game/GameState.cpp \
    src/ai/Evaluator.cpp src/ai/MinimaxAI.cpp src/ai/AIFactory.cpp \
    src/ui/CLI.cpp tests/tests.cpp -o tests.exe
```

## Como jogar

```bash
./xadrez.exe
```

1. Escolha a dificuldade (Fácil / Médio / Difícil)
2. Escolha jogar com as Brancas ou Pretas
3. Clique em **JOGAR**

**Controles:**
- Clique na peça para selecioná-la (as casas válidas ficam destacadas)
- Clique na casa de destino para mover
- Na promoção, uma janela aparece para escolher a peça (Dama, Torre, Bispo, Cavalo)

## Dificuldades

| Nível   | Profundidade | Avaliador  |
|---------|--------------|------------|
| Fácil   | 2            | Material   |
| Médio   | 3            | Posicional |
| Difícil | 4            | Posicional |

A explicação completa do código está em [DOCUMENTACAO.md](DOCUMENTACAO.md).
