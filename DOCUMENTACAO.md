# Documentação Técnica — Xadrez com IA em C++

Este documento explica **todo o código** do projeto, arquivo por arquivo, com o porquê
de cada decisão. O objetivo é permitir defender cada linha na apresentação: o que faz,
por que está escrito assim e qual conceito de Programação Orientada a Objetos (POO) ela
demonstra.

---

## 1. Visão geral

O projeto é um jogo de xadrez jogável no terminal, em **C++17**, contra um adversário
controlado por IA. A IA usa o algoritmo **Minimax com poda Alpha-Beta** — a mesma base
conceitual de motores profissionais.

O sistema está dividido em módulos independentes, cada um com uma responsabilidade única:

| Módulo | Arquivos | Responsabilidade |
|---|---|---|
| Tipos + Tabuleiro | `Board.h`, `Board.cpp` | Representação do jogo, estado, geração de movimentos |
| Estados do jogo | `GameState.h/.cpp` | Xeque, xeque-mate, afogamento, filtragem de legais |
| Avaliação | `Evaluator.h/.cpp` | Converter uma posição em um número (o "cérebro") |
| Busca | `MinimaxAI.h/.cpp` | Minimax + Alpha-Beta: escolher o melhor lance |
| Dificuldade | `AIFactory.h/.cpp` | Montar a IA conforme o nível escolhido |
| Interface | `CLI.h/.cpp`, `main.cpp` | Menu, leitura de lances, laço da partida |
| Testes | `tests/tests.cpp` | Garantir que tudo funciona |

---

## 2. Mapeamento com as fases do roteiro

| Fase do roteiro | O que pedia | Onde está |
|---|---|---|
| Fase 1 — Tabuleiro | `enum`, `struct`, `Board`, impressão | `Board.h`, `Board.cpp` |
| Fase 2 — Movimentos | geração por peça + regras especiais | `Board::generatePseudoLegalMoves` e helpers |
| Fase 3 — Estados | xeque / mate / afogamento | `GameState` |
| Fase 4 — IA | Minimax + Alpha-Beta + avaliação | `MinimaxAI`, `Evaluator` |
| Fase 5 — CLI/Integração | laço, fim de jogo, sair/reiniciar | `CLI`, `main.cpp` |

Regras especiais pedidas como melhoria (roque, en passant, promoção) **estão todas
implementadas**.

---

## 3. Como compilar e jogar

```bash
cmake -S . -B build
cmake --build build
./build/xadrez      # jogo
./build/tests       # testes
```

Sem CMake, há a linha `g++` equivalente no `README.md`.

No jogo: escolha a dificuldade (1/2/3), a cor, e digite lances como `e2 e4`. Roque é
mover o rei duas casas (`e1 g1`). `sair` encerra.

---

## 4. Convenções fundamentais

Antes do código, dois acordos que valem para tudo:

**Coordenadas.** O tabuleiro é uma matriz `[linha][coluna]`, ambas de 0 a 7.

- `linha 0` = fileira 8 (peças pretas em cima); `linha 7` = fileira 1 (brancas embaixo).
- `coluna 0` = coluna `a`; `coluna 7` = coluna `h`.
- As brancas sobem (linha **diminui**); as pretas descem (linha **aumenta**).

**Enums fortemente tipados.** `PieceType` e `Color` são `enum class`. Diferente de um
`enum` antigo, eles não se convertem implicitamente para `int`, o que evita comparar
acidentalmente uma cor com um tipo de peça. É segurança de tipo em tempo de compilação.

---

## 5. `include/Board.h` — tipos e a interface do tabuleiro

Este arquivo define os tipos básicos do jogo (originais do projeto, em inglês) e a classe
`Board`. Aqui também estão as **extensões** necessárias para a IA funcionar.

### 5.1 Enums

```cpp
enum class PieceType { NONE, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };
enum class Color { NONE, WHITE, BLACK };
```

