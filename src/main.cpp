#include <iostream>
#include "../include/Board.h"

using namespace std;

int main() {
    cout << "=== Motor de Xadrez C++ Iniciado ===\n";
    
    Board tabuleiro;
    tabuleiro.printBoard();

    // Testando um movimento manual (Fase 1 pede métodos de movimentação básicos)
    cout << "Movendo peao de e2 para e4 (Teste):\n";
    Move teste = {6, 4, 4, 4}; // Lembrado que array começa em 0 (linha 6 = rank 2)
    tabuleiro.makeMove(teste);
    tabuleiro.printBoard();

    return 0;
}