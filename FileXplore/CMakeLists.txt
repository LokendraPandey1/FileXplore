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

# Find and configure Crow web framework
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(CROW QUIET crow)
endif()

# If Crow not found via pkg-config, try to find it manually
if(NOT CROW_FOUND)
    # Try to find Crow in standard locations
    find_path(CROW_INCLUDE_DIR
        NAMES crow_all.h
        PATHS
            /usr/include/crow
            /usr/local/include/crow
            /opt/homebrew/include/crow
            ${CMAKE_SOURCE_DIR}/external/crow/include
    )

    if(CROW_INCLUDE_DIR)
        set(CROW_INCLUDE_DIRS ${CROW_INCLUDE_DIR})
        set(CROW_FOUND TRUE)
    endif()
endif()

# If Crow still not found, try to download it via FetchContent
if(NOT CROW_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG        v1.0-5
    )
    FetchContent_GetProperties(crow)
    if(NOT crow_POPULATED)
        FetchContent_Populate(crow)
    endif()
    add_subdirectory(${crow_SOURCE_DIR} ${crow_BINARY_DIR})
    set(CROW_INCLUDE_DIRS ${crow_SOURCE_DIR}/include)
    set(CROW_FOUND TRUE)
endif()

# Find nlohmann JSON library
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.2
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Source files
set(SOURCES
    src/PathUtils.cpp
    src/FileManager.cpp
    src/DirManager.cpp
    src/CommandParser.cpp
    src/HistoryManager.cpp
    src/SystemInfo.cpp
    src/WebServer.cpp
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
    include/WebServer.h
)

# Create CLI executable (original)
add_executable(FileXplore ${SOURCES} ${HEADERS})

# Create GUI executable
add_executable(FileXplore-GUI ${SOURCES} ${HEADERS})

# Link filesystem library if needed (for older compilers)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(FileXplore stdc++fs)
    target_link_libraries(FileXplore-GUI stdc++fs)
endif()

# Link Crow and JSON libraries to GUI executable
if(CROW_FOUND)
    target_include_directories(FileXplore-GUI PRIVATE ${CROW_INCLUDE_DIRS})
endif()

if(nlohmann_json_FOUND)
    target_link_libraries(FileXplore-GUI nlohmann_json::nlohmann_json)
endif()

# Link pthread for Crow (needed on Unix systems)
if(UNIX)
    target_link_libraries(FileXplore-GUI pthread)
endif()

# Set output directory
set_target_properties(FileXplore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(FileXplore-GUI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Installation
install(TARGETS FileXplore
    RUNTIME DESTINATION bin
)
install(TARGETS FileXplore-GUI
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