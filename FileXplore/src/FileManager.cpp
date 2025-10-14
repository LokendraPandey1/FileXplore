using namespace std;
#include "../include/FileManager.h"
#include "../include/PathUtils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

string FileManager::createFile(const string& virtual_path) {
    string real_path = PathUtils::virtualToRealPath(virtual_path);
    
    // Debug output
    cout << "DEBUG: Creating file - Virtual: " << virtual_path << ", Real: " << real_path << endl;
    
    if (!validateFileOperation(real_path, "create")) {
        cout << "DEBUG: Validation failed for: " << real_path << endl;
        return "Error: Invalid file path or access denied";
    }
    
    // Check if file already exists
    cout << "DEBUG: Checking if file exists: " << virtual_path << endl;
    cout << "DEBUG: Real path for existence check: " << real_path << endl;
    if (PathUtils::pathExists(virtual_path)) {
        cout << "DEBUG: File exists check returned true" << endl;
        return "Error: File already exists: " + virtual_path;
    }
    cout << "DEBUG: File already exists check passed for: " << virtual_path << endl;
    
    // Create parent directories if they don't exist
    string parent_path = PathUtils::getParentPath(virtual_path);
    string real_parent = PathUtils::virtualToRealPath(parent_path);
    
    cout << "DEBUG: Parent path - Virtual: " << parent_path << ", Real: " << real_parent << endl;
    
    if (!PathUtils::pathExists(parent_path)) {
        cout << "DEBUG: Parent directory does not exist: " << parent_path << endl;
        return "Error: Parent directory does not exist: " + parent_path;
    }
    
    try {
        cout << "DEBUG: Attempting to create file at: " << real_path << endl;
        ofstream file(real_path);
        if (!file.is_open()) {
            cout << "DEBUG: Failed to open file for creation: " << real_path << endl;
            return "Error: Failed to create file: " + virtual_path;
        }
        file.close();
        cout << "DEBUG: File created successfully at: " << real_path << endl;
        return "File created: " + virtual_path;
    } catch (const exception& e) {
        cout << "DEBUG: Exception during file creation: " << e.what() << endl;
        return "Error creating file: " + string(e.what());
    }
}

string FileManager::writeFile(const string& virtual_path, const string& content) {
    string real_path = PathUtils::virtualToRealPath(virtual_path);
    
    if (!validateFileOperation(real_path, "write")) {
        return "Error: Invalid file path or access denied";
    }
    
    try {
        ofstream file(real_path, ios::trunc);
        if (!file.is_open()) {
            return "Error: Cannot open file for writing: " + virtual_path;
        }
        
        file << content;
        file.close();
        
        if (file.fail()) {
            return "Error: Failed to write to file: " + virtual_path;
        }
        
        return "Content written to file: " + virtual_path;
    } catch (const exception& e) {
        return "Error writing file: " + string(e.what());
    }
}

string FileManager::appendFile(const string& virtual_path, const string& content) {
    string real_path = PathUtils::virtualToRealPath(virtual_path);
    
    if (!validateFileOperation(real_path, "append")) {
        return "Error: Invalid file path or access denied";
    }
    
    try {
        ofstream file(real_path, ios::app);
        if (!file.is_open()) {
            return "Error: Cannot open file for appending: " + virtual_path;
        }
        
        file << content;
        file.close();
        
        if (file.fail()) {
            return "Error: Failed to append to file: " + virtual_path;
        }
        
        return "Content appended to file: " + virtual_path;
    } catch (const exception& e) {
        return "Error appending to file: " + string(e.what());
    }
}

string FileManager::readFile(const string& virtual_path) {
    string real_path = PathUtils::virtualToRealPath(virtual_path);
    
    if (!validateFileOperation(real_path, "read")) {
        return "Error: Invalid file path or access denied";
    }
    
    if (!PathUtils::pathExists(virtual_path)) {
        return "Error: File does not exist: " + virtual_path;
    }
    
    if (!PathUtils::isFile(virtual_path)) {
        return "Error: Path is not a file: " + virtual_path;
    }
    
    try {
        ifstream file(real_path);
        if (!file.is_open()) {
            return "Error: Cannot open file for reading: " + virtual_path;
        }
        
        stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        if (file.fail() && !file.eof()) {
            return "Error: Failed to read file: " + virtual_path;
        }
        
        return buffer.str();
    } catch (const exception& e) {
        return "Error reading file: " + string(e.what());
    }
}

string FileManager::deleteFile(const string& virtual_path) {
    string real_path = PathUtils::virtualToRealPath(virtual_path);
    
    if (!validateFileOperation(real_path, "delete")) {
        return "Error: Invalid file path or access denied";
    }
    
    if (!PathUtils::pathExists(virtual_path)) {
        return "Error: File does not exist: " + virtual_path;
    }
    
    if (!PathUtils::isFile(virtual_path)) {
        return "Error: Path is not a file: " + virtual_path;
    }
    
    try {
        if (remove(real_path.c_str()) != 0) {
            return "Error: Failed to delete file: " + virtual_path;
        }
        
        return "File deleted: " + virtual_path;
    } catch (const exception& e) {
        return "Error deleting file: " + string(e.what());
    }
}

bool FileManager::fileExists(const string& virtual_path) {
    return PathUtils::pathExists(virtual_path) && PathUtils::isFile(virtual_path);
}

long long FileManager::getFileSize(const string& virtual_path) {
    string real_path = PathUtils::virtualToRealPath(virtual_path);
    
    if (!PathUtils::isPathSafe(real_path) || !PathUtils::isFile(virtual_path)) {
        return -1;
    }
    
    try {
        ifstream file(real_path, ios::binary | ios::ate);
        if (!file.is_open()) {
            return -1;
        }
        
        long long size = file.tellg();
        file.close();
        return size;
    } catch (const exception&) {
        return -1;
    }
}

bool FileManager::validateFileOperation(const string& real_path, const string& operation) {
    if (real_path.empty()) {
        return false;
    }
    
    if (!PathUtils::isPathSafe(real_path)) {
        return false;
    }
    
    // Additional validation can be added here based on operation type
    return true;
}