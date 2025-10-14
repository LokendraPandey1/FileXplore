using namespace std;
#pragma once

#include <string>
#include <vector>

/**
 * DirManager - Handles all directory operations
 * Provides methods for creating, removing, listing, and navigating directories
 */
class DirManager {
public:
    // Create directory
    static bool createDirectory(const string& virtual_path);
    
    // Remove directory (must be empty)
    static bool removeDirectory(const string& virtual_path);
    
    // List directory contents
    static vector<string> listDirectory(const string& virtual_path);
    
    // Display directory tree structure
    static void displayTree(const string& virtual_path, int depth = 0);
    
    // Change current directory
    static bool changeDirectory(const string& virtual_path);
    
    // Get current working directory
    static string getCurrentDirectory();
    
    // Check if directory exists
    static bool directoryExists(const string& virtual_path);
    
    // Check if directory is empty
    static bool isDirectoryEmpty(const string& virtual_path);
    
private:
    // Helper method for recursive tree display
    static void displayTreeRecursive(const string& real_path, const string& virtual_path, 
                                   int depth, const string& prefix);
    
    // Helper method to validate directory operations
    static bool validateDirectoryOperation(const string& virtual_path, bool should_exist = true);
};