@echo off
setlocal

cmake -S "%~dp0." -B "%~dp0build"
if errorlevel 1 (
    echo Configuration CMake echouee.
    exit /b 1
)

cmake --build "%~dp0build" --config Release
if errorlevel 1 (
    echo Compilation Release echouee.
    exit /b 1
)

if not exist "%~dp0build\controller\Release\controller.exe" (
    echo controller.exe est introuvable.
    exit /b 1
)

if not exist "%~dp0build\implant\Release\implant.exe" (
    echo implant.exe est introuvable.
    exit /b 1
)

echo Compilation Release terminee avec succes.
echo Demarrage du controleur.
"%~dp0build\controller\Release\controller.exe"
