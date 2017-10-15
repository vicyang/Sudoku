@echo off
call :comp %1
exit /b

:comp
gcc -o "%~n1.exe" "%~1"
if %errorlevel% == 0 "%~n1.exe"