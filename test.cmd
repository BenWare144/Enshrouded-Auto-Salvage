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
for %%T in (rules rule_store inventory abi automatic performance panel_text_style) do (
    cl /nologo /O2 /MT /EHsc /std:c++17 /W4 src\%%T_test.cpp /Fo:build\%%T_test.obj /Fe:build\%%T_test.exe /link user32.lib
    if errorlevel 1 exit /b 1
    pushd build
    %%T_test.exe "%CD%\build"
    if errorlevel 1 exit /b 1
    popd
)
echo All seven native mock test programs passed. These are not real-game or Proton tests.
