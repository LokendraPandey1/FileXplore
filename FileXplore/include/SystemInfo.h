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
        std::size_t total_files;
        std::size_t total_directories;
        std::size_t total_size_bytes;
        std::string formatted_size;
        
        DiskUsage() : total_files(0), total_directories(0), total_size_bytes(0) {}
    };
    
    // Get disk usage information for VFS root
    static DiskUsage getDiskUsage();
    
    // Display disk usage information (df command)
    static void displayDiskUsage();
    
    // Format bytes to human-readable format
    static std::string formatBytes(std::size_t bytes);
    
    // Get VFS information summary
    static std::string getVFSInfo();
    
private:
    // Recursively calculate directory size and counts
    static void calculateDirectoryStats(const std::string& path, DiskUsage& usage);
};