`NONE` representa casa vazia / ausência de cor. Usar `enum class` (em vez de `int`) deixa
o código auto-documentado: `PieceType::QUEEN` é muito mais claro que `5`.

### 5.2 Função `opponent`

```cpp
inline Color opponent(Color c) {
    return (c == Color::WHITE) ? Color::BLACK : Color::WHITE;
}
```

Inverte a cor. É usada o tempo todo no Minimax para alternar entre quem maximiza e quem
minimiza. É `inline` porque é minúscula e chamada com altíssima frequência — evita o custo
de uma chamada de função.

### 5.3 `struct Piece`

```cpp
struct Piece {
    PieceType type = PieceType::NONE;
    Color color = Color::NONE;
    bool isEmpty() const { return type == PieceType::NONE; }
};
```

Uma casa guarda **o que** é a peça e **de quem** ela é. É um `struct` simples e leve de
propósito: o tabuleiro guarda 64 deles, então mantê-lo pequeno importa para a velocidade
da busca. O método `isEmpty()` é um pequeno auxílio de legibilidade — `p.isEmpty()` lê
melhor que `p.type == PieceType::NONE` repetido em dezenas de lugares. É `const` porque só
consulta, não altera.

### 5.4 `struct Move` e a sobrecarga de operador

```cpp
enum class MoveType { Normal, Castle, EnPassant };

struct Move {
    int fromRow = 0, fromCol = 0, toRow = 0, toCol = 0;
    PieceType promotion = PieceType::NONE;
    MoveType type = MoveType::Normal;

    bool operator==(const Move& other) const {
        return fromRow == other.fromRow && fromCol == other.fromCol &&
               toRow == other.toRow && toCol == other.toCol &&
               promotion == other.promotion;
    }
};
```

Um lance é "de uma casa para outra". Os campos extras carregam informação que a origem e o
destino sozinhos não revelam:

- `promotion`: em que peça o peão vira ao promover (`NONE` quando não é promoção).
- `type`: marca se o lance é roque ou en passant — o tabuleiro precisa saber para mover
  também a torre (roque) ou remover o peão capturado de lado (en passant).

A **sobrecarga de `operator==`** (conceito de POO) deixa comparar dois lances com `a == b`.
Ela compara origem, destino e promoção — o suficiente para casar o lance digitado pelo
jogador com um lance da lista de jogadas legais. Os valores padrão em todos os campos
permitem criar um lance "vazio" (`Move{}`) e mantêm compatível a forma `Move{6,4,4,4}`.

### 5.5 A classe `Board` — parte privada

```cpp
private:
    Piece board[8][8];
    Color sideToMove_ = Color::WHITE;
    bool castleKingSide[3]  = {false, true, true};
    bool castleQueenSide[3] = {false, true, true};
    int epRow = -1, epCol = -1;
    struct Undo { Move move; Piece captured; Color prevSide; bool ck[3]; bool cq[3]; int epR; int epC; };
    std::vector<Undo> history;
```

Tudo aqui é **privado** (encapsulamento): o mundo de fora nunca mexe direto na matriz; só
fala com a classe pelos métodos públicos. Isso garante que o estado nunca fique
inconsistente.

- `board[8][8]`: o tabuleiro em si.
- `sideToMove_`: de quem é a vez. Foi adicionado porque a IA precisa saber quem joga.
- `castleKingSide` / `castleQueenSide`: direitos de roque, indexados pelo valor da cor
  (`WHITE` = 1, `BLACK` = 2; o índice 0 = `NONE` nunca é usado). Começam `true` para as
  cores reais. Viram `false` assim que o rei ou a torre correspondente se move.
- `epRow` / `epCol`: a casa "atrás" de um peão que acabou de avançar duas casas — o único
  lugar onde uma captura en passant pode acontecer no próximo lance. `-1` = não há.
- `history` + `struct Undo`: uma **pilha** de tudo que é preciso para desfazer um lance.
  Esse é o coração que torna o Minimax viável (ver seção 11).

### 5.6 A classe `Board` — parte pública

