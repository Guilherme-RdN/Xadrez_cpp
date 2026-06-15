#pragma once
#include "Board.h"
#include "GameState.h"
#include "AIFactory.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

class GUI {
public:
    void run();

private:
    enum class State { Menu, Playing, Promotion, GameOver };

    static constexpr int SQUARE   = 80;
    static constexpr int BOARD_X  = 40;
    static constexpr int BOARD_Y  = 40;
    static constexpr int PANEL_X  = 700;
    static constexpr int WIN_W    = 930;
    static constexpr int WIN_H    = 720;

    sf::RenderWindow          window;
    sf::Font                  font;

    State                     state      = State::Menu;
    Board                     board;
    std::unique_ptr<MinimaxAI> ai;
    Color                     humanColor = Color::WHITE;
    Difficulty                difficulty = Difficulty::Medium;

    int               selRow = -1, selCol = -1;
    std::vector<Move> highlights;
    Move              pendingPromo{};
    std::string       lastAiMove;
    long              lastNodes  = 0;
    std::string       gameResult;
    Move              lastAiMoveObj{};   // fromRow = -1 quando nao ha lance anterior
    bool              hasLastAiMove = false;

    void startGame();
    void drawMenu();
    void drawBoard();
    void drawSidebar();
    void drawPromotion();
    void drawGameOver();

    void onMenuClick(int x, int y);
    void onBoardClick(int x, int y);
    void onPromotionClick(int x, int y);
    void onGameOverClick(int x, int y);

    void applyMove(Move m);
    void aiTurn();
    void checkEnd();

    sf::Vector2f boardToScreen(int r, int c) const;
    bool         screenToBoard(int x, int y, int& r, int& c) const;
    sf::String   pieceStr(const Piece& p) const;

    void drawText(const sf::String& s, float x, float y, unsigned sz, sf::Color col);
    void drawRect(float x, float y, float w, float h, sf::Color fill,
                  sf::Color outline = sf::Color::Transparent, float thick = 0.f);
    bool hit(int mx, int my, float x, float y, float w, float h) const;
};
