using namespace std;
#pragma once

#include <string>

/**
 * SystemInfo - Provides system information and statistics
 * Handles disk usage, file/folder counts, and other system utilities
 */
class SystemInfo {
public:
    // Structure to hold disk usage information
    struct DiskUsage {
        size_t total_files;
        size_t total_directories;
        size_t total_size_bytes;
        string formatted_size;
        
        DiskUsage() : total_files(0), total_directories(0), total_size_bytes(0) {}
    };
    
    // Get disk usage information for VFS root
    static DiskUsage getDiskUsage();
    
    // Display disk usage information (df command)
    static void displayDiskUsage();
    
    // Format bytes to human-readable format
    static string formatBytes(size_t bytes);
    
    // Get VFS information summary
    static string getVFSInfo();
    
private:
    // Recursively calculate directory size and counts
    static void calculateDirectoryStats(const string& path, DiskUsage& usage);
};