Mantém os métodos originais do projeto (`makeMove`, `undoMove`, `isOccupied`, `getPiece`,
`printBoard`) e acrescenta o que a IA exige:

```cpp
Color sideToMove() const { return sideToMove_; }
bool inBounds(int row, int col) const;
bool findKing(Color color, int& row, int& col) const;
std::vector<Move> generatePseudoLegalMoves(Color color) const;
```

`sideToMove()` é um *getter* `const`: dá acesso de leitura sem expor o membro. `findKing`
devolve a posição do rei por **referência** (`int&`) e retorna `bool` indicando se achou —
um padrão clássico em C++ para "devolver dois valores".

---

## 6. `src/board/Board.cpp` — o tabuleiro por dentro

### 6.1 Construtor — posição inicial

Limpa as 64 casas e posiciona peões e peças de trás. A ordem da fileira de trás
(`backRank`) segue as colunas `a..h`: torre, cavalo, bispo, rainha, rei, bispo, cavalo,
torre.

### 6.2 `makeMove` — aplicar um lance (e guardar como desfazer)

Esta é a versão completa do método (o esqueleto original só movia a peça). Passo a passo:

1. **Salva um `Undo`** com o estado atual: o lance, o lado da vez, os direitos de roque e a
   casa de en passant. Isso é empilhado em `history`.
2. **Captura.** Em en passant, a peça capturada **não** está na casa de destino, e sim ao
   lado (mesma linha de origem, coluna de destino) — então ela é guardada e removida de lá.
   Nos demais lances, o capturado é o que estava no destino.
3. **Move a peça** da origem para o destino e esvazia a origem.
4. **Promoção:** se `promotion` não é `NONE`, troca o tipo da peça no destino.
5. **Roque:** se o lance é `Castle`, move **também a torre** (torre do lado do rei: de `h`
   para `f`; do lado da dama: de `a` para `d`).
6. **Marca en passant:** se um peão andou duas casas, registra a casa intermediária em
   `epRow/epCol` para o próximo lance. Caso contrário, zera.
7. **Atualiza direitos de roque:** se o rei moveu, perde os dois lados; se uma torre moveu
   da casa de origem, perde aquele lado; se uma torre foi capturada no canto, o dono perde
   aquele lado.
8. **Troca a vez** com `opponent`.

### 6.3 `undoMove` — desfazer

Faz exatamente o inverso, usando o topo da pilha `history`:

1. Desempilha o último `Undo`.
2. Devolve a peça ao lugar de origem (se foi promoção, volta a ser peão).
3. Restaura a casa de destino: em en passant, o destino fica vazio e o peão capturado
   volta para o lado; nos demais, o capturado volta ao destino.
4. Se foi roque, devolve a torre.
5. Restaura direitos de roque, en passant e a vez a partir do `Undo`.

> O parâmetro `Move` é mantido por compatibilidade com a assinatura original do projeto,
> mas a fonte da verdade é a pilha — assim o desfazer funciona mesmo para lances especiais,
> cujo estado não cabe só nas coordenadas.

### 6.4 Geração de movimentos pseudo-legais

"Pseudo-legal" = segue o andar da peça, mas **pode** deixar o próprio rei em xeque. A
filtragem fina fica na Fase 3 (mais simples e menos sujeito a erro, como o roteiro sugere).

Helpers, um por família de peça:

- **`addSliding`** (torre, bispo, dama): anda em cada direção até bater na borda ou numa
  peça; captura inimigo e para.
- **`addSteps`** (cavalo, rei): testa cada destino fixo uma vez.
- **`addPawn`**: avanço de 1 casa, avanço duplo a partir da fila inicial, capturas
  diagonais, **promoção** (gera as 4 peças quando chega na última fileira) e **en passant**
  (captura na casa marcada em `epRow/epCol`).
- **`addCastles`**: gera o roque se o direito existe, as casas entre rei e torre estão
  vazias e a torre certa está no canto. (A regra de "não passar por xeque" é checada na
  Fase 3, que tem acesso à noção de casa atacada.)

