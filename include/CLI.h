#pragma once
#include "Board.h"
#include "AIFactory.h"
#include <string>

class CLI {
public:
    void run();

private:
    Difficulty chooseDifficulty() const;
    Color chooseColor() const;
    void playGame(Difficulty difficulty, Color humanColor);

    bool parseSquares(const std::string& line, int& fr, int& fc, int& tr, int& tc) const;
    PieceType askPromotion() const;
    std::string squareName(int row, int col) const;
    bool askPlayAgain() const;
};
