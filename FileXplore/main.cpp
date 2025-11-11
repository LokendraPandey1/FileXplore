#include "include/PathUtils.h"
#include "include/CommandParser.h"
#include "include/SystemInfo.h"
#if ENABLE_GUI
#include "include/WebServer.h"
#endif
#include "include/PersistenceManager.h"
#include "include/HistoryManager.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>

#ifdef _WIN32
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <windows.h>
#endif

using namespace std;

void displayWelcome() {
    cout << string(70, '=') << endl;
    cout << "Welcome to FileXplore - Virtual File System Simulator" << endl;
    cout << "Version 1.0 - C++17 Implementation" << endl;
    cout << string(70, '=') << endl;
    cout << "Type 'help' for available commands or 'exit' to quit." << endl;
    cout << string(70, '-') << endl;
}

void displayGUIWelcome() {
    cout << string(70, '=') << endl;
    cout << "FileXplore GUI Mode - Virtual File System Simulator" << endl;
    cout << "Version 1.0 - C++17 Implementation" << endl;
    cout << string(70, '=') << endl;
}

bool isGUIMode(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
        if (arg == "--gui" || arg == "-g") {
            return true;
        }
    }
    return false;
}

void showUsage() {
    cout << "Usage: FileXplore [options] [vfs_root_directory]" << endl;
    cout << "Options:" << endl;
    cout << "  --gui, -g        Start in GUI mode (web interface)" << endl;
    cout << "  --help, -h       Show this help message" << endl;
    cout << endl;
    cout << "Examples:" << endl;
    cout << "  FileXplore                    # Start CLI mode with default VFS root" << endl;
    cout << "  FileXplore /tmp/myfs          # Start CLI mode with custom VFS root" << endl;
    cout << "  FileXplore --gui              # Start GUI mode with default VFS root" << endl;
    cout << "  FileXplore --gui /tmp/myfs    # Start GUI mode with custom VFS root" << endl;
}

void displayPrompt() {
    string current_dir = PathUtils::getCurrentVirtualPath();
    cout << "FileXplore:" << current_dir << "$ ";
}

int main(int argc, char* argv[]) {
    // Check for help argument
    for (int i = 1; i < argc; ++i) {
        string arg(argv[i]);
        transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
        if (arg == "--help" || arg == "-h") {
            showUsage();
            return 0;
        }
    }

    // Determine if GUI mode
    bool gui_mode = isGUIMode(argc, argv);

    // Parse VFS root directory (skip GUI flag)
    string vfs_root = "./filexplore_root";
    for (int i = 1; i < argc; ++i) {
        string arg(argv[i]);
        transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
        if (arg != "--gui" && arg != "-g") {
            vfs_root = argv[i];
            break;
        }
    }

    // Initialize the virtual file system
    if (!PathUtils::initializeVFSRoot(vfs_root)) {
        cerr << "Error: Failed to initialize VFS root directory: " << vfs_root << endl;
        cerr << "Please check permissions and try again." << endl;
        return 1;
    }

    // Initialize persistence system
    if (PersistenceManager::initialize(vfs_root)) {
        if (!gui_mode) {
            cout << "Persistence system initialized." << endl;

            // Load previous state if available
            if (PathUtils::loadVFSState()) {
                cout << "Previous VFS state restored." << endl;
            }

            if (HistoryManager::loadHistory()) {
                cout << "Command history restored." << endl;
            }
        }
    } else if (!gui_mode) {
        cout << "Warning: Persistence system not available. Session data will not be saved." << endl;
    }

    // Initialize command parser
    CommandParser::initialize();

    // GUI Mode
    if (gui_mode) {
        displayGUIWelcome();
#if ENABLE_GUI
        // Start embedded web server
        int port = 8080;
        WebServer server(port);
        if (!server.start()) {
            cerr << "Error: Failed to start web server on port " << port << endl;
            return 1;
        }

        cout << "Web server started on http://localhost:" << port << "/" << endl;
        cout << "Serving static files from ./web. Press Ctrl+C to stop." << endl;

        // Keep process alive while server runs
        while (server.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        server.stop();
        return 0;
#else
        cerr << "GUI is disabled in this build. Rebuild with a newer compiler (e.g., MSYS2 MinGW-w64 GCC >= 9) or MSVC to enable GUI." << endl;
        return 1;
#endif
    }

    // CLI Mode (original functionality)
    displayWelcome();

    // Show initial system information
    cout << "VFS Root: " << PathUtils::getVFSRoot() << endl;
    cout << "Current Directory: " << PathUtils::getCurrentVirtualPath() << endl;
    cout << string(70, '-') << endl;

    // Main CLI loop
    string input;
    bool running = true;

    while (running) {
        displayPrompt();

        // Get user input
        if (!getline(cin, input)) {
            // Handle EOF (Ctrl+D on Unix, Ctrl+Z on Windows)
            cout << endl << "Goodbye! Exiting FileXplore..." << endl;
            break;
        }

        // Skip empty input
        if (input.empty()) {
            continue;
        }

        // Execute command
        CommandParser::CommandResult result = CommandParser::executeCommand(input);

        // Handle command result
        if (!result.success) {
            cerr << "Error: " << result.message << endl;
        } else if (!result.message.empty()) {
            if (result.message == "EXIT") {
                running = false;
            } else {
                cout << result.message << endl;
            }
        }

        // Add a blank line for better readability (except for certain commands)
        if (running && input != "clear" && input != "help" && input != "history" && input != "df") {
            cout << endl;
        }
    }

    // Save state before exiting
    if (PersistenceManager::isPersistenceAvailable()) {
        cout << "Saving session data..." << endl;

        if (PathUtils::saveVFSState()) {
            cout << "VFS state saved." << endl;
        }

        if (HistoryManager::saveHistory()) {
            cout << "Command history saved." << endl;
        }
    }

    return 0;
}