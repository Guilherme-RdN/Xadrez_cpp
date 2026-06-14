@echo off
if not exist "%~dp0bin\xadrez.exe" (
    echo Executavel nao encontrado. Rode build.bat primeiro.
    pause
    exit /b 1
)
start "" "%~dp0bin\xadrez.exe"
