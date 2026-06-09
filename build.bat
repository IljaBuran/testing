@echo off
setlocal

call actcl

REM check whether cl.exe is available in PATH
where cl >nul 2>nul
if errorlevel 1 (
    echo Error: cl.exe was not found!
    exit /b 1
)

set "cflags=/EHsc /fp:fast /Dplatform_win32 /DDEBUG /Zi /Od /std:c++latest /MD"

cl %cflags% %*

if errorlevel 1 (
    exit /b 1
)