#include "include/PathUtils.h"
#include "include/CommandParser.h"
#include "include/SystemInfo.h"
#include "include/PersistenceManager.h"
#include "include/HistoryManager.h"
#include <iostream>
#include <string>

using namespace std;

void displayWelcome() {
    cout << string(70, '=') << endl;
    cout << "Welcome to FileXplore - Virtual File System Simulator" << endl;
    cout << "Version 1.0 - C++17 Implementation" << endl;
    cout << string(70, '=') << endl;
    cout << "Type 'help' for available commands or 'exit' to quit." << endl;
    cout << string(70, '-') << endl;
}

void displayPrompt() {
    string current_dir = PathUtils::getCurrentVirtualPath();
    cout << "FileXplore:" << current_dir << "$ ";
}

int main(int argc, char* argv[]) {
    // Default VFS root directory
    string vfs_root = "./filexplore_root";
    
    // Allow custom VFS root from command line
    if (argc > 1) {
        vfs_root = argv[1];
    }
    
    // Initialize the virtual file system
    if (!PathUtils::initializeVFSRoot(vfs_root)) {
        cerr << "Error: Failed to initialize VFS root directory: " << vfs_root << endl;
        cerr << "Please check permissions and try again." << endl;
        return 1;
    }
    
    // Initialize persistence system
    if (PersistenceManager::initialize(vfs_root)) {
        cout << "Persistence system initialized." << endl;
        
        // Load previous state if available
        if (PathUtils::loadVFSState()) {
            cout << "Previous VFS state restored." << endl;
        }
        
        if (HistoryManager::loadHistory()) {
            cout << "Command history restored." << endl;
        }
    } else {
        cout << "Warning: Persistence system not available. Session data will not be saved." << endl;
    }
    
    // Initialize command parser
    CommandParser::initialize();
    
    // Display welcome message
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