`generatePseudoLegalMoves` varre as 64 casas e, para cada peça da cor pedida, chama o
helper certo via um `switch` no tipo. As direções (`rookDirs`, `bishopDirs`, etc.) são
`static const` — criadas uma vez só, compartilhadas por todas as chamadas.

### 6.5 `printBoard` e `getPieceChar`

Imprimem o tabuleiro com coordenadas `a–h` e `1–8`. Brancas em maiúsculas, pretas em
minúsculas, `.` para casa vazia. É a função reaproveitada pela interface CLI.

---

## 7. `GameState` — detecção de estados (Fase 3)

Classe com **apenas métodos estáticos**. Decisão consciente: ela não guarda estado nenhum
— é uma coleção de funções puras que operam sobre um `Board`. Por isso não faz sentido
criar um objeto `GameState`; chamamos `GameState::isInCheck(...)` diretamente.

### 7.1 `isSquareAttacked`

```cpp
bool GameState::isSquareAttacked(const Board& board, int row, int col, Color attacker) {
    for (const Move& m : board.generatePseudoLegalMoves(attacker))
        if (m.toRow == row && m.toCol == col && m.type != MoveType::Castle)
            return true;
    return false;
}
```

Pergunta central de todo o módulo: "alguma peça da cor `attacker` ataca a casa (row,col)?"
Gera os pseudo-legais do atacante e vê se algum termina nela. Lances de roque são
ignorados porque o rei "andando para o roque" não constitui um ataque àquela casa. Recebe
o tabuleiro por **referência constante** (`const Board&`): lê sem copiar e sem alterar.

### 7.2 `isInCheck`

```cpp
bool GameState::isInCheck(const Board& board, Color side) {
    int kr, kc;
    if (!board.findKing(side, kr, kc)) return false;
    return isSquareAttacked(board, kr, kc, opponent(side));
}
```

O rei está em xeque se a casa dele é atacada pelo adversário. Três linhas, porque toda a
complexidade já mora em `isSquareAttacked`.

### 7.3 `generateLegalMoves` — a regra de ouro

```cpp
for (const Move& m : board.generatePseudoLegalMoves(side)) {
    if (m.type == MoveType::Castle) {
        if (isInCheck(board, side)) continue;
        int pass = (m.toCol == 6) ? 5 : 3;
        if (isSquareAttacked(board, m.fromRow, pass, enemy)) continue;
    }
    board.makeMove(m);
    bool safe = !isInCheck(board, side);
    board.undoMove(m);
    if (safe) legal.push_back(m);
}
```

Para cada lance candidato: **faz**, verifica se o próprio rei ficou em xeque, **desfaz**.
Só os que deixam o rei seguro entram na lista. O roque tem duas checagens extras antes
(porque o rei não pode estar em xeque nem **passar** por uma casa atacada) — reaproveitando
`isSquareAttacked`. É exatamente a "dica crítica" do roteiro: gerar tudo e depois filtrar.

### 7.4 Xeque-mate e afogamento

```cpp
bool isCheckmate(...) { return  isInCheck(...) && generateLegalMoves(...).empty(); }
bool isStalemate(...) { return !isInCheck(...) && generateLegalMoves(...).empty(); }
```

Os dois usam a **mesma** ideia ("sem jogada legal") e diferem só em uma coisa: estar ou não
em xeque. Mate = encurralado **e** em xeque. Afogamento = encurralado **sem** xeque (empate).

---

## 8. `Evaluator` — herança e polimorfismo (Fase 4)

Aqui está o uso mais honesto de POO do projeto.

### 8.1 A classe base abstrata

```cpp
class Evaluator {
public:
    virtual ~Evaluator() = default;
    virtual int evaluate(const Board& board, Color side) const = 0;
    virtual const char* name() const = 0;
};
```

