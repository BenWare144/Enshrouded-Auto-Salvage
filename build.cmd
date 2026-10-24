@echo off
setlocal
cd /d "%~dp0"
where cl.exe >nul 2>&1
if errorlevel 1 (
    if defined VCVARS64 (
        call "%VCVARS64%"
    ) else (
        echo Open an x64 Native Tools Command Prompt for Visual Studio, or set VCVARS64 to vcvars64.bat.
        exit /b 1
    )
)
if errorlevel 1 exit /b 1
if not exist build mkdir build
if not exist dist mkdir dist
cl /nologo /O2 /MT /EHsc /std:c++17 /W4 /LD src\quick_salvage.cpp /Fo:build\ /link /OUT:dist\QuickSalvage.asi /IMPLIB:build\QuickSalvage.lib user32.lib
if errorlevel 1 exit /b 1
copy /y config\QuickSalvage.ini dist\QuickSalvage.ini >nul
exit /b %errorlevel%
