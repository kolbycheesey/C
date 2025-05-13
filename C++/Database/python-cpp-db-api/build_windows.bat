@echo off
echo Building database engine library first...

cd ..
if not exist build mkdir build
cd build

echo Running CMake for the main database project...
cmake .. -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo Failed to configure the main database project.
    cd ..\python-cpp-db-api
    exit /b 1
)

echo Building the database engine static library with Visual Studio...
cmake --build . --config Release --target database_engine_lib
if %ERRORLEVEL% neq 0 (
    echo Failed to build the database engine library.
    cd ..\python-cpp-db-api
    exit /b 1
)

echo.
echo Checking if database_engine.lib was built successfully:
if exist lib\database_engine.lib (
    echo Found library at: %CD%\lib\database_engine.lib
) else if exist lib\Debug\database_engine.lib (
    echo Found library at: %CD%\lib\Debug\database_engine.lib
) else if exist lib\Release\database_engine.lib (
    echo Found library at: %CD%\lib\Release\database_engine.lib
) else (
    echo Library not found in standard locations!
    dir /s /b database_engine.lib
    echo Failed to find the database_engine.lib file. Build may not have succeeded.
    cd ..\python-cpp-db-api
    exit /b 1
)

echo.
echo Now building Python bindings...
cd ..\python-cpp-db-api

if exist build (
    echo Cleaning existing build directory...
    rd /s /q build
)

echo Creating new build directory...
mkdir build
cd build

echo Running CMake for Python bindings...
cmake .. -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo Failed to configure the Python bindings project.
    cd ..
    exit /b 1
)

echo Building Python module with Visual Studio...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo Failed to build the Python module.
    cd ..
    exit /b 1
)

echo.
echo Build completed.
echo.
echo If the build was successful, you can install the package with:
echo pip install -e .
echo.