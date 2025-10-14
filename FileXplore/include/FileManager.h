using namespace std;
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
    static string createFile(const string& virtual_path);
    static string writeFile(const string& virtual_path, const string& content);
    static string appendFile(const string& virtual_path, const string& content);
    static string readFile(const string& virtual_path);
    static string deleteFile(const string& virtual_path);
    
    // File information
    static bool fileExists(const string& virtual_path);
    static long long getFileSize(const string& virtual_path);
    
private:
    // Helper method for validation
    static bool validateFileOperation(const string& real_path, const string& operation);
};