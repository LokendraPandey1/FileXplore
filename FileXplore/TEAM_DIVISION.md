# FileXplore Project - Team Division

This document outlines the division of work among 4 teammates for the FileXplore Virtual File System Simulator project.

---

## 📋 Overview

The project is divided into 4 main areas:
- **Teammate 1**: GUI Frontend & CLI Interface
- **Teammate 2**: HistoryManager, PersistenceManager, SystemInfo, and WebServer
- **Teammate 3**: PathUtils, FileManager, and CompressionManager
- **Teammate 4**: DirManager, main.cpp (integration), and Build System

---

## 👤 Teammate 1: GUI and CLI

### Responsibilities
- **GUI Frontend Development** (Web Interface)
- **CLI Command Parser** (Command-line interface logic)
- **Integration between CLI and GUI**

### Files Assigned

#### GUI Frontend (Web Interface)
- `web/index.html` - Main HTML structure and layout
- `web/styles.css` - CSS styling and themes (dark/light mode)
- `web/app.js` - JavaScript application logic, API calls, UI interactions
- `web/test-server.py` - Python test server for frontend development

#### CLI Interface
- `include/CommandParser.h` - Command parser header (command definitions, parsing logic)
- `src/CommandParser.cpp` - Command parser implementation (all command handlers)
  - Handles: mkdir, rmdir, ls, tree, cd, pwd, create, write, append, read, delete, help, clear, history, df, zip, unzip, exit

#### Main Integration (Partial)
- `main.cpp` - CLI mode implementation (lines 150-211)
  - CLI welcome message
  - CLI main loop
  - Command execution loop
  - CLI-specific state saving

### Key Tasks
1. Implement and maintain the web-based GUI interface
2. Handle all user interactions in the web interface
3. Implement API communication between frontend and backend
4. Develop CLI command parsing and execution logic
5. Ensure consistent user experience between CLI and GUI modes
6. Implement command help system
7. Handle input validation for CLI commands

### Dependencies
- Works with: WebServer (Teammate 2), FileManager (Teammate 3), DirManager (Teammate 4), CompressionManager (Teammate 3)
- Uses: HistoryManager (Teammate 2) for command history in CLI

---

## 👤 Teammate 2: HistoryManager, PersistenceManager, SystemInfo, and WebServer

### Responsibilities
- **Command History Management**
- **Session Persistence**
- **System Information & Statistics**
- **Web Server Backend** (HTTP API endpoints)

### Files Assigned

#### History Management
- `include/HistoryManager.h` - Command history header
- `src/HistoryManager.cpp` - Command history implementation
  - Stores last 20 commands
  - Save/load history from disk
  - History retrieval

#### Persistence Management
- `include/PersistenceManager.h` - Persistence system header
- `src/PersistenceManager.cpp` - Persistence implementation
  - Session state saving/loading
  - VFS state persistence
  - File-based storage

#### System Information
- `include/SystemInfo.h` - System info header
- `src/SystemInfo.cpp` - System info implementation
  - Disk usage statistics (df command)
  - File/directory counts
  - Size calculations
  - VFS statistics

#### Web Server (Backend API)
- `include/WebServer.h` - Web server header
- `src/WebServer.cpp` - Web server implementation
  - HTTP server setup (Crow framework)
  - RESTful API endpoints:
    - `/api/filesystem` - Get directory structure
    - `/api/command` - Execute CLI commands
    - `/api/file/{path}` - File operations (read/write)
    - `/api/history` - Get command history
    - `/api/system` - Get system information
    - `/api/compress` - Compress files
    - `/api/decompress` - Extract zip files
  - Static file serving for frontend
  - JSON response formatting
  - CORS configuration

#### Main Integration (Partial)
- `main.cpp` - GUI mode and persistence initialization (lines 101-117, 122-148, 197-208)
  - Persistence system initialization
  - VFS state loading
  - History loading
  - GUI mode server startup
  - State saving on exit

