#include "GUI.h"
#include <algorithm>
#include <string>

static const sf::Color C_LIGHT    {240, 217, 181};
static const sf::Color C_DARK     {181, 136,  99};
static const sf::Color C_SEL      {247, 247, 105};
static const sf::Color C_HINT     {106, 168,  79, 160};
static const sf::Color C_CAP      {196,  64,  64, 180};
static const sf::Color C_BG       { 22,  21,  18};
static const sf::Color C_PANEL    { 38,  36,  33};
static const sf::Color C_WHITE_P  {255, 255, 240};
static const sf::Color C_BLACK_P  { 28,  28,  28};
static const sf::Color C_LAST_MOVE{162, 213, 120};  // verde-oliva: origem e destino do lance da IA

static void playMoveSound(bool capture) {
#ifdef _WIN32
    if (capture)
        Beep(600, 45);
    else
        Beep(1100, 22);
#endif
}

static std::string sqName(int r, int c) {
    return std::string(1, static_cast<char>('a' + c)) + std::to_string(8 - r);
}

void GUI::run() {
    window.create(sf::VideoMode({static_cast<unsigned>(WIN_W), static_cast<unsigned>(WIN_H)}),
                  "Xadrez IA");
    window.setFramerateLimit(60);

    if (!font.openFromFile("C:/Windows/Fonts/seguisym.ttf"))
        (void)font.openFromFile("C:/Windows/Fonts/arial.ttf");

    while (window.isOpen()) {
        while (const std::optional<sf::Event> ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>())
                window.close();

            if (const auto* mb = ev->getIf<sf::Event::MouseButtonReleased>()) {
                if (mb->button == sf::Mouse::Button::Left) {
                    int mx = mb->position.x, my = mb->position.y;
                    switch (state) {
                        case State::Menu:      onMenuClick(mx, my);      break;
                        case State::Playing:   onBoardClick(mx, my);     break;
                        case State::Promotion: onPromotionClick(mx, my); break;
                        case State::GameOver:  onGameOverClick(mx, my);  break;
                    }
                }
            }
        }

        if (state == State::Playing && board.sideToMove() != humanColor) {
            if (!aiDelayPending ||
                aiDelayClock.getElapsedTime().asMilliseconds() >= 250)
                aiTurn();
        }

        window.clear(C_BG);
        switch (state) {
            case State::Menu:
                drawMenu();
                break;
            case State::Playing:
                drawBoard();
                drawSidebar();
                break;
            case State::Promotion:
                drawBoard();
                drawSidebar();
                drawPromotion();
                break;
            case State::GameOver:
                drawBoard();
                drawSidebar();
                drawGameOver();
                break;
        }
        window.display();
    }
}

void GUI::startGame() {
    board = Board{};
    MinimaxAI tmp = AIFactory::create(difficulty);
    ai = std::make_unique<MinimaxAI>(std::move(tmp));
    selRow = selCol = -1;
    highlights.clear();
    lastAiMove.clear();
    lastNodes      = 0;
    gameResult.clear();
    hasLastAiMove  = false;
    aiDelayPending = false;
    state = State::Playing;
}

void GUI::drawMenu() {
    float cx = WIN_W / 2.f;

    drawText("XADREZ IA", cx - 110, 80, 52, sf::Color::White);
    drawText("por IA Minimax com Alpha-Beta", cx - 155, 148, 18, sf::Color(160, 160, 160));

    drawText("Dificuldade", cx - 56, 220, 20, sf::Color(200, 200, 200));

    struct DBtn { const char* label; float ox; Difficulty d; };
    DBtn diffs[] = {
        {"Facil",   -240.f, Difficulty::Easy},
        {"Medio",    -80.f, Difficulty::Medium},
        {"Dificil",  +80.f, Difficulty::Hard}
    };
    for (auto& b : diffs) {
        float bx = cx + b.ox;
        bool sel = difficulty == b.d;
        drawRect(bx, 258, 140, 52,
                 sel ? sf::Color(80, 140, 80) : sf::Color(55, 55, 55),
                 sel ? sf::Color(120, 200, 120) : sf::Color::Transparent, 2.f);
        drawText(b.label, bx + 16, 272, 20, sf::Color::White);
    }

    drawText("Jogar como", cx - 54, 348, 20, sf::Color(200, 200, 200));

    bool asWhite = humanColor == Color::WHITE;
    drawRect(cx - 160, 384, 140, 52,
             asWhite ? sf::Color(80, 140, 80) : sf::Color(55, 55, 55),
             asWhite ? sf::Color(120, 200, 120) : sf::Color::Transparent, 2.f);
    drawText("Brancas", cx - 144, 398, 20, sf::Color::White);

    drawRect(cx + 20, 384, 140, 52,
             !asWhite ? sf::Color(80, 140, 80) : sf::Color(55, 55, 55),
             !asWhite ? sf::Color(120, 200, 120) : sf::Color::Transparent, 2.f);
    drawText("Pretas", cx + 36, 398, 20, sf::Color::White);

    drawRect(cx - 110, 480, 220, 60, sf::Color(160, 90, 40));
    drawText("JOGAR", cx - 50, 495, 30, sf::Color::White);
}

void GUI::drawBoard() {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            bool light = (r + c) % 2 == 0;
            bool isLastAi = hasLastAiMove &&
                            ((r == lastAiMoveObj.fromRow && c == lastAiMoveObj.fromCol) ||
                             (r == lastAiMoveObj.toRow   && c == lastAiMoveObj.toCol));
            sf::Color sq = (r == selRow && c == selCol) ? C_SEL
                         : isLastAi ? C_LAST_MOVE
                         : light ? C_LIGHT : C_DARK;

            auto pos = boardToScreen(r, c);
            drawRect(pos.x, pos.y, SQUARE, SQUARE, sq);

            for (const Move& m : highlights) {
                if (m.toRow == r && m.toCol == c) {
                    Piece target = board.getPiece(r, c);
                    if (target.isEmpty() && m.type != MoveType::EnPassant) {
                        sf::CircleShape dot(13.f);
                        dot.setFillColor(C_HINT);
                        dot.setPosition({pos.x + SQUARE / 2.f - 13.f, pos.y + SQUARE / 2.f - 13.f});
                        window.draw(dot);
                    } else {
                        drawRect(pos.x, pos.y, SQUARE, SQUARE, C_CAP);
                    }
                    break;
                }
            }

            Piece p = board.getPiece(r, c);
            if (!p.isEmpty()) {
                sf::String sym = pieceStr(p);

                sf::Text shadow(font, sym, 56);
                shadow.setFillColor(sf::Color(0, 0, 0, 120));
                auto lb = shadow.getLocalBounds();
                shadow.setOrigin({lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f});
                shadow.setPosition({pos.x + SQUARE / 2.f + 2.f, pos.y + SQUARE / 2.f + 3.f});
                window.draw(shadow);

                sf::Text piece(font, sym, 56);
                piece.setFillColor(p.color == Color::WHITE ? C_WHITE_P : C_BLACK_P);
                lb = piece.getLocalBounds();
                piece.setOrigin({lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f});
                piece.setPosition({pos.x + SQUARE / 2.f, pos.y + SQUARE / 2.f});
                window.draw(piece);
            }
        }
    }

    for (int i = 0; i < 8; i++) {
        sf::Color lc{120, 120, 120};
        drawText(std::string(1, static_cast<char>('a' + i)),
                 BOARD_X + i * SQUARE + SQUARE / 2.f - 5.f, BOARD_Y + 640 + 4, 13, lc);
        drawText(std::to_string(8 - i),
                 BOARD_X - 18.f, BOARD_Y + i * SQUARE + SQUARE / 2.f - 10.f, 13, lc);
    }
}

void GUI::drawSidebar() {
    drawRect(PANEL_X, 0, WIN_W - PANEL_X, WIN_H, C_PANEL);

    drawText("XADREZ IA", PANEL_X + 14, 18, 22, sf::Color::White);

    const char* dstr = difficulty == Difficulty::Easy   ? "Facil"
                     : difficulty == Difficulty::Medium  ? "Medio" : "Dificil";
    drawText(std::string("Nivel: ") + dstr, PANEL_X + 14, 58, 16, sf::Color(170, 170, 170));

    Color turn = board.sideToMove();
    bool myTurn = turn == humanColor;
    drawText(myTurn ? "Sua vez" : "Vez da IA",
             PANEL_X + 14, 92, 18,
             myTurn ? sf::Color(100, 220, 100) : sf::Color(220, 130, 80));

    if (GameState::isInCheck(board, turn))
        drawText("XEQUE!", PANEL_X + 14, 120, 18, sf::Color(255, 70, 70));

    if (!lastAiMove.empty()) {
        drawText("Ultimo lance IA:", PANEL_X + 14, 178, 14, sf::Color(150, 150, 150));
        drawText(lastAiMove,         PANEL_X + 14, 198, 20, sf::Color::White);
        drawText("Nos: " + std::to_string(lastNodes),
                 PANEL_X + 14, 224, 13, sf::Color(120, 120, 120));
    }

    drawRect(PANEL_X + 10, WIN_H - 56, 210, 42, sf::Color(75, 55, 55));
    drawText("Menu Principal", PANEL_X + 22, WIN_H - 46, 16, sf::Color(220, 220, 220));
}

void GUI::drawPromotion() {
    sf::RectangleShape overlay({static_cast<float>(WIN_W), static_cast<float>(WIN_H)});
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(overlay);

    float bx0 = WIN_W / 2.f - 200.f;
    float by  = WIN_H / 2.f - 70.f;
    drawRect(bx0 - 10, by - 40, 420, 200, sf::Color(45, 45, 45));
    drawText("Promover para:", bx0 + 60, by - 30, 20, sf::Color::White);

    PieceType promos[] = {PieceType::QUEEN, PieceType::ROOK, PieceType::BISHOP, PieceType::KNIGHT};
    for (int i = 0; i < 4; i++) {
        float x = bx0 + i * 100.f;
        drawRect(x, by, 88, 88, sf::Color(90, 90, 90));

        Piece p{promos[i], humanColor};
        sf::Text t(font, pieceStr(p), 54);
        auto lb = t.getLocalBounds();
        t.setOrigin({lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f});
        t.setFillColor(humanColor == Color::WHITE ? C_WHITE_P : C_BLACK_P);
        t.setPosition({x + 44.f, by + 44.f});
        window.draw(t);
    }
}

void GUI::drawGameOver() {
    sf::RectangleShape overlay({static_cast<float>(WIN_W), static_cast<float>(WIN_H)});
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(overlay);

    float cx = WIN_W / 2.f;
    drawRect(cx - 210, WIN_H / 2.f - 100, 420, 200, sf::Color(42, 42, 42));
    drawText(gameResult, cx - static_cast<float>(gameResult.size()) * 7, WIN_H / 2.f - 80, 26, sf::Color::White);

    drawRect(cx - 120, WIN_H / 2.f + 20, 110, 46, sf::Color(55, 100, 55));
    drawText("Jogar novamente", cx - 115, WIN_H / 2.f + 32, 13, sf::Color::White);

    drawRect(cx + 10, WIN_H / 2.f + 20, 110, 46, sf::Color(100, 55, 55));
    drawText("Menu", cx + 36, WIN_H / 2.f + 32, 16, sf::Color::White);
}

void GUI::onMenuClick(int mx, int my) {
    float cx = WIN_W / 2.f;

    float dox[] = {-240.f, -80.f, +80.f};
    Difficulty ds[] = {Difficulty::Easy, Difficulty::Medium, Difficulty::Hard};
    for (int i = 0; i < 3; i++)
        if (hit(mx, my, cx + dox[i], 258, 140, 52)) difficulty = ds[i];

    if (hit(mx, my, cx - 160, 384, 140, 52)) humanColor = Color::WHITE;
    if (hit(mx, my, cx + 20,  384, 140, 52)) humanColor = Color::BLACK;

    if (hit(mx, my, cx - 110, 480, 220, 60)) startGame();
}

void GUI::onBoardClick(int mx, int my) {
    if (hit(mx, my, PANEL_X + 10, WIN_H - 56, 210, 42)) {
        state = State::Menu;
        return;
    }

    if (board.sideToMove() != humanColor) return;

    int r, c;
    if (!screenToBoard(mx, my, r, c)) {
        selRow = selCol = -1;
        highlights.clear();
        return;
    }

    if (selRow >= 0) {
        for (const Move& m : highlights) {
            if (m.toRow == r && m.toCol == c) {
                if (m.promotion != PieceType::NONE) {
                    pendingPromo = m;
                    state = State::Promotion;
                    return;
                }
                applyMove(m);
                return;
            }
        }
    }

    Piece p = board.getPiece(r, c);
    if (!p.isEmpty() && p.color == humanColor) {
        selRow = r;
        selCol = c;
        highlights.clear();
        for (const Move& m : GameState::generateLegalMoves(board, humanColor))
            if (m.fromRow == r && m.fromCol == c)
                highlights.push_back(m);
    } else {
        selRow = selCol = -1;
        highlights.clear();
    }
}

void GUI::onPromotionClick(int mx, int my) {
    float bx0 = WIN_W / 2.f - 200.f;
    float by  = WIN_H / 2.f - 70.f;
    PieceType promos[] = {PieceType::QUEEN, PieceType::ROOK, PieceType::BISHOP, PieceType::KNIGHT};
    for (int i = 0; i < 4; i++) {
        if (hit(mx, my, bx0 + i * 100.f, by, 88, 88)) {
            Move m    = pendingPromo;
            m.promotion = promos[i];
            state = State::Playing;
            applyMove(m);
            return;
        }
    }
}

void GUI::onGameOverClick(int mx, int my) {
    float cx = WIN_W / 2.f;
    if (hit(mx, my, cx - 120, WIN_H / 2.f + 20, 110, 46)) startGame();
    if (hit(mx, my, cx + 10,  WIN_H / 2.f + 20, 110, 46)) state = State::Menu;
}

void GUI::applyMove(Move m) {
    bool capture = !board.getPiece(m.toRow, m.toCol).isEmpty() || m.type == MoveType::EnPassant;
    board.makeMove(m);
    playMoveSound(capture);
    selRow = selCol = -1;
    highlights.clear();
    // Inicia delay para a IA nao soar imediatamente apos o lance humano
    aiDelayPending = true;
    aiDelayClock.restart();
    checkEnd();
}

void GUI::aiTurn() {
    if (!ai || board.sideToMove() == humanColor) return;

    bool found = false;
    Color side = board.sideToMove();
    Move m = ai->chooseMove(board, side, found);
    if (found) {
        lastAiMove     = sqName(m.fromRow, m.fromCol) + "-" + sqName(m.toRow, m.toCol);
        lastNodes      = ai->nodesVisited();
        lastAiMoveObj  = m;
        hasLastAiMove  = true;
        aiDelayPending = false;
        bool capture   = !board.getPiece(m.toRow, m.toCol).isEmpty() || m.type == MoveType::EnPassant;
        board.makeMove(m);
        playMoveSound(capture);
        checkEnd();
    }
}

void GUI::checkEnd() {
    Color side = board.sideToMove();
    if (GameState::isCheckmate(board, side)) {
        Color winner = (side == Color::WHITE) ? Color::BLACK : Color::WHITE;
        gameResult = (winner == humanColor) ? "Voce venceu!" : "IA venceu!";
        state = State::GameOver;
    } else if (GameState::isStalemate(board, side)) {
        gameResult = "Empate por afogamento";
        state = State::GameOver;
    }
}

sf::Vector2f GUI::boardToScreen(int r, int c) const {
    return {static_cast<float>(BOARD_X + c * SQUARE),
            static_cast<float>(BOARD_Y + r * SQUARE)};
}

bool GUI::screenToBoard(int x, int y, int& r, int& c) const {
    if (x < BOARD_X || y < BOARD_Y) return false;
    c = (x - BOARD_X) / SQUARE;
    r = (y - BOARD_Y) / SQUARE;
    return r >= 0 && r < 8 && c >= 0 && c < 8;
}

sf::String GUI::pieceStr(const Piece& p) const {
    static const char32_t wh[] = {U'♔', U'♕', U'♖', U'♗', U'♘', U'♙'};
    static const char32_t bl[] = {U'♚', U'♛', U'♜', U'♝', U'♞', U'♟'};
    const char32_t* arr = (p.color == Color::WHITE) ? wh : bl;
    int idx = 0;
    switch (p.type) {
        case PieceType::KING:   idx = 0; break;
        case PieceType::QUEEN:  idx = 1; break;
        case PieceType::ROOK:   idx = 2; break;
        case PieceType::BISHOP: idx = 3; break;
        case PieceType::KNIGHT: idx = 4; break;
        case PieceType::PAWN:   idx = 5; break;
        default: { char32_t sp = U' '; return sf::String::fromUtf32(&sp, &sp + 1); }
    }
    return sf::String::fromUtf32(&arr[idx], &arr[idx] + 1);
}

void GUI::drawText(const sf::String& s, float x, float y, unsigned sz, sf::Color col) {
    sf::Text t(font, s, sz);
    t.setFillColor(col);
    t.setPosition({x, y});
    window.draw(t);
}

void GUI::drawRect(float x, float y, float w, float h, sf::Color fill, sf::Color outline, float thick) {
    sf::RectangleShape r({w, h});
    r.setFillColor(fill);
    r.setOutlineColor(outline);
    r.setOutlineThickness(thick);
    r.setPosition({x, y});
    window.draw(r);
}

bool GUI::hit(int mx, int my, float x, float y, float w, float h) const {
    return mx >= x && mx <= x + w && my >= y && my <= y + h;
}