`Evaluator` é **abstrata**: tem métodos **virtuais puros** (`= 0`), ou seja, não dá para
criar um `Evaluator` direto — só suas filhas concretas. Ela define o **contrato**:
"todo avaliador sabe transformar uma posição num número (`evaluate`) e dizer seu nome".

- `virtual int evaluate(...) const = 0;` — positivo = bom para `side`. Cada filha decide
  *como* pontuar.
- `virtual ~Evaluator() = default;` — **destrutor virtual**, a regra de ouro ao usar
  herança com ponteiros. Sem ele, apagar uma filha por um ponteiro da base não chamaria o
  destrutor correto (vazaria recursos).

### 8.2 As três estratégias (herança)

```cpp
class MaterialEvaluator   : public Evaluator { ... };  // só material
class PositionalEvaluator : public Evaluator { ... };  // material + posição
class AggressiveEvaluator : public Evaluator { ... };  // material + mobilidade
```

Cada uma **herda** de `Evaluator` e implementa `evaluate` com `override` (palavra-chave que
diz ao compilador "estou sobrescrevendo um método virtual" — se a assinatura não bater, dá
erro, evitando bugs sutis).

- **Material:** soma o valor das minhas peças e subtrai o das do adversário. Valores em
  *centipawns* (peão=100, cavalo=320, bispo=330, torre=500, dama=900, rei=20000).
- **Posicional:** material **mais** um bônus de posição via *piece-square tables* (tabelas
  que dizem, por exemplo, que cavalo no centro vale mais). Para as pretas, a tabela é
  espelhada (`7 - r`), pois foi escrita do ponto de vista das brancas.
- **Agressiva:** material **mais** um bônus de mobilidade (diferença entre o número de
  lances meus e do adversário) — premia posições mais ativas.

### 8.3 A fábrica (Factory)

```cpp
class EvaluatorFactory {
public:
    enum class Strategy { Material, Positional, Aggressive };
    static std::unique_ptr<Evaluator> create(Strategy strategy);
};
```

`EvaluatorFactory::create` recebe a estratégia desejada e devolve o avaliador pronto, já
embrulhado num **`std::unique_ptr`** (ponteiro inteligente — gerencia a memória sozinho,
sem `delete` manual). Quem chama não precisa conhecer as classes filhas: pede pelo enum e
recebe um `Evaluator`. Isso é o padrão **Factory** + **Strategy** trabalhando juntos.

> **Por que polimorfismo cabe aqui e não nas peças?** O avaliador é chamado uma vez por
> folha da árvore de busca, não milhões de vezes por nó. Já dar comportamento virtual a
> cada peça tornaria a geração de movimentos lenta. Saber **onde** usar polimorfismo é o
> ponto de maturidade do projeto.

---

## 9. `MinimaxAI` — o motor de busca (Fase 4)

### 9.1 A classe

```cpp
class MinimaxAI {
public:
    MinimaxAI(int depth, std::unique_ptr<Evaluator> evaluator);
    Move chooseMove(Board& board, Color side, bool& found);
    long nodesVisited() const { return nodes; }
    int depth() const { return depth_; }
    const char* evaluatorName() const { return evaluator->name(); }
private:
    int depth_;
    std::unique_ptr<Evaluator> evaluator;
    long nodes = 0;
    int minimax(Board&, int depth, int alpha, int beta, bool maximizing, Color rootSide);
};
```

A IA é dona de um `Evaluator` por `unique_ptr` (**composição**: "a IA *tem um* avaliador").
Recebe a estratégia pela base — ela nunca sabe qual filha está usando, só chama
`evaluate()`. Isso é **polimorfismo dinâmico** de verdade.

```cpp
MinimaxAI::MinimaxAI(int depth, std::unique_ptr<Evaluator> evaluator)
    : depth_(depth), evaluator(std::move(evaluator)) {}
```

O `std::move` transfere a posse do ponteiro (não copia — `unique_ptr` não é copiável). Como
a classe tem um membro só-movível, a própria `MinimaxAI` vira **só-movível**, o que é usado
pela fábrica de dificuldade (seção 10).

