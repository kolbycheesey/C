# CMakeLists.txt for cpp_wrapper

cmake_minimum_required(VERSION 3.15)

project(cpp_wrapper)

# Set C++17 standard explicitly
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Specify the source files
set(SOURCES
    db_bridge.cpp
)

# Create a library from the source files
add_library(cpp_wrapper STATIC
    db_bridge.cpp
    db_bridge.h
)

# Include directories for the wrapper
target_include_directories(cpp_wrapper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../src
)

# Link against the database engine library
target_link_libraries(cpp_wrapper PUBLIC database_engine)