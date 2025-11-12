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
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -pthread")
endif()

# Include directories (project + vendored Crow headers)
include_directories(
	"${CMAKE_SOURCE_DIR}/include"
	"${CMAKE_SOURCE_DIR}/third_party/include"
	"${CMAKE_SOURCE_DIR}/third_party/Crow-master/include"
)

# Avoid forcing system include paths; rely on the active compiler toolchain

# Define Crow static directory used by WebServer
add_definitions(-DCROW_STATIC_DIRECTORY="${CMAKE_SOURCE_DIR}/web")

# JSON (header-only) - prefer local vendored header to avoid Git requirement
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/include/nlohmann/json.hpp")
	message(STATUS "Using vendored nlohmann/json single-header")
	set(NLOHMANN_JSON_USE_VENDORED TRUE)
else()
	# Try system package first, then FetchContent as fallback
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
endif()

# Option to control GUI explicitly
option(ENABLE_GUI "Enable building the GUI (Crow web server)" ON)

# Determine GUI availability based on compiler (Crow requires full C++17 <string_view>)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0")
	message(WARNING "GCC ${CMAKE_CXX_COMPILER_VERSION} does not fully support C++17 <string_view>. Disabling GUI (Crow) for this build. Use MSYS2 MinGW-w64 (GCC >= 9) or MSVC to enable GUI.")
	set(ENABLE_GUI OFF)
endif()

# Check for standalone Asio header (warn only; don't hard-disable to allow manual setups)
if(ENABLE_GUI)
    # --- Asio Detection Fix (for MSYS2 + CMake internal checks) ---
    set(ASIO_PATH "C:/msys64/mingw64/include")

    if(EXISTS "${ASIO_PATH}/asio.hpp")
        message(STATUS "Found Asio header at: ${ASIO_PATH}/asio.hpp")

        # Make CMake's check_include_file_cxx() aware of this path
        set(CMAKE_REQUIRED_INCLUDES "${ASIO_PATH}")

        # Also add to project include directories (for actual build)
        include_directories("${ASIO_PATH}")
    else()
        message(WARNING "Asio header not found at ${ASIO_PATH}")
    endif()

    include(CheckIncludeFileCXX)
    check_include_file_cxx("asio.hpp" ASIO_HPP_FOUND)

    # Fallback: Force-detect if we know it exists
    if(EXISTS "${ASIO_PATH}/asio.hpp" AND NOT ASIO_HPP_FOUND)
        set(ASIO_HPP_FOUND TRUE)
        message(STATUS "Manually marking Asio header as found (CMake detection issue workaround).")
    endif()

    if(NOT ASIO_HPP_FOUND)
        message(WARNING "asio.hpp not found on your include path. Proceeding anyway; ensure Asio is installed (MSYS2: pacman -S mingw-w64-x86_64-asio).")
    else()
        message(STATUS "Asio header found and usable.")
    endif()
endif()


# Source files
set(SOURCES
	src/PathUtils.cpp
	src/FileManager.cpp
	src/DirManager.cpp
	src/CommandParser.cpp
	src/PersistenceManager.cpp
	src/HistoryManager.cpp
	src/SystemInfo.cpp
	src/CompressionManager.cpp
	main.cpp
)

if(ENABLE_GUI)
	list(APPEND SOURCES src/WebServer.cpp)
	add_definitions(-DENABLE_GUI=1)
	# Crow uses standalone Asio
	add_definitions(-DASIO_STANDALONE)
else()
	add_definitions(-DENABLE_GUI=0)
endif()

# Header files (for IDE support)
set(HEADERS
	include/PathUtils.h
	include/FileManager.h
	include/DirManager.h
	include/CommandParser.h
	include/HistoryManager.h
	include/SystemInfo.h
	include/CompressionManager.h
	include/WebServer.h
)

# Single executable (includes GUI web server; CLI remains available via terminal)
add_executable(FileXplore ${SOURCES} ${HEADERS})

# Link filesystem library if needed (for older compilers)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
	target_link_libraries(FileXplore stdc++fs)
endif()

# Link JSON
if(nlohmann_json_FOUND AND NOT NLOHMANN_JSON_USE_VENDORED)
	target_link_libraries(FileXplore nlohmann_json::nlohmann_json)
endif()

# Link pthread for Crow (needed on Unix systems)
if(UNIX)
	target_link_libraries(FileXplore pthread)
endif()

if(WIN32)
	# Winsock for networking
	target_link_libraries(FileXplore ws2_32 mswsock)
endif()

# Link zlib for compression support
find_package(ZLIB QUIET)
if(ZLIB_FOUND)
	target_link_libraries(FileXplore ${ZLIB_LIBRARIES})
	target_include_directories(FileXplore PRIVATE ${ZLIB_INCLUDE_DIRS})
	message(STATUS "Found zlib: ${ZLIB_LIBRARIES}")
else()
	# Try to find zlib in common locations (MSYS2)
	if(EXISTS "C:/msys64/mingw64/lib/libz.a" OR EXISTS "C:/msys64/mingw64/lib/libz.dll.a")
		target_link_libraries(FileXplore z)
		message(STATUS "Linking zlib from MSYS2")
	else()
		message(WARNING "zlib not found. Compression features may not work. Install zlib (MSYS2: pacman -S mingw-w64-x86_64-zlib)")
	endif()
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

# Custom targets for running the applications
add_custom_target(run
	COMMAND FileXplore
	DEPENDS FileXplore
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
	COMMENT "Running FileXplore..."
)
