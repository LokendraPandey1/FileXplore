#ifndef PATHUTILS_H
#define PATHUTILS_H

#include <string>
#include <vector>

using std::string;
using std::vector;

/**
 * PathUtils class for safe path resolution and sandbox security
 * Compatible with older compilers that don't support filesystem
 * Supports persistence through PersistenceManager
 */
class PathUtils {
private:
    static string vfs_root;
    static string current_virtual_path;

public:
    // Initialize the VFS root directory
    static bool initializeVFSRoot(const string& root_path);
    
    // Convert virtual path to real filesystem path
    static string virtualToRealPath(const string& virtual_path);
    
    // Ensure path is within sandbox (security check)
    static bool isPathSafe(const string& real_path);
    
    // Resolve and normalize path (handle .., ., etc.)
    static string resolvePath(const string& path);
    
    // Normalize path separators and remove redundant components
    static string normalizePath(const string& path);
    
    // Get current virtual directory
    static string getCurrentVirtualPath();
    
    // Set current virtual directory
    static bool setCurrentVirtualPath(const string& path);
    
    // Check if path exists
    static bool pathExists(const string& path);
    
    // Check if path is a directory
    static bool isDirectory(const string& path);
    
    // Check if path is a regular file
    static bool isFile(const string& path);
    
    // Get VFS root path
    static string getVFSRoot();
    
    // Split path into components
    static vector<string> splitPath(const string& path);
    
    // Join path components
    static string joinPath(const vector<string>& components);
    
    // Get parent directory
    static string getParentPath(const string& path);
    
    // Get filename from path
    static string getFilename(const string& path);
    
    // Persistence methods
    static bool saveVFSState();
    static bool loadVFSState();

    // Resolve a virtual path to a real filesystem path (wrapper convenience)
    static string resolveVirtualPath(const string& path);

    // Convert a real filesystem path under VFS root back to virtual path
    static string getVirtualPath(const string& real_path);
};

#endif // PATHUTILS_H