### Key Tasks
1. Implement command history tracking and storage
2. Develop session persistence system
3. Create system statistics and disk usage reporting
4. Build RESTful API backend for GUI
5. Handle HTTP requests and responses
6. Integrate all backend services with main application
7. Ensure data persistence across sessions

### Dependencies
- Works with: PathUtils (Teammate 3), FileManager (Teammate 3), DirManager (Teammate 4), CompressionManager (Teammate 3)
- Uses: Crow web framework, nlohmann::json

---

## 👤 Teammate 3: PathUtils, FileManager, and CompressionManager

### Responsibilities
- **Path Resolution & Security** (Sandbox enforcement)
- **File Operations** (Create, Read, Write, Append, Delete)
- **Compression/Decompression** (ZIP support)

### Files Assigned

#### Path Utilities & Security
- `include/PathUtils.h` - Path utilities header
- `src/PathUtils.cpp` - Path utilities implementation
  - Virtual to real path conversion
  - Path normalization and resolution
  - Sandbox security checks
  - Directory traversal prevention
  - Path validation
  - Current directory management
  - VFS root initialization

#### File Management
- `include/FileManager.h` - File manager header
- `src/FileManager.cpp` - File manager implementation
  - `createFile()` - Create empty files
  - `writeFile()` - Write content to files (overwrite)
  - `appendFile()` - Append content to files
  - `readFile()` - Read file contents
  - `deleteFile()` - Delete files
  - `fileExists()` - Check file existence
  - `getFileSize()` - Get file size

#### Compression Management
- `include/CompressionManager.h` - Compression manager header
- `src/CompressionManager.cpp` - Compression manager implementation
  - `compressToZip()` - Compress files/directories to ZIP
  - `decompressFromZip()` - Extract ZIP files
  - `isZipFile()` - Validate ZIP files
  - `listZipContents()` - List ZIP archive contents
  - Uses zlib for compression

### Key Tasks
1. Implement secure path resolution and sandbox enforcement
2. Prevent directory traversal attacks
3. Develop all file I/O operations
4. Implement ZIP compression and decompression
5. Ensure all operations respect VFS boundaries
6. Handle file system errors gracefully
7. Optimize file operations for performance

### Dependencies
- Works with: DirManager (Teammate 4) for directory operations
- Uses: Standard C++ filesystem library, zlib library
- Provides foundation for: CommandParser (Teammate 1), WebServer (Teammate 2)

---

## 👤 Teammate 4: DirManager, main.cpp (Integration), and Build System

### Responsibilities
- **Directory Operations** (Create, Remove, List, Navigate)
- **Application Integration** (main.cpp)
- **Build System** (CMake, Makefile)

### Files Assigned

#### Directory Management
- `include/DirManager.h` - Directory manager header
- `src/DirManager.cpp` - Directory manager implementation
  - `createDirectory()` - Create directories (mkdir)
  - `removeDirectory()` - Remove empty directories (rmdir)
  - `listDirectory()` - List directory contents (ls)
  - `displayTree()` - Display directory tree structure (tree)
  - `changeDirectory()` - Change current directory (cd)
  - `getCurrentDirectory()` - Get current directory (pwd)
  - `directoryExists()` - Check directory existence
  - `isDirectoryEmpty()` - Check if directory is empty

#### Application Integration
- `main.cpp` - Main application entry point (full file)
  - Application initialization
  - Command-line argument parsing
  - VFS root initialization
  - Mode selection (CLI/GUI)
  - Integration of all components
  - Application lifecycle management

#### Build System
- `CMakeLists.txt` - CMake build configuration
  - Compiler settings
  - Dependency management (Crow, nlohmann::json, Asio, zlib)
  - Source file compilation
  - Library linking
  - Build options (ENABLE_GUI)
- `Makefile` - Alternative build system
  - Build targets
  - Compilation rules
  - Clean targets

### Key Tasks
1. Implement all directory operations
2. Develop directory tree visualization
3. Integrate all components in main.cpp
4. Handle application startup and shutdown
5. Manage build system configuration
6. Ensure cross-platform compatibility
7. Handle command-line arguments
8. Coordinate between CLI and GUI modes