### 9.2 `minimax` — recursão com Alpha-Beta

Casos base:

- **Sem jogadas legais:** se está em xeque, é mate — devolve um valor enorme com sinal
  conforme o lado (`±MATE`), ajustado pela profundidade para **preferir mates mais
  rápidos**. Se não está em xeque, é afogamento — devolve 0 (empate).
- **Profundidade 0:** chama o avaliador na posição atual.

Recursão:

```cpp
std::sort(moves.begin(), moves.end(), [&board](const Move& a, const Move& b) {
    return !board.getPiece(a.toRow, a.toCol).isEmpty() >
           !board.getPiece(b.toRow, b.toCol).isEmpty();
});
```

Antes de descer, os lances são **ordenados com capturas primeiro**, usando `std::sort` da
STL com uma **lambda** que captura o tabuleiro por referência. Por que? A poda Alpha-Beta
corta mais cedo quando os melhores lances vêm na frente — capturas costumam ser bons
candidatos, então isso reduz os nós visitados.

```cpp
if (maximizing) {
    int best = -INF;
    for (const Move& m : moves) {
        board.makeMove(m);
        best = std::max(best, minimax(board, depth-1, alpha, beta, false, rootSide));
        board.undoMove(m);
        alpha = std::max(alpha, best);
        if (beta <= alpha) break;   // poda
    }
    return best;
}
```

No nó maximizador escolhemos o maior score; no minimizador, o menor (assume-se que o
adversário joga o melhor para ele). `alpha` é o melhor garantido para quem maximiza;
`beta`, para quem minimiza. Quando `beta <= alpha`, o ramo nunca será escolhido por um dos
lados — então **podamos** (`break`). O resultado final é idêntico ao Minimax puro, só que
muito mais rápido. O contador `nodes` mede esse ganho.

### 9.3 `chooseMove`

No nível raiz, percorre os lances legais, chama `minimax` para o adversário em cada um e
guarda o de maior score. Devolve por referência (`bool& found`) se havia jogada — `false`
significa mate/afogamento.

---

## 10. `AIFactory` — níveis de dificuldade

```cpp
enum class Difficulty { Easy, Medium, Hard };

MinimaxAI AIFactory::create(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Easy:
            return MinimaxAI(2, EvaluatorFactory::create(Strategy::Material));
        case Difficulty::Medium:
            return MinimaxAI(3, EvaluatorFactory::create(Strategy::Positional));
        case Difficulty::Hard:
            return MinimaxAI(4, EvaluatorFactory::create(Strategy::Positional));
    }
}
```

Cada nível muda **dois** parâmetros reais da IA:

| Nível | Profundidade | Avaliador |
|---|---|---|
| Fácil | 2 | Material (ingênuo) |
| Médio | 3 | Posicional |
| Difícil | 4 | Posicional |

Profundidade maior = a IA "enxerga" mais lances à frente; avaliador melhor = ela julga
melhor cada posição. A fábrica devolve a `MinimaxAI` **por valor** — possível porque a
classe é movível (o `unique_ptr` é movido para fora). Mais uma demonstração de *move
semantics*.

---

## 11. Por que `makeMove`/`undoMove` (e não copiar o tabuleiro)?

O Minimax visita milhares de posições. Copiar um tabuleiro inteiro a cada lance seria caro.
Em vez disso, **aplicamos** o lance, descemos na recursão e **desfazemos** ao voltar,
reaproveitando o mesmo objeto `Board`. Por isso `undoMove` precisa restaurar tudo: peça
capturada, direitos de roque, casa de en passant e a vez — informação que guardamos na
pilha `history`. É o padrão *make/unmake* dos motores de xadrez de verdade.

---

## 12. `CLI` e `main.cpp` — a interface (Fase 5)

`CLI` **encapsula** uma partida: tem um `Board` e uma `MinimaxAI` (composição). Métodos
privados quebram o problema em pedaços legíveis:

