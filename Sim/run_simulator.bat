@echo off
cd /d "%~dp0"

where python >nul 2>nul
if errorlevel 1 (
    echo [ERROR] "python" was not found. Check your Python install and PATH.
    pause
    exit /b 1
)

pip install -r requirements.txt

python simulator.py

echo.
echo Simulator stopped.
pause