### Dependencies
- Works with: PathUtils (Teammate 3) for path operations
- Integrates: All other components
- Uses: CMake, Make, compiler toolchains

---

## 🔗 Component Dependencies

### Dependency Graph
```
main.cpp (T4)
  ├── PathUtils (T3) ──┐
  ├── CommandParser (T1) ──┐
  ├── FileManager (T3) ──┐  │
  ├── DirManager (T4) ───┼──┼── All use PathUtils
  ├── CompressionManager (T3) ──┘
  ├── HistoryManager (T2)
  ├── PersistenceManager (T2)
  ├── SystemInfo (T2)
  └── WebServer (T2) ── Uses all above components
```

### Integration Points

1. **CommandParser (T1)** calls:
   - FileManager (T3) - for file operations
   - DirManager (T4) - for directory operations
   - CompressionManager (T3) - for zip/unzip
   - HistoryManager (T2) - for command history
   - SystemInfo (T2) - for df command

2. **WebServer (T2)** calls:
   - All managers (T3, T4) - for operations
   - HistoryManager (T2) - for history API
   - SystemInfo (T2) - for system API

3. **All components** use:
   - PathUtils (T3) - for path resolution and security

---

## 📝 Development Guidelines

### Code Style
- Follow existing code style and naming conventions
- Use C++17 standard features
- Add comments for complex logic
- Handle errors gracefully

### Testing
- Test your components independently
- Test integration with dependent components
- Test edge cases and error conditions

### Communication
- Coordinate API changes with dependent teammates
- Document any interface changes
- Share progress updates regularly

### Git Workflow
- Create feature branches for your work
- Commit frequently with clear messages
- Coordinate merges to avoid conflicts

---

## 🎯 Milestones

### Phase 1: Core Components (Week 1-2)
- Teammate 3: PathUtils, FileManager, CompressionManager
- Teammate 4: DirManager, Build System

### Phase 2: Backend Services (Week 2-3)
- Teammate 2: HistoryManager, PersistenceManager, SystemInfo, WebServer

### Phase 3: User Interfaces (Week 3-4)
- Teammate 1: CLI (CommandParser), GUI Frontend

### Phase 4: Integration (Week 4-5)
- Teammate 4: main.cpp integration
- All: Testing and bug fixes

---

## 📚 Additional Resources

### Documentation Files
- `README.md` - Project documentation
- `demo_commands.txt` - Demo commands for testing

### Third-Party Dependencies
- `third_party/Crow-master/` - Crow web framework
- `third_party/include/nlohmann/` - JSON library
- `third_party/include/crow/` - Crow headers

### Test Data
- `filexplore_root/` - Default VFS root directory for testing

---

## ✅ Checklist for Each Teammate

### Teammate 1 (GUI & CLI)
- [ ] Implement all CLI commands in CommandParser
- [ ] Create responsive web GUI
- [ ] Implement API communication
- [ ] Add keyboard shortcuts
- [ ] Implement drag-and-drop
- [ ] Test CLI and GUI modes

### Teammate 2 (Backend Services)
- [ ] Implement command history (20 commands)
- [ ] Implement persistence system
- [ ] Implement system statistics
- [ ] Create all API endpoints
- [ ] Test API with frontend
- [ ] Handle CORS and security

### Teammate 3 (Path, File, Compression)
- [ ] Implement path resolution
- [ ] Implement sandbox security
- [ ] Implement all file operations
- [ ] Implement ZIP compression
- [ ] Test security boundaries
- [ ] Optimize performance

### Teammate 4 (Directory, Integration, Build)
- [ ] Implement all directory operations
- [ ] Implement tree visualization
- [ ] Integrate all components in main.cpp
- [ ] Configure build system
- [ ] Test cross-platform compatibility
- [ ] Document build process

---

**Last Updated**: Project Division Document
**Version**: 1.0

