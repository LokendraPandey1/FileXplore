cmake_minimum_required(VERSION 3.10)
project(FileXplore VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/PathUtils.cpp
    src/FileManager.cpp
    src/DirManager.cpp
    src/CommandParser.cpp
    src/HistoryManager.cpp
    src/SystemInfo.cpp
    main.cpp
)

# Header files (for IDE support)
set(HEADERS
    include/PathUtils.h
    include/FileManager.h
    include/DirManager.h
    include/CommandParser.h
    include/HistoryManager.h
    include/SystemInfo.h
)

# Create executable
add_executable(FileXplore ${SOURCES} ${HEADERS})

# Link filesystem library if needed (for older compilers)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(FileXplore stdc++fs)
endif()

# Set output directory
set_target_properties(FileXplore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Installation
install(TARGETS FileXplore
    RUNTIME DESTINATION bin
)

# Print build information
message(STATUS "FileXplore Virtual File System Simulator")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# Custom target for running the application
add_custom_target(run
    COMMAND FileXplore
    DEPENDS FileXplore
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running FileXplore..."
)