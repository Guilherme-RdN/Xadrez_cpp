# Xadrez com IA em C++

Jogo de xadrez com interface gráfica em C++17.
Adversário controlado por IA — algoritmo Minimax com poda Alpha-Beta.

## Pré-requisitos

É necessário ter o **MSYS2** com o ambiente **UCRT64** e o **SFML 3**.

1. Instale o MSYS2: [msys2.org](https://www.msys2.org)
2. Abra o terminal **MSYS2 UCRT64** e instale o compilador e o SFML:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-sfml
```

> **Importante:** o ambiente precisa ser o **UCRT64** (não o MINGW64). O compilador e o
> SFML têm que ser do mesmo ambiente — misturar UCRT64 com MINGW64 causa erros de
> runtime (tela branca, fechamento inesperado).

---

## Compilar

No Windows, dê **duplo clique em `build.bat`** (ou rode `.\build.bat` no terminal).

O script compila o jogo em `bin\xadrez.exe` e copia automaticamente todas as
bibliotecas (`.dll`) necessárias para a pasta `bin\`.

---

## Rodar

Dê **duplo clique em `run.bat`**, ou abra diretamente `bin\xadrez.exe`.

> Não é preciso ter o MSYS2 no PATH: as DLLs ficam dentro de `bin\`, ao lado do executável.

---

## Como jogar

1. A janela abre no **menu** — escolha a dificuldade e a cor
2. Clique em **JOGAR**
3. Clique em uma peça para selecioná-la (as casas válidas ficam destacadas em verde)
4. Clique na casa de destino para mover
5. Quando um peão chega na última fileira, uma janela abre para escolher a promoção
6. Para roque: mova o rei duas casas (ex.: rei de `e1` para `g1` = roque pequeno)

---

## Dificuldades

| Nível | Profundidade | Avaliador |
|---|---|---|
| Fácil | 2 | Material (contagem de peças) |
| Médio | 3 | Posicional (posição + centro) |
| Difícil | 4 | Posicional (busca mais profunda) |

---

## Estrutura do projeto

```
include/             headers públicos
  Board.h            tabuleiro, Move, Piece, enums
  GameState.h        xeque, xeque-mate, afogamento, movimentos legais
  Evaluator.h        avaliadores + fábrica (Strategy pattern)
  MinimaxAI.h        algoritmo Minimax com Alpha-Beta
  AIFactory.h        criação de IA por dificuldade (Factory pattern)
  GUI.h              interface gráfica (SFML)
src/
  board/Board.cpp    representação e geração de movimentos (roque, en passant, promoção)
  game/GameState.cpp detecção de estados do jogo
  ai/                motor de IA (avaliadores, Minimax, fábrica)
  ui/GUI.cpp         renderização e interação por mouse
  main.cpp           ponto de entrada
tests/tests.cpp      25 testes unitários
build.bat            compila o jogo e copia as DLLs para bin\
run.bat              executa bin\xadrez.exe
bin/                 saída do build (gerada por build.bat, não versionada)
```

A documentação completa do código está em [DOCUMENTACAO.md](DOCUMENTACAO.md).

---

## Testes

Os testes não dependem do SFML. No terminal **MSYS2 UCRT64**:

```bash
g++ -std=c++17 -Iinclude \
    src/board/Board.cpp src/game/GameState.cpp \
    src/ai/Evaluator.cpp src/ai/MinimaxAI.cpp src/ai/AIFactory.cpp \
    tests/tests.cpp -o bin/tests.exe

./bin/tests.exe
```