- `chooseDifficulty` / `chooseColor`: o menu inicial.
- `parseSquares`: converte `"e2 e4"` em coordenadas (`coluna = letra - 'a'`,
  `linha = 8 - número`).
- `askPromotion`: pergunta a peça quando um peão promove.
- `squareName`: o inverso, para mostrar o lance da IA.
- `playGame`: o **laço principal** — imprime o tabuleiro; se não há lance legal, anuncia
  mate ou afogamento e termina; senão, lê o lance do humano (validando contra a lista legal
  e tratando promoção) ou pede o lance à IA; aplica e repete.
- `run`: roda partidas enquanto o jogador quiser (`askPlayAgain`).

O casamento do lance digitado usa as coordenadas para achar o lance correspondente **na
lista de legais** e aplica o objeto encontrado — que já carrega o `MoveType` certo. Assim
o jogador digita um simples `e1 g1` e o sistema sabe que é roque.

`main.cpp` só cria a `CLI` e chama `run()` — ponto de entrada mínimo.

---

## 13. `tests/tests.cpp`

São 25 verificações automáticas cobrindo: ausência de xeque na abertura, as 20 jogadas
iniciais, mate/afogamento, a IA achar e jogar lances legais, capturar peça pendurada,
evitar sacrifício ruim, a poda terminar em profundidade 4, polimorfismo do avaliador,
a fábrica, **roque** (disponível, bloqueado e rejeitado por casa atacada), **en passant**
(captura e `make`+`undo` idênticos) e **promoção** (4 opções, vira a peça certa), além das
três dificuldades. Rodar `./build/tests` deve mostrar `25/25`.

---

## 14. Tabela de conceitos POO → onde aparecem

| Conceito | Onde no código |
|---|---|
| **Abstração** (classe base abstrata) | `Evaluator` com método virtual puro |
| **Herança** | `Material/Positional/AggressiveEvaluator : public Evaluator` |
| **Polimorfismo dinâmico** | `MinimaxAI` chama `evaluator->evaluate()` sem saber a filha |
| **Encapsulamento** | membros `private` em `Board`, `MinimaxAI`, `CLI` |
| **Composição** | `CLI` tem `Board`+`MinimaxAI`; `MinimaxAI` tem `Evaluator` |
| **Destrutor virtual** | `virtual ~Evaluator() = default` |
| **`override`** | nas três filhas de `Evaluator` |
| **RAII / ponteiro inteligente** | `std::unique_ptr<Evaluator>` |
| **Move semantics** | `std::move` no construtor; `AIFactory` retorna por valor |
| **Sobrecarga de operador** | `Move::operator==` |
| **`enum class`** | `PieceType`, `Color`, `MoveType`, `Difficulty`, `Strategy` |
| **Métodos estáticos / função pura** | `GameState`, as fábricas |
| **STL + lambda** | `std::vector`, `std::sort` com comparador na busca |
| **const-correctness** | `const Board&`, getters `const` |
| **Factory + Strategy (padrões)** | `EvaluatorFactory`, `AIFactory` |

---

## 15. Perguntas prováveis da banca (e respostas curtas)

- **Por que Minimax e não regras fixas?** Porque ele raciocina sobre o futuro: simula os
  dois lados jogando bem e escolhe o lance que leva à melhor posição garantida.
- **A poda muda o resultado?** Não. Alpha-Beta só corta ramos que comprovadamente não
  seriam escolhidos; o lance final é o mesmo do Minimax puro, só calculado mais rápido.
- **Qual a diferença de mate para afogamento no código?** A mesma checagem ("sem jogada
  legal") com um `if` a mais: em xeque → mate; sem xeque → afogamento.
- **Como adicionar uma 4ª personalidade de IA?** Criar uma nova filha de `Evaluator` e um
  caso na fábrica. O `MinimaxAI` não muda **uma linha** — é o ganho do polimorfismo.
- **Por que `unique_ptr` e não `new/delete`?** Para não vazar memória: o ponteiro se
  autodestrói ao sair de escopo (RAII).
```
