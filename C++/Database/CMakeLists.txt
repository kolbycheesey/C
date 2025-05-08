cmake_minimum_required(VERSION 3.10)
project(HighPerformanceDatabase VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to enable or disable tests (default: OFF)
option(BUILD_TESTS "Build the test suite" OFF)

# MongoDB configuration options
option(USE_MONGODB "Enable MongoDB benchmarks" OFF)
set(MONGODB_ROOT_DIR "" CACHE PATH "MongoDB C++ Driver installation directory")

# Logging options
set(LOG_LEVEL "INFO" CACHE STRING "Set the logging level (DEBUG, INFO, WARNING, ERR, NONE)")
set_property(CACHE LOG_LEVEL PROPERTY STRINGS DEBUG INFO WARNING ERR NONE)
option(LOG_TO_FILE "Write logs to file instead of console" OFF)
set(LOG_FILE_PATH "" CACHE STRING "Path to the log file (empty for default)")

# Define source directories
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
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

# Add MongoDB support if enabled
set(MONGODB_FOUND FALSE)
if(USE_MONGODB)
    # Check if we're using vcpkg
    if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
        set(USING_VCPKG TRUE)
        # Extract vcpkg root directory from toolchain file path
        string(REGEX REPLACE "/scripts/buildsystems/vcpkg.cmake$" "" VCPKG_ROOT "${CMAKE_TOOLCHAIN_FILE}")
        message(STATUS "Using vcpkg from ${VCPKG_ROOT}")
        
        # When using vcpkg, add the share directory to module path for mongocxx config files
        list(APPEND CMAKE_PREFIX_PATH 
            "${VCPKG_ROOT}/installed/x64-windows/share"
            "${VCPKG_ROOT}/installed/x86-windows/share"
        )
        
        # Use CONFIG mode specifically as recommended by vcpkg
        message(STATUS "Looking for MongoDB C++ driver with vcpkg's CONFIG mode...")
        find_package(bsoncxx CONFIG QUIET)
        find_package(mongocxx CONFIG QUIET)
        
        if(bsoncxx_FOUND AND mongocxx_FOUND)
            message(STATUS "Found MongoDB C++ driver via vcpkg")
            set(MONGODB_FOUND TRUE)
            
            # Setup the libraries based on what's available (static or shared)
            if(TARGET mongo::bsoncxx_static AND TARGET mongo::mongocxx_static)
                message(STATUS "Using static MongoDB libraries")
                set(BSONCXX_LIBRARIES mongo::bsoncxx_static)
                set(MONGOCXX_LIBRARIES mongo::mongocxx_static)
            else()
                message(STATUS "Using shared MongoDB libraries")
                set(BSONCXX_LIBRARIES mongo::bsoncxx_shared)
                set(MONGOCXX_LIBRARIES mongo::mongocxx_shared)
            endif()
            
            # Add MongoDB-related definitions
            add_definitions(-DUSE_MONGODB)
        else()
            message(WARNING "MongoDB C++ driver not found in vcpkg. Check your installation.")
            message(STATUS "Trying without vcpkg...")
        endif()
    endif()
    
    # If not found through vcpkg CONFIG mode, try standard approach
    if(NOT MONGODB_FOUND)
        # Set MongoDB search paths based on user input
        if(MONGODB_ROOT_DIR)
            list(APPEND CMAKE_PREFIX_PATH "${MONGODB_ROOT_DIR}")
        endif()
        
        # Check common installation directories
        if(WIN32)
            list(APPEND CMAKE_PREFIX_PATH 
                "C:/Program Files/mongo-cxx-driver"
                "C:/mongo-cxx-driver"
            )
        else()
            list(APPEND CMAKE_PREFIX_PATH 
                "/usr/local"
                "/usr/local/mongodb-cxx-driver"
                "/usr"
            )
        endif()
        
        # Try to find MongoDB using standard approach
        find_package(mongocxx QUIET)
        find_package(bsoncxx QUIET)
        
        if(mongocxx_FOUND AND bsoncxx_FOUND)
            set(MONGODB_FOUND TRUE)
            message(STATUS "Found MongoDB C++ driver through standard method")
        endif()
    endif()
    
    # Final check if MongoDB was found through any method
    if(MONGODB_FOUND)
        message(STATUS "MongoDB C++ driver found. MongoDB benchmark support enabled.")
    else()
        if(DEFINED USING_VCPKG AND USING_VCPKG)
            message(WARNING "MongoDB C++ driver not found through vcpkg.")
            message(STATUS "Please make sure you've properly installed it:")
            message(STATUS "  1. Run: vcpkg install mongo-cxx-driver:x64-windows")
            message(STATUS "  2. Run: vcpkg integrate install")
            message(STATUS "  3. Then try building again")
        else()
            message(WARNING "MongoDB C++ driver not found. MongoDB benchmarks will be disabled.")
            message(STATUS "To enable MongoDB support, try one of the following:")
            message(STATUS "  1. Install the MongoDB C++ driver")
            message(STATUS "  2. Set MONGODB_ROOT_DIR to your MongoDB installation directory")
            message(STATUS "  3. Use vcpkg: install mongo-cxx-driver:x64-windows and use the toolchain file")
        endif()
        
        # Disable MongoDB support
        set(USE_MONGODB OFF)
    endif()
endif()

# Configure logging
if(LOG_LEVEL STREQUAL "DEBUG")
    add_definitions(-DLOG_LEVEL=LOG_LEVEL_DEBUG)
elseif(LOG_LEVEL STREQUAL "INFO")
    add_definitions(-DLOG_LEVEL=LOG_LEVEL_INFO)
elseif(LOG_LEVEL STREQUAL "WARNING")
    add_definitions(-DLOG_LEVEL=LOG_LEVEL_WARNING)
elseif(LOG_LEVEL STREQUAL "ERR")
    add_definitions(-DLOG_LEVEL=LOG_LEVEL_ERROR)
elseif(LOG_LEVEL STREQUAL "NONE")
    add_definitions(-DLOG_LEVEL=LOG_LEVEL_NONE)
endif()

if(LOG_TO_FILE)
    add_definitions(-DLOG_TO_FILE)
    if(NOT "${LOG_FILE_PATH}" STREQUAL "")
        add_definitions(-DLOG_FILE_PATH="${LOG_FILE_PATH}")
    endif()
endif()

# Collect all the source files for the main application (excluding main.cpp)
file(GLOB_RECURSE LIB_SOURCES 
    ${SRC_DIR}/benchmark/*.cpp
    ${SRC_DIR}/database/*.cpp
    ${SRC_DIR}/memory/*.cpp
    ${SRC_DIR}/storage/*.cpp
    ${SRC_DIR}/utils/*.cpp
    ${SRC_DIR}/query/*.cpp
)

# Create the main executable
add_executable(database_engine ${SRC_DIR}/main.cpp ${LIB_SOURCES})
target_include_directories(database_engine PRIVATE ${INCLUDE_DIRS})

# Only build tests if BUILD_TESTS is enabled
if(BUILD_TESTS)
    # Create a benchmark test executable
    add_executable(benchmark_test ${TESTS_DIR}/benchmark.cpp ${LIB_SOURCES})
    target_include_directories(benchmark_test PRIVATE ${INCLUDE_DIRS})

    # Create other test executables
    file(GLOB TEST_SOURCES ${TESTS_DIR}/*.cpp)
    foreach(TEST_SOURCE ${TEST_SOURCES})
        if(NOT TEST_SOURCE MATCHES "benchmark.cpp")
            get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
            add_executable(${TEST_NAME} ${TEST_SOURCE} ${LIB_SOURCES})
            target_include_directories(${TEST_NAME} PRIVATE ${INCLUDE_DIRS})
            
            # Set the output directory
            set_target_properties(${TEST_NAME} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
            )
        endif()
    endforeach()
endif()

# Add threading support
find_package(Threads REQUIRED)
target_link_libraries(database_engine PRIVATE Threads::Threads)

# Conditionally link the test executables with Threads
if(BUILD_TESTS)
    target_link_libraries(benchmark_test PRIVATE Threads::Threads)

    # Link with MongoDB if enabled
    if(USE_MONGODB)
        target_include_directories(benchmark_test PRIVATE ${MONGOCXX_INCLUDE_DIRS} ${BSONCXX_INCLUDE_DIRS})
        target_link_libraries(benchmark_test PRIVATE ${MONGOCXX_LIBRARIES} ${BSONCXX_LIBRARIES})
    endif()

    # Make sure all test executables have the Threads library
    get_property(TEST_TARGETS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(TARGET ${TEST_TARGETS})
        if(TARGET ${TARGET} AND NOT ${TARGET} MATCHES "database_engine|benchmark_test|clean-artifacts|purge-all|ZERO_CHECK|ALL_BUILD")
            target_link_libraries(${TARGET} PRIVATE Threads::Threads)
        endif()
    endforeach()
endif()

# Windows-specific libraries
if(WIN32)
    target_link_libraries(database_engine PRIVATE ws2_32)
    
    if(BUILD_TESTS)
        target_link_libraries(benchmark_test PRIVATE ws2_32)
        
        # Add ws2_32 to all test targets
        foreach(TARGET ${TEST_TARGETS})
            if(TARGET ${TARGET} AND NOT ${TARGET} MATCHES "database_engine|benchmark_test|clean-artifacts|purge-all|ZERO_CHECK|ALL_BUILD")
                target_link_libraries(${TARGET} PRIVATE ws2_32)
            endif()
        endforeach()
    endif()
endif()

# Output directories
set_target_properties(database_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(BUILD_TESTS)
    set_target_properties(benchmark_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
    )

    # Add an 'all_tests' target that builds all tests
    add_custom_target(all_tests)
    add_dependencies(all_tests benchmark_test)
    foreach(TARGET ${TEST_TARGETS})
        if(TARGET ${TARGET} AND NOT ${TARGET} MATCHES "database_engine|benchmark_test|clean-artifacts|purge-all|ZERO_CHECK|ALL_BUILD")
            add_dependencies(all_tests ${TARGET})
        endif()
    endforeach()
endif()

# Installation
install(TARGETS database_engine DESTINATION bin)
if(BUILD_TESTS)
    install(TARGETS benchmark_test DESTINATION bin/tests)
endif()

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