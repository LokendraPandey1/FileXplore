using namespace std;
#include "../include/DirManager.h"
#include "../include/PathUtils.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

bool DirManager::createDirectory(const string& virtual_path) {
    if (!validateDirectoryOperation(virtual_path, false)) {
        return false;
    }
    
    string real_path = PathUtils::virtualToRealPath(virtual_path);
    if (real_path.empty()) {
        cerr << "Error: Invalid path: " << virtual_path << endl;
        return false;
    }
    
    // Check if directory already exists (use virtual path)
    if (PathUtils::pathExists(virtual_path)) {
        cerr << "Error: Directory already exists: " << virtual_path << endl;
        return false;
    }
    
    // Create directory
#ifdef _WIN32
    if (_mkdir(real_path.c_str()) == 0) {
        return true;
    } else {
        cerr << "Error creating directory: " << virtual_path << endl;
        return false;
    }
#else
    if (mkdir(real_path.c_str(), 0755) == 0) {
        return true;
    } else {
        cerr << "Error creating directory: " << virtual_path << endl;
        return false;
    }
#endif
}

bool DirManager::removeDirectory(const string& virtual_path) {
    if (!validateDirectoryOperation(virtual_path, true)) {
        return false;
    }
    
    string real_path = PathUtils::virtualToRealPath(virtual_path);
    if (real_path.empty()) {
        cerr << "Error: Invalid path: " << virtual_path << endl;
        return false;
    }
    
    // Check if directory exists (use virtual path)
    if (!PathUtils::pathExists(virtual_path)) {
        cerr << "Error: Directory does not exist: " << virtual_path << endl;
        return false;
    }
    
    // Check if it's actually a directory (use virtual path)
    if (!PathUtils::isDirectory(virtual_path)) {
        cerr << "Error: Path is not a directory: " << virtual_path << endl;
        return false;
    }
    
    // Check if directory is empty
    if (!isDirectoryEmpty(virtual_path)) {
        cerr << "Error: Directory is not empty: " << virtual_path << endl;
        return false;
    }
    
    // Remove directory
#ifdef _WIN32
    if (_rmdir(real_path.c_str()) == 0) {
        return true;
    } else {
        cerr << "Error removing directory: " << virtual_path << endl;
        return false;
    }
#else
    if (rmdir(real_path.c_str()) == 0) {
        return true;
    } else {
        cerr << "Error removing directory: " << virtual_path << endl;
        return false;
    }
#endif
}

vector<string> DirManager::listDirectory(const string& virtual_path) {
    vector<string> entries;
    
    string real_path = PathUtils::virtualToRealPath(virtual_path);
    cout << "DEBUG listDirectory: Virtual: " << virtual_path << ", Real: " << real_path << endl;
    
    if (real_path.empty()) {
        cout << "DEBUG listDirectory: Real path is empty" << endl;
        return entries;
    }
    
    if (!PathUtils::pathExists(virtual_path)) {
        cout << "DEBUG listDirectory: Path does not exist" << endl;
        return entries;
    }
    
    if (!PathUtils::isDirectory(virtual_path)) {
        cout << "DEBUG listDirectory: Path is not a directory" << endl;
        return entries;
    }
    
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    string searchPath = real_path + "\\*";
    cout << "DEBUG listDirectory: Search path: " << searchPath << endl;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            string filename = findFileData.cFileName;
            if (filename != "." && filename != "..") {
                entries.push_back(filename);
                cout << "DEBUG listDirectory: Found file: " << filename << endl;
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    } else {
        cout << "DEBUG listDirectory: FindFirstFile failed" << endl;
    }
#else
    DIR* dir = opendir(real_path.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            string filename = entry->d_name;
            if (filename != "." && filename != "..") {
                entries.push_back(filename);
            }
        }
        closedir(dir);
    }
#endif
    
    sort(entries.begin(), entries.end());
    cout << "DEBUG listDirectory: Total entries found: " << entries.size() << endl;
    return entries;
}

void DirManager::displayTree(const string& virtual_path, int depth) {
    if (!validateDirectoryOperation(virtual_path, true)) {
        return;
    }
    
    string real_path = PathUtils::virtualToRealPath(virtual_path);
    if (real_path.empty()) {
        cerr << "Error: Invalid path: " << virtual_path << endl;
        return;
    }
    
    // Check if directory exists
    if (!PathUtils::pathExists(real_path)) {
        cerr << "Error: Directory does not exist: " << virtual_path << endl;
        return;
    }
    
    // Check if it's actually a directory
    if (!PathUtils::isDirectory(real_path)) {
        cerr << "Error: Path is not a directory: " << virtual_path << endl;
        return;
    }
    
    cout << virtual_path << endl;
    displayTreeRecursive(real_path, virtual_path, 0, "");
}

void DirManager::displayTreeRecursive(const string& real_path, const string& virtual_path, 
                                     int depth, const string& prefix) {
    if (depth > 10) { // Prevent infinite recursion
        return;
    }
    
    vector<string> entries = listDirectory(virtual_path);
    
    for (size_t i = 0; i < entries.size(); ++i) {
        const string& name = entries[i];
        bool is_last = (i == entries.size() - 1);
        
        string current_prefix = prefix + (is_last ? "└── " : "├── ");
        string next_prefix = prefix + (is_last ? "    " : "│   ");
        
        string full_virtual_path = virtual_path + "/" + name;
        string full_real_path = PathUtils::virtualToRealPath(full_virtual_path);
        
        cout << current_prefix << name;
        if (PathUtils::isDirectory(full_real_path)) {
            cout << "/";
            cout << endl;
            // Recursively display subdirectories
            displayTreeRecursive(full_real_path, full_virtual_path, depth + 1, next_prefix);
        } else {
            cout << endl;
        }
    }
}

bool DirManager::changeDirectory(const string& virtual_path) {
    if (!validateDirectoryOperation(virtual_path, true)) {
        return false;
    }
    
    // Use PathUtils to change directory (it handles validation)
    if (!PathUtils::setCurrentVirtualPath(virtual_path)) {
        cerr << "Error: Cannot change to directory: " << virtual_path << endl;
        return false;
    }
    
    return true;
}

string DirManager::getCurrentDirectory() {
    return PathUtils::getCurrentVirtualPath();
}

bool DirManager::directoryExists(const string& virtual_path) {
    string real_path = PathUtils::virtualToRealPath(virtual_path);
    return !real_path.empty() && PathUtils::pathExists(real_path) && PathUtils::isDirectory(real_path);
}

bool DirManager::isDirectoryEmpty(const string& virtual_path) {
    string real_path = PathUtils::virtualToRealPath(virtual_path);
    if (real_path.empty() || !PathUtils::pathExists(virtual_path) || !PathUtils::isDirectory(virtual_path)) {
        return false;
    }
    
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    string searchPath = real_path + "\\*";
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            string filename = findFileData.cFileName;
            if (filename != "." && filename != "..") {
                FindClose(hFind);
                return false;
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
    return true;
#else
    DIR* dir = opendir(real_path.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            string filename = entry->d_name;
            if (filename != "." && filename != "..") {
                closedir(dir);
                return false;
            }
        }
        closedir(dir);
    }
    return true;
#endif
}

bool DirManager::validateDirectoryOperation(const string& virtual_path, bool should_exist) {
    if (virtual_path.empty()) {
        cerr << "Error: Empty path provided" << endl;
        return false;
    }
    
    if (!PathUtils::isPathSafe(PathUtils::virtualToRealPath(virtual_path))) {
        cerr << "Error: Unsafe path (outside sandbox): " << virtual_path << endl;
        return false;
    }
    
    return true;
}