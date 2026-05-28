# Makefile — DEV 2 (Fases 3 e 4)
#   make       -> compila
#   make test  -> compila e roda os testes
#   make clean -> apaga os binarios

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

FONTES := Tabuleiro.cpp EstadoJogo.cpp Avaliador.cpp IAMinimax.cpp
TESTE  := teste_main.cpp

testes: $(FONTES) $(TESTE)
	$(CXX) $(CXXFLAGS) $(FONTES) $(TESTE) -o testes

.PHONY: test clean
test: testes
	./testes

clean:
	rm -f testes
