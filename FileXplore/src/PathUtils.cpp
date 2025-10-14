using namespace std;
#include "../include/PathUtils.h"
#include "../include/PersistenceManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>

#ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    #define PATH_SEPARATOR '\\'
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <unistd.h>
    #define PATH_SEPARATOR '/'
#endif

// Static member definitions
string PathUtils::vfs_root = "";
string PathUtils::current_virtual_path = "/";

bool PathUtils::initializeVFSRoot(const string& root_path) {
    try {
        // Convert to platform-specific path separators
        string platform_root = root_path;
        replace(platform_root.begin(), platform_root.end(), '/', PATH_SEPARATOR);
        
        // Check if directory exists
        struct stat info;
        if (stat(platform_root.c_str(), &info) != 0) {
            // Directory doesn't exist, try to create it
            if (mkdir(platform_root.c_str(), 0755) != 0) {
                cerr << "Error: Failed to create VFS root directory: " << platform_root << endl;
                return false;
            }
        } else if (!(info.st_mode & S_IFDIR)) {
            cerr << "Error: VFS root path is not a directory: " << platform_root << endl;
            return false;
        }
        
        // Store the original Unix-style path for internal use
        vfs_root = root_path;
        current_virtual_path = "/";
        
        cout << "VFS Root initialized: " << vfs_root << endl;
        return true;
    } catch (const exception& e) {
        cerr << "Error initializing VFS root: " << e.what() << endl;
        return false;
    }
}

string PathUtils::virtualToRealPath(const string& virtual_path) {
    if (vfs_root.empty()) {
        return "";
    }
    
    string resolved = resolvePath(virtual_path);
    
    // Start with the platform-specific VFS root
    string real_path = vfs_root;
    replace(real_path.begin(), real_path.end(), '/', PATH_SEPARATOR);
    
    // If the resolved path is not just the root, append the path
    if (resolved != "/" && !resolved.empty()) {
        if (real_path.back() != PATH_SEPARATOR) {
            real_path += PATH_SEPARATOR;
        }
        // Remove leading '/' from resolved path and convert to platform separators
        string path_part = resolved.substr(1);
        replace(path_part.begin(), path_part.end(), '/', PATH_SEPARATOR);
        real_path += path_part;
    }
    
    return real_path;
}

bool PathUtils::isPathSafe(const string& real_path) {
    if (vfs_root.empty()) {
        return false;
    }
    
    // Convert both paths to use consistent separators for comparison
    string normalized_real = real_path;
    string normalized_root = vfs_root;
    
    // Convert to platform-specific separators
    replace(normalized_real.begin(), normalized_real.end(), '/', PATH_SEPARATOR);
    replace(normalized_root.begin(), normalized_root.end(), '/', PATH_SEPARATOR);
    
#ifdef _WIN32
    // On Windows, use GetFullPathName to resolve the canonical path
    char resolved_real[MAX_PATH];
    char resolved_root[MAX_PATH];
    
    if (GetFullPathNameA(normalized_real.c_str(), MAX_PATH, resolved_real, nullptr) == 0) {
        return false;
    }
    if (GetFullPathNameA(normalized_root.c_str(), MAX_PATH, resolved_root, nullptr) == 0) {
        return false;
    }
    
    string canonical_real(resolved_real);
    string canonical_root(resolved_root);
#else
    // On Unix-like systems, use realpath
    char* resolved_real = realpath(normalized_real.c_str(), nullptr);
    char* resolved_root = realpath(normalized_root.c_str(), nullptr);
    
    if (!resolved_real || !resolved_root) {
        if (resolved_real) free(resolved_real);
        if (resolved_root) free(resolved_root);
        return false;
    }
    
    string canonical_real(resolved_real);
    string canonical_root(resolved_root);
    
    free(resolved_real);
    free(resolved_root);
#endif
    
    // Ensure the canonical real path starts with the canonical VFS root
    // and add a separator check to prevent partial matches
    if (canonical_real.length() < canonical_root.length()) {
        return false;
    }
    
    if (canonical_real.substr(0, canonical_root.length()) != canonical_root) {
        return false;
    }
    
    // If paths are exactly equal, it's safe
    if (canonical_real.length() == canonical_root.length()) {
        return true;
    }
    
    // Check that the character after the root is a path separator
    char next_char = canonical_real[canonical_root.length()];
    return (next_char == PATH_SEPARATOR);
}

string PathUtils::resolvePath(const string& path) {
    if (path.empty()) {
        return current_virtual_path;
    }
    
    string working_path;
    if (path[0] == '/') {
        working_path = path; // Absolute path
    } else {
        // Relative path - combine with current directory
        working_path = current_virtual_path;
        if (working_path.back() != '/') {
            working_path += '/';
        }
        working_path += path;
    }
    
    return normalizePath(working_path);
}

string PathUtils::normalizePath(const string& path) {
    if (path.empty()) {
        return "/";
    }
    
    vector<string> components = splitPath(path);
    vector<string> normalized;
    
    for (const auto& component : components) {
        if (component == "." || component.empty()) {
            continue; // Skip current directory and empty components
        } else if (component == "..") {
            if (!normalized.empty() && normalized.back() != "..") {
                normalized.pop_back(); // Go up one directory
            }
        } else {
            normalized.push_back(component);
        }
    }
    
    string result = "/";
    for (size_t i = 0; i < normalized.size(); ++i) {
        if (i > 0) result += "/";
        result += normalized[i];
    }
    
    return result;
}

string PathUtils::getCurrentVirtualPath() {
    return current_virtual_path;
}

bool PathUtils::setCurrentVirtualPath(const string& path) {
    string resolved = resolvePath(path);
    string real_path = virtualToRealPath(resolved);
    
    if (!isPathSafe(real_path)) {
        return false;
    }
    
    if (isDirectory(resolved)) {
        current_virtual_path = resolved;
        return true;
    }
    
    return false;
}

bool PathUtils::pathExists(const string& virtual_path) {
    string real_path = virtualToRealPath(virtual_path);
    cout << "DEBUG pathExists: Virtual: " << virtual_path << ", Real: " << real_path << endl;
    
    if (!isPathSafe(real_path)) {
        cout << "DEBUG pathExists: Path not safe" << endl;
        return false;
    }
    
    struct stat info;
    int result = stat(real_path.c_str(), &info);
    cout << "DEBUG pathExists: stat result: " << result << endl;
    return result == 0;
}

bool PathUtils::isDirectory(const string& virtual_path) {
    string real_path = virtualToRealPath(virtual_path);
    if (!isPathSafe(real_path)) {
        return false;
    }
    
    struct stat info;
    if (stat(real_path.c_str(), &info) != 0) {
        return false;
    }
    
    return (info.st_mode & S_IFDIR) != 0;
}

bool PathUtils::isFile(const string& virtual_path) {
    string real_path = virtualToRealPath(virtual_path);
    if (!isPathSafe(real_path)) {
        return false;
    }
    
    struct stat info;
    if (stat(real_path.c_str(), &info) != 0) {
        return false;
    }
    
    return (info.st_mode & S_IFREG) != 0;
}

string PathUtils::getVFSRoot() {
    return vfs_root;
}

vector<string> PathUtils::splitPath(const string& path) {
    vector<string> components;
    stringstream ss(path);
    string component;
    
    while (getline(ss, component, '/')) {
        if (!component.empty()) {
            components.push_back(component);
        }
    }
    
    return components;
}

string PathUtils::joinPath(const vector<string>& components) {
    if (components.empty()) {
        return "/";
    }
    
    string result = "/";
    for (size_t i = 0; i < components.size(); ++i) {
        if (i > 0) result += "/";
        result += components[i];
    }
    
    return result;
}

string PathUtils::getParentPath(const string& path) {
    if (path == "/" || path.empty()) {
        return "/";
    }
    
    vector<string> components = splitPath(path);
    if (components.empty()) {
        return "/";
    }
    
    components.pop_back();
    return joinPath(components);
}

string PathUtils::getFilename(const string& path) {
    if (path == "/" || path.empty()) {
        return "";
    }
    
    vector<string> components = splitPath(path);
    if (components.empty()) {
        return "";
    }
    
    return components.back();
}

bool PathUtils::saveVFSState() {
    return PersistenceManager::saveVFSState(current_virtual_path, vfs_root);
}

bool PathUtils::loadVFSState() {
    map<string, string> state = PersistenceManager::loadVFSState();
    
    if (state.empty()) {
        return false;
    }
    
    // Load current directory if available
    if (state.find("current_directory") != state.end()) {
        string loaded_dir = state["current_directory"];
        if (!loaded_dir.empty()) {
            current_virtual_path = loaded_dir;
        }
    }
    
    // Load VFS root if available (for validation)
    if (state.find("vfs_root") != state.end()) {
        string loaded_root = state["vfs_root"];
        if (!loaded_root.empty() && loaded_root == vfs_root) {
            // VFS root matches, state is valid
            return true;
        }
    }
    
    return !state.empty();
}