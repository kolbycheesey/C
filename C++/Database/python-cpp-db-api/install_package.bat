@echo off
echo ===================================
echo Python Database API Installation
echo ===================================

echo.
echo Step 1: Looking for database_engine.lib...

set "LIB_PATH="

REM Check in standard locations
if exist "..\build\lib\Release\database_engine.lib" (
    set "LIB_PATH=..\build\lib\Release"
    echo Found library at %CD%\..\build\lib\Release\database_engine.lib
    goto :found_lib
)

if exist "..\build\lib\Debug\database_engine.lib" (
    set "LIB_PATH=..\build\lib\Debug"
    echo Found library at %CD%\..\build\lib\Debug\database_engine.lib
    goto :found_lib
)

if exist "..\build\lib\database_engine.lib" (
    set "LIB_PATH=..\build\lib"
    echo Found library at %CD%\..\build\lib\database_engine.lib
    goto :found_lib
)

echo Library not found in standard locations!
echo Searching recursively (this might take a moment)...

for /r "..\build" %%i in (database_engine.lib) do (
    set "LIB_PATH=%%~dpi"
    echo Found library at %%i
    goto :found_lib
)

echo.
echo ERROR: Could not find database_engine.lib
echo Please build the main database project first using:
echo    cd ..
echo    mkdir build
echo    cd build
echo    cmake ..
echo    cmake --build . --config Release --target database_engine_lib
echo.
goto :error

:found_lib
echo.
echo Step 2: Setting up environment...

echo LIBRARY_PATH=%LIB_PATH% > .env
echo.
echo Step 3: Installing Python package...

pip install -e .

if %ERRORLEVEL% == 0 (
    echo.
    echo ===================================
    echo Installation completed successfully!
    echo ===================================
    goto :end
) else (
    echo.
    echo ===================================
    echo Installation failed! See errors above.
    echo ===================================
    goto :error
)

:error
exit /b 1

:end
echo.
echo You can now import the package in Python with:
echo    from src.python.api import DatabaseAPI
echo    from src.python.connection import DatabaseConnection
echo.
exit /b 0