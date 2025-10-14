# FileXplore - Virtual File System Simulator

A complete CLI-based virtual file system simulator implemented in C++17 that operates on a real folder sandbox while providing a Unix-like command interface.

## 🚀 Features

### Core Functionality
- **Sandboxed Virtual File System**: All operations are contained within a designated root directory
- **Unix-like Commands**: Familiar command-line interface with standard file and directory operations
- **Path Safety**: Prevents directory traversal attacks and access outside the sandbox
- **Persistent Storage**: Works with real files and directories for data persistence

### Supported Commands

#### Directory Operations
- `mkdir <path>` - Create directory
- `rmdir <path>` - Remove empty directory  
- `ls [path]` - List directory contents
- `tree [path]` - Display directory tree structure
- `cd <path>` - Change current directory
- `pwd` - Show current working directory

#### File Operations
- `create <path>` - Create empty file
- `write <path> "content"` - Write content to file (overwrite)
- `append <path> "content"` - Append content to file
- `read <path>` - Display file content
- `delete <path>` - Delete file

#### System & Utility
- `df` - Show disk usage statistics
- `history` - Show last 20 commands
- `clear` - Clear terminal screen
- `help` - Display help information
- `exit` - Exit FileXplore

### Path Support
- **Absolute paths**: `/home/user/file.txt`
- **Relative paths**: `documents/file.txt`
- **Current directory**: `./file.txt` or `file.txt`
- **Parent directory**: `../file.txt`

## 🛠️ Building the Project

### Prerequisites
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10+ (optional, for CMake build)
- Make (optional, for Makefile build)

### Build Options

#### Option 1: CMake (Recommended)
```bash
mkdir build
cd build
cmake ..
make
```

#### Option 2: Makefile
```bash
make
```

#### Option 3: Manual Compilation
```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp main.cpp -o FileXplore
```

### Windows Build
```cmd
# Using CMake
mkdir build
cd build
cmake ..
cmake --build .

# Using MSVC directly
cl /std:c++17 /EHsc /Iinclude src\*.cpp main.cpp /Fe:FileXplore.exe
```

## 🎮 Usage

### Starting FileXplore
```bash
# Use default VFS root (./filexplore_root)
./FileXplore

# Use custom VFS root directory
./FileXplore /path/to/custom/root
```

### Example Session
```bash
FileXplore:/$ mkdir /home
Directory created: /home

FileXplore:/$ cd /home
Changed directory to: /home

FileXplore:/home$ create welcome.txt
File created: welcome.txt

FileXplore:/home$ write welcome.txt "Hello, FileXplore!"
Content written to file: welcome.txt

FileXplore:/home$ read welcome.txt
Content of welcome.txt:
--------------------------------------------------
Hello, FileXplore!
--------------------------------------------------

FileXplore:/home$ ls
Contents of .:
  welcome.txt

FileXplore:/home$ tree /
/
└── home/
    └── welcome.txt

FileXplore:/home$ df
======================================================================
FileXplore Virtual File System Statistics
======================================================================
VFS Root:           /path/to/filexplore_root
Current Directory:  /home
----------------------------------------------------------------------
Total Files:        1
Total Directories:  1
Total Size:         17.00 B (17 bytes)
======================================================================
```

### Demo Commands
Run the included demo to see all features:
```bash
# Copy commands from demo_commands.txt and paste them into FileXplore
cat demo_commands.txt
```

## 🏗️ Architecture

### Class Structure
- **PathUtils**: Safe path resolution and sandbox security
- **FileManager**: File operations (create, read, write, append, delete)
- **DirManager**: Directory operations (mkdir, rmdir, ls, tree, cd, pwd)
- **CommandParser**: CLI command parsing and execution
- **HistoryManager**: Command history management (last 20 commands)
- **SystemInfo**: System statistics and disk usage information

### Security Features
- **Sandbox Enforcement**: All paths are validated to prevent access outside VFS root
- **Path Normalization**: Resolves `..`, `.`, and other path components safely
- **Input Validation**: Comprehensive validation of all user inputs
- **Error Handling**: Graceful handling of filesystem errors and edge cases

## 📁 Project Structure
```
FileXplore/
├── include/                 # Header files
│   ├── PathUtils.h
│   ├── FileManager.h
│   ├── DirManager.h
│   ├── CommandParser.h
│   ├── HistoryManager.h
│   └── SystemInfo.h
├── src/                     # Source files
│   ├── PathUtils.cpp
│   ├── FileManager.cpp
│   ├── DirManager.cpp
│   ├── CommandParser.cpp
│   ├── HistoryManager.cpp
│   └── SystemInfo.cpp
├── main.cpp                 # Main application entry point
├── CMakeLists.txt          # CMake build configuration
├── Makefile                # Make build configuration
├── demo_commands.txt       # Demo commands showcase
└── README.md               # This file
```

## 🔧 Technical Details

### Requirements
- **Language**: C++17
- **Standard Library**: Uses only STL (no external dependencies)
- **Filesystem**: Uses `std::filesystem` for cross-platform compatibility
- **Compiler Support**: GCC 7+, Clang 5+, MSVC 2017+

### Key Features
- **Cross-platform**: Works on Windows, Linux, and macOS
- **Memory Safe**: Uses RAII and smart resource management
- **Exception Safe**: Proper exception handling throughout
- **Performance**: Efficient file operations with minimal overhead

## 🐛 Error Handling

FileXplore provides comprehensive error handling for:
- Invalid paths and directory traversal attempts
- File/directory not found scenarios
- Permission denied situations
- Disk space and filesystem errors
- Invalid command syntax and arguments

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## 📄 License

This project is open source and available under the MIT License.

## 🙏 Acknowledgments

- Built with modern C++17 features
- Inspired by Unix/Linux filesystem commands
- Designed for educational and practical use

---

**FileXplore** - Experience the power of virtual file systems! 🚀