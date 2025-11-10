# FileXplore GUI Implementation

This document describes the GUI implementation for FileXplore, which provides a responsive web-based interface for the virtual file system simulator.

## Overview

The FileXplore GUI implementation adds a web-based interface to the existing CLI file system simulator, maintaining full compatibility with the current C++ backend architecture.

## Architecture

### Backend Components

1. **WebServer.h/cpp** - HTTP server using Crow web framework
   - RESTful API endpoints for all CLI commands
   - Static file serving for frontend
   - JSON response formatting
   - CORS support for local development

2. **Updated main.cpp** - Application entry point with GUI mode
   - `--gui` or `-g` flag to start GUI mode
   - Web server initialization and management
   - Dual-mode support (CLI and GUI)

3. **Updated CMakeLists.txt** - Build configuration
   - Crow web framework dependency management
   - nlohmann::json library integration
   - Separate targets for CLI and GUI executables

### Frontend Components

1. **index.html** - Main GUI application structure
   - Responsive single-page application
   - Semantic HTML5 structure
   - Accessibility features

2. **styles.css** - Responsive CSS design
   - CSS Grid/Flexbox layout system
   - Mobile-first responsive breakpoints
   - Dark/light theme support
   - Modern design with smooth transitions

3. **app.js** - Frontend application logic
   - Vanilla JavaScript (no framework dependencies)
   - API client for backend communication
   - File system visualization
   - Real-time state management
   - Keyboard shortcuts and touch support

## Features

### Core Functionality
- Visual file system navigation
- File and directory operations (create, delete, rename, move, copy)
- Command execution through GUI and direct command input
- Real-time file system state visualization
- Drag-and-drop file upload

### User Interface
- **Header**: Application title, breadcrumb navigation, view controls, theme toggle
- **Sidebar**: Quick actions, system information display
- **Main Content**: File explorer with multiple view modes (list, grid, tree)
- **Toolbar**: Search, sort options, refresh controls
- **Command Interface**: Interactive command line with history
- **Status Bar**: Current status and selection information

### Responsive Design
- **Mobile (320px-767px)**: Stacked layout, simplified controls
- **Tablet (768px-1023px)**: Side-by-side layout, touch-friendly
- **Desktop (1024px+)**: Full layout with all panels visible
- **Large Desktop (1440px+)**: Enhanced spacing and information

### Advanced Features
- Dark/light theme switching
- Keyboard shortcuts (Ctrl+F for search, F5 to refresh, etc.)
- Context menus for file operations
- File preview modal
- Toast notifications
- Loading states and error handling

## API Endpoints

The GUI communicates with the backend through RESTful API endpoints:

- `GET /api/filesystem` - Get current directory structure
- `POST /api/command` - Execute CLI commands
- `GET /api/file/{path}` - Download file content
- `POST /api/file/{path}` - Upload/write file content
- `GET /api/history` - Get command history
- `GET /api/system` - Get system information

## Building and Running

### Prerequisites
- C++17 compatible compiler
- Crow web framework
- nlohmann::json library
- CMake 3.10+ (or use provided Makefile)

### Build Instructions

#### Using CMake
```bash
mkdir build
cd build
cmake ..
make
```

#### Using Makefile
```bash
make all          # Build both CLI and GUI
make cli          # Build CLI only
make gui          # Build GUI only
```

### Running the Application

#### CLI Mode
```bash
./FileXplore-CLI
./FileXplore-CLI /path/to/vfs
```

#### GUI Mode
```bash
./FileXplore-GUI --gui
./FileXplore-GUI --gui /path/to/vfs
```

#### Using CMake targets
```bash
make run-cli      # Build and run CLI
make run-gui      # Build and run GUI
```

### Testing the Frontend

A Python test server is provided for testing the frontend without the C++ backend:

```bash
cd web
python3 test-server.py [port]
```

The test server runs on `http://localhost:8080` by default and provides mock data for testing the GUI interface.

## File Structure

```
FileXplore/
├── include/
│   ├── WebServer.h              # Web server interface
│   └── [existing headers]
├── src/
│   ├── WebServer.cpp            # Web server implementation
│   └── [existing source files]
├── web/                         # Frontend files
│   ├── index.html               # Main HTML structure
│   ├── styles.css               # CSS styling
│   ├── app.js                   # JavaScript application
│   └── test-server.py           # Python test server
├── CMakeLists.txt               # Updated build configuration
├── Makefile                     # Alternative build system
├── main.cpp                     # Updated entry point
└── README-GUI.md               # This file
```

## Security Considerations

- Path validation prevents directory traversal attacks
- Input sanitization for all user inputs
- CORS configuration restricts access to localhost by default
- File access controls maintain existing sandbox restrictions

## Performance Optimizations

- Lazy loading for large directories
- Debounced search requests
- Efficient DOM manipulation
- Minimal external dependencies

## Browser Compatibility

The GUI is designed to work with modern browsers:
- Chrome 80+
- Firefox 75+
- Safari 13+
- Edge 80+

## Future Enhancements

Potential future features include:
- Integrated text editor with syntax highlighting
- File preview for images and documents
- Batch operations with progress tracking
- Full-text search across file contents
- Multiple theme options
- Keyboard navigation for power users
- File sharing capabilities

## Troubleshooting

### Build Issues
- Ensure C++17 support is enabled
- Install Crow web framework (`git clone https://github.com/CrowCpp/Crow.git`)
- Install nlohmann/json (`git clone https://github.com/nlohmann/json.git`)

### Runtime Issues
- Check that port 8080 is available
- Verify file permissions on VFS root directory
- Ensure JavaScript is enabled in browser

### Development Tips
- Use the Python test server for frontend development
- Test with different screen sizes for responsive design
- Check browser console for JavaScript errors
- Use browser developer tools for debugging API requests

## License

This GUI implementation maintains the same license as the original FileXplore project.