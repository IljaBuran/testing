@echo off
setlocal

call C:\"Program Files"\"Microsoft Visual Studio"\18\Community\Common7\Tools\VsDevCmd.bat -arch=x64

REM check whether cl.exe is available in PATH
where cl >nul 2>nul
if errorlevel 1 (
    echo Error: cl.exe was not found!
    exit /b 1
)

set "cflags=/EHsc /fp:fast /Dplatform_win32 /DDEBUG /Zi /Od /std:c++20 /MD"

cl %cflags% %*

if errorlevel 1 (
    exit /b 1
)