#pragma once

#include <string>
#include <vector>

/**
 * FileManager - Handles all file operations
 * Provides methods for creating, reading, writing, appending, and deleting files
 */
class FileManager {
public:
    // File operations
    static std::string createFile(const std::string& virtual_path);
    static std::string writeFile(const std::string& virtual_path, const std::string& content);
    static std::string appendFile(const std::string& virtual_path, const std::string& content);
    static std::string readFile(const std::string& virtual_path);
    static std::string deleteFile(const std::string& virtual_path);
    
    // File information
    static bool fileExists(const std::string& virtual_path);
    static long long getFileSize(const std::string& virtual_path);
    
private:
    // Helper method for validation
    static bool validateFileOperation(const std::string& real_path, const std::string& operation);
};