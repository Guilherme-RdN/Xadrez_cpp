@echo off
set PATH=C:\msys64\mingw64\bin;%PATH%

g++ -std=c++17 -Iinclude -I"C:/msys64/mingw64/include" ^
    src/board/Board.cpp src/game/GameState.cpp ^
    src/ai/Evaluator.cpp src/ai/MinimaxAI.cpp src/ai/AIFactory.cpp ^
    src/ui/GUI.cpp src/main.cpp ^
    -L"C:/msys64/mingw64/lib" ^
    -lsfml-graphics -lsfml-window -lsfml-system ^
    -static-libgcc -static-libstdc++ ^
    -o xadrez.exe

if %ERRORLEVEL% == 0 (
    echo.
    echo Compilado com sucesso: xadrez.exe
    echo Para jogar: clique duas vezes em xadrez.exe ou run.bat
) else (
    echo.
    echo Erro na compilacao.
)
