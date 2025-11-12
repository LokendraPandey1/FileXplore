#pragma once

#include <string>
#include <vector>

/**
 * CompressionManager - Handles file compression and decompression
 * Supports ZIP format for archiving files and directories
 */
class CompressionManager {
public:
    // Compress files/directories to a zip file
    static bool compressToZip(const std::string& zipPath, const std::vector<std::string>& paths);
    
    // Decompress a zip file to a destination directory
    static bool decompressFromZip(const std::string& zipPath, const std::string& destDir);
    
    // Check if a file is a zip archive
    static bool isZipFile(const std::string& path);
    
    // List contents of a zip file
    static std::vector<std::string> listZipContents(const std::string& zipPath);
};

