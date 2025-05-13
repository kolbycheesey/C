cmake_minimum_required(VERSION 3.15)
project(MiniDBAPI VERSION 0.1 LANGUAGES CXX)

# Set C++ standard globally for all targets
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Python development libraries
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
message(STATUS "Found Python: ${Python3_EXECUTABLE}")
message(STATUS "Python include dirs: ${Python3_INCLUDE_DIRS}")
message(STATUS "Python libraries: ${Python3_LIBRARIES}")

# Include directories for the database source code
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# Specify the path to the main database_engine.lib static library - UPDATED CORRECT PATH
set(DATABASE_ENGINE_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../build/lib/Release")
set(DATABASE_ENGINE_LIB "${DATABASE_ENGINE_LIB_DIR}/database_engine.lib")

# Check if the library exists, otherwise use Debug path
if(NOT EXISTS ${DATABASE_ENGINE_LIB})
    set(DATABASE_ENGINE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../build/lib/Debug/database_engine.lib")
    if(NOT EXISTS ${DATABASE_ENGINE_LIB})
        message(WARNING "database_engine.lib not found in expected locations. You may need to build the main project first.")
    else()
        message(STATUS "Found database_engine.lib at: ${DATABASE_ENGINE_LIB}")
    endif()
else()
    message(STATUS "Found database_engine.lib at: ${DATABASE_ENGINE_LIB}")
endif()

# Link directories for finding libraries - UPDATED CORRECT PATHS
link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../build/lib/Release
    ${CMAKE_CURRENT_SOURCE_DIR}/../build/lib/Debug
    ${CMAKE_CURRENT_SOURCE_DIR}/../build/lib
)

# Add the cpp_wrapper subdirectory
add_subdirectory(src/cpp_wrapper)

# Set up the Python module
add_library(_db_module MODULE
    bindings/db_module.cpp
)

# Set the output name for the module
set_target_properties(_db_module PROPERTIES
    PREFIX ""
    OUTPUT_NAME "_db_module"
    SUFFIX "${Python3_SUFFIX}"
)

if(WIN32)
    # Use /bigobj for Windows to handle large object files
    target_compile_options(_db_module PRIVATE /bigobj)
endif()

# Include directories for the module
target_include_directories(_db_module PRIVATE
    ${Python3_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# Link against the Python libraries, cpp_wrapper, and database_engine.lib
target_link_libraries(_db_module PRIVATE
    ${Python3_LIBRARIES}
    cpp_wrapper
    ${DATABASE_ENGINE_LIB}
)

# Find threading package and link
find_package(Threads REQUIRED)
target_link_libraries(_db_module PRIVATE Threads::Threads)

# Windows-specific libraries
if(WIN32)
    target_link_libraries(_db_module PRIVATE ws2_32)
endif()

# Install targets
install(TARGETS _db_module
    LIBRARY DESTINATION python-cpp-db-api
)

# Setup package build (optional)
if(Python3_FOUND)
    message(STATUS "Python setup.py will be used for installation")
    
    # Copy Python package files to build directory
    file(COPY 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/python/
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/python-cpp-db-api
    )
    
    # Install using setup.py
    install(CODE "execute_process(COMMAND ${Python3_EXECUTABLE} setup.py install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
endif()