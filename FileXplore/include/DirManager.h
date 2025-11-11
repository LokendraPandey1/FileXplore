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
    static bool createDirectory(const std::string& virtual_path);
    
    // Remove directory (must be empty)
    static bool removeDirectory(const std::string& virtual_path);
    
    // List directory contents
    static std::vector<std::string> listDirectory(const std::string& virtual_path);
    
    // Display directory tree structure
    static void displayTree(const std::string& virtual_path, int depth = 0);
    
    // Change current directory
    static bool changeDirectory(const std::string& virtual_path);
    
    // Get current working directory
    static std::string getCurrentDirectory();
    
    // Check if directory exists
    static bool directoryExists(const std::string& virtual_path);
    
    // Check if directory is empty
    static bool isDirectoryEmpty(const std::string& virtual_path);
    
private:
    // Helper method for recursive tree display
    static void displayTreeRecursive(const std::string& real_path, const std::string& virtual_path, 
                                   int depth, const std::string& prefix);
    
    // Helper method to validate directory operations
    static bool validateDirectoryOperation(const std::string& virtual_path, bool should_exist = true);
};