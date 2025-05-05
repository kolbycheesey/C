cmake_minimum_required(VERSION 3.10)
project(HighPerformanceDatabase VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source directories
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIRS 
    ${SRC_DIR}
    ${SRC_DIR}/database
    ${SRC_DIR}/memory
    ${SRC_DIR}/storage
    ${SRC_DIR}/index
    ${SRC_DIR}/lsm
    ${SRC_DIR}/benchmark
    ${SRC_DIR}/utils
    ${SRC_DIR}/query
)

# Windows-specific definitions
if(WIN32)
    add_definitions(-D_WIN32)
    add_definitions(-DNOMINMAX)  # Prevent Windows macros from breaking std::min/max
endif()

# Collect all the source files
file(GLOB_RECURSE SOURCES 
    ${SRC_DIR}/*.cpp
)

# Create the executable
add_executable(database_engine ${SOURCES})
target_include_directories(database_engine PRIVATE ${INCLUDE_DIRS})

# Add threading support
find_package(Threads REQUIRED)
target_link_libraries(database_engine PRIVATE Threads::Threads)

# Windows-specific libraries
if(WIN32)
    target_link_libraries(database_engine PRIVATE ws2_32)
endif()

# Output directory
set_target_properties(database_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Installation
install(TARGETS database_engine DESTINATION bin)

# Clean target - only cleans build artifacts, preserves CMake configuration
if(WIN32)
    add_custom_target(clean-artifacts
        COMMAND cmd.exe /C "echo Cleaning build artifacts..."
        COMMAND cmd.exe /C "if exist bin\\*.exe del /Q bin\\*.exe"
        COMMAND cmd.exe /C "if exist bin\\*.pdb del /Q bin\\*.pdb"
        COMMAND cmd.exe /C "if exist bin\\*.ilk del /Q bin\\*.ilk"
        COMMAND cmd.exe /C "if exist CMakeFiles\\database_engine.dir rmdir /S /Q CMakeFiles\\database_engine.dir"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Cleaning build artifacts while preserving CMake configuration..."
    )
else()
    add_custom_target(clean-artifacts
        COMMAND rm -f ${CMAKE_BINARY_DIR}/bin/*
        COMMAND rm -rf ${CMAKE_BINARY_DIR}/CMakeFiles/database_engine.dir
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Cleaning build artifacts while preserving CMake configuration..."
    )
endif()

# Purge target - removes everything under the build directory
if(WIN32)
    add_custom_target(purge-all
        COMMAND cmd.exe /C "echo Purging entire build directory..."
        COMMAND cmd.exe /C "for /D %%i in (*) do if not \"%%i\"==\"CMakeFiles\" rmdir /S /Q \"%%i\""
        COMMAND cmd.exe /C "for %%i in (*.*) do if not \"%%i\"==\"CMakeCache.txt\" del /Q \"%%i\""
        COMMAND cmd.exe /C "if exist CMakeFiles\\*.* del /S /Q CMakeFiles\\*.*"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Purging all files in build directory..."
    )
else()
    add_custom_target(purge-all
        COMMAND find ${CMAKE_BINARY_DIR} -mindepth 1 -delete
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Purging all files in build directory..."
    )
endif()