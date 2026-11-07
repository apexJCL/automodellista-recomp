@echo off
rem automodellista: the project CLI (automodellista.py). Python 3.9 or newer.
where py >nul 2>nul
if %errorlevel%==0 (
    py -3 "%~dp0automodellista.py" %*
) else (
    python "%~dp0automodellista.py" %*
)
exit /b %errorlevel%
