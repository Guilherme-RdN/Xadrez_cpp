@echo off
setlocal

set MSYS=C:\msys64\ucrt64
set PATH=%MSYS%\bin;%PATH%

if not exist bin mkdir bin

echo Compilando...
g++ -std=c++17 -Iinclude ^
    src/board/Board.cpp src/game/GameState.cpp ^
    src/ai/Evaluator.cpp src/ai/MinimaxAI.cpp src/ai/AIFactory.cpp ^
    src/ui/GUI.cpp src/main.cpp ^
    -lsfml-graphics -lsfml-window -lsfml-system ^
    -o bin\xadrez.exe

if not %ERRORLEVEL% == 0 (
    echo.
    echo Erro na compilacao.
    exit /b 1
)

echo Copiando bibliotecas...
for %%D in (
    libsfml-graphics-3.dll libsfml-window-3.dll libsfml-system-3.dll
    libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll
    libfreetype-6.dll libpng16-16.dll zlib1.dll libbz2-1.dll
    libbrotlidec.dll libbrotlicommon.dll libharfbuzz-0.dll
    libglib-2.0-0.dll libgraphite2.dll libiconv-2.dll
    libintl-8.dll libpcre2-8-0.dll
) do copy /Y "%MSYS%\bin\%%D" bin\ >nul

echo.
echo Pronto! Execute run.bat ou de duplo clique em bin\xadrez.exe
endlocal
