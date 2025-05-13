@echo off
echo Searching for database_engine.exe in all possible locations...
echo.

set FOUND=0

echo Checking parent build directories:
echo ---------------------------------
if exist ..\build\database_engine.exe (
    echo FOUND: ..\build\database_engine.exe
    set FOUND=1
)
if exist ..\build\Debug\database_engine.exe (
    echo FOUND: ..\build\Debug\database_engine.exe
    set FOUND=1
)
if exist ..\build\Release\database_engine.exe (
    echo FOUND: ..\build\Release\database_engine.exe
    set FOUND=1
)
if exist ..\build\x64\Debug\database_engine.exe (
    echo FOUND: ..\build\x64\Debug\database_engine.exe
    set FOUND=1
)
if exist ..\build\x64\Release\database_engine.exe (
    echo FOUND: ..\build\x64\Release\database_engine.exe
    set FOUND=1
)
if exist ..\build\bin\Debug\database_engine.exe (
    echo FOUND: ..\build\bin\Debug\database_engine.exe
    set FOUND=1
)
if exist ..\build\bin\Release\database_engine.exe (
    echo FOUND: ..\build\bin\Release\database_engine.exe
    set FOUND=1
)

echo.
if %FOUND%==0 (
    echo No database_engine.exe found in any standard locations.
    echo Performing a full search of the build directory...
    dir /s /b ..\build\*.exe | findstr database_engine
) else (
    echo Executable file(s) found. Rather than linking with an executable,
    echo we need to create a static library from the database code.
)
echo.