# Xadrez com IA em C++

Jogo de xadrez com interface gráfica em C++17.
Adversário controlado por IA — algoritmo Minimax com poda Alpha-Beta.

## Pré-requisitos

| Ferramenta | Versão mínima | Instalação |
|---|---|---|
| MSYS2 | qualquer | [msys2.org](https://www.msys2.org) |
| g++ (MinGW-w64) | 12+ | via MSYS2 |
| SFML | 3.x | via MSYS2 |

> **Importante:** todos os comandos abaixo devem ser executados no terminal **MSYS2 MinGW 64-bit**
> (não o CMD ou PowerShell padrão do Windows).

---

## Instalação (uma vez só)

Abra o terminal **MSYS2 MinGW 64-bit** e execute:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-sfml
```

---

## Compilar

No terminal **MSYS2 MinGW 64-bit**, dentro da pasta do projeto:

```bash
g++ -std=c++17 -Iinclude -I"C:/msys64/mingw64/include" \
    src/board/Board.cpp src/game/GameState.cpp \
    src/ai/Evaluator.cpp src/ai/MinimaxAI.cpp src/ai/AIFactory.cpp \
    src/ui/GUI.cpp src/main.cpp \
    -L"C:/msys64/mingw64/lib" -lsfml-graphics -lsfml-window -lsfml-system \
    -o xadrez.exe
```

Ou, no Windows (CMD/PowerShell), clique duas vezes em **`build.bat`**.

---

## Rodar

**Opção 1 — mais simples:** clique duas vezes em **`run.bat`**

**Opção 2 — terminal MSYS2 MinGW 64-bit:**
```bash
./xadrez.exe
```

> **Por que não funciona com duplo clique direto no exe?**
> O executável precisa das DLLs do SFML que ficam na pasta `C:\msys64\mingw64\bin`.
> O `run.bat` configura esse caminho automaticamente.

---

## Como jogar

1. A janela abre no **menu** — escolha a dificuldade e a cor
2. Clique em **JOGAR**
3. Clique em uma peça para selecioná-la (casas válidas ficam destacadas em verde)
4. Clique na casa de destino para mover
5. Quando um peão chega na última fileira, uma janela abre para escolher a promoção
6. Para roque: mova o rei duas casas (ex.: `e1→g1` para roque pequeno das brancas)

---

## Dificuldades

| Nível | Profundidade | Avaliador |
|---|---|---|
| Fácil | 2 | Material (contagem de peças) |
| Médio | 3 | Posicional (posição + centro) |
| Difícil | 4 | Posicional (mais fundo) |

---

## Estrutura do projeto

```
include/            headers públicos
  Board.h           tabuleiro, Move, Piece, enums
  GameState.h       xeque, xeque-mate, afogamento, movimentos legais
  Evaluator.h       avaliadores + fábrica (Strategy pattern)
  MinimaxAI.h       algoritmo Minimax com Alpha-Beta
  AIFactory.h       criação de IA por dificuldade (Factory pattern)
  GUI.h             interface gráfica SFML
src/
  board/Board.cpp   representação e geração de movimentos (roque, en passant, promoção)
  game/GameState.cpp detecção de estados
  ai/               motor de IA
  ui/GUI.cpp        renderização e interação (mouse)
  main.cpp          ponto de entrada
tests/tests.cpp     25 testes unitários
build.bat           script de compilação (Windows)
run.bat             script de execução   (Windows)
```

A documentação completa está em [DOCUMENTACAO.md](DOCUMENTACAO.md).

---

## Compilar e rodar os testes

```bash
g++ -std=c++17 -Iinclude \
    src/board/Board.cpp src/game/GameState.cpp \
    src/ai/Evaluator.cpp src/ai/MinimaxAI.cpp src/ai/AIFactory.cpp \
    src/ui/CLI.cpp tests/tests.cpp -o tests.exe

./tests.exe
```
