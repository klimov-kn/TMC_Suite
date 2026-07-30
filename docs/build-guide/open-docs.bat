@echo off
set "DOC=%~dp0sphinx\_build\html\index.html"
if not exist "%DOC%" goto nofile
start "" "%DOC%"
exit /b
:nofile
echo.
echo HTML-документация не найдена:
echo   "%DOC%"
echo Соберите её через Sphinx и запустите снова.
echo.
pause
