@echo off
cd /d "%~dp0"

where python >nul 2>nul
if errorlevel 1 (
    echo [ERROR] python 명령을 찾을 수 없습니다. Python 설치와 PATH 설정을 확인하세요.
    pause
    exit /b 1
)

pip install -r requirements.txt

python simulator.py

echo.
echo Simulator stopped.
pause
