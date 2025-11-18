#include "../include/CompressionManager.h"
#include "../include/PathUtils.h"
#include "../include/FileManager.h"
#include "../include/DirManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <cstdint>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define PATH_SEPARATOR '\\'
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #define PATH_SEPARATOR '/'
#endif

// Try to use minizip if available
#ifdef HAVE_MINIZIP
    #include <zip.h>
    #include <unzip.h>
    #define USE_MINIZIP 1
#else
    // Fallback: Simple zip implementation using zlib
    #include <zlib.h>
    #define USE_MINIZIP 0
#endif

using namespace std;
namespace fs = std::filesystem;

// Simple ZIP file structure (without minizip)
struct ZipLocalFileHeader {
    uint32_t signature;      // 0x04034b50
    uint16_t version;
    uint16_t flags;
    uint16_t compression;
    uint16_t modTime;
    uint16_t modDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t filenameLength;
    uint16_t extraFieldLength;
};

struct ZipCentralDirHeader {
    uint32_t signature;      // 0x02014b50
    uint16_t versionMadeBy;
    uint16_t versionNeeded;
    uint16_t flags;
    uint16_t compression;
    uint16_t modTime;
    uint16_t modDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t filenameLength;
    uint16_t extraFieldLength;
    uint16_t commentLength;
    uint16_t diskNumber;
    uint16_t internalAttribs;
    uint32_t externalAttribs;
    uint32_t localHeaderOffset;
};

static void writeUint16(ofstream& file, uint16_t value) {
    file.write(reinterpret_cast<const char*>(&value), 2);
}

static void writeUint32(ofstream& file, uint32_t value) {
    file.write(reinterpret_cast<const char*>(&value), 4);
}

static uint16_t readUint16(ifstream& file) {
    uint16_t value;
    file.read(reinterpret_cast<char*>(&value), 2);
    return value;
}

static uint32_t readUint32(ifstream& file) {
    uint32_t value;
    file.read(reinterpret_cast<char*>(&value), 4);
    return value;
}

static uint32_t calculateCRC32(const string& data) {
    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, reinterpret_cast<const Bytef*>(data.c_str()), static_cast<uInt>(data.length()));
    return static_cast<uint32_t>(crc);
}

static string compressData(const string& data) {
    if (data.empty()) {
        return "";
    }
    
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    
    if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return "";
    }
    
    zs.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data.c_str()));
    zs.avail_in = static_cast<uInt>(data.length());
    
    string compressed;
    char buffer[16384];
    
    do {
        zs.next_out = reinterpret_cast<Bytef*>(buffer);
        zs.avail_out = sizeof(buffer);
        
        int ret = deflate(&zs, Z_FINISH);
        if (ret == Z_STREAM_ERROR) {
            deflateEnd(&zs);
            return "";
        }
        
        compressed.append(buffer, sizeof(buffer) - zs.avail_out);
    } while (zs.avail_out == 0);
    
    deflateEnd(&zs);
    return compressed;
}

static string decompressData(const string& compressed, size_t uncompressedSize) {
    if (compressed.empty() || uncompressedSize == 0) {
        return "";
    }
    
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    
    if (inflateInit2(&zs, -MAX_WBITS) != Z_OK) {
        return "";
    }
    
    zs.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(compressed.c_str()));
    zs.avail_in = static_cast<uInt>(compressed.length());
    
    string decompressed;
    decompressed.resize(uncompressedSize);
    zs.next_out = reinterpret_cast<Bytef*>(&decompressed[0]);
    zs.avail_out = static_cast<uInt>(uncompressedSize);
    
    int ret = inflate(&zs, Z_FINISH);
    inflateEnd(&zs);
    
    if (ret != Z_STREAM_END) {
        return "";
    }
    
    return decompressed;
}

bool CompressionManager::compressToZip(const string& zipPath, const vector<string>& paths) {
    string realZipPath = PathUtils::virtualToRealPath(zipPath);
    
    if (!PathUtils::isPathSafe(realZipPath)) {
        cerr << "Error: Invalid zip file path" << endl;
        return false;
    }
    
    // Create parent directory if needed
    fs::path zipParent = fs::path(realZipPath).parent_path();
    if (!zipParent.empty() && !fs::exists(zipParent)) {
        fs::create_directories(zipParent);
    }
    
    ofstream zipFile(realZipPath, ios::binary);
    if (!zipFile.is_open()) {
        cerr << "Error: Cannot create zip file: " << zipPath << endl;
        return false;
    }
    
    vector<pair<string, uint32_t>> centralDir;  // filename, local header offset
    
    for (const auto& path : paths) {
        string realPath = PathUtils::virtualToRealPath(path);
        if (!PathUtils::isPathSafe(realPath)) {
            cerr << "Warning: Skipping unsafe path: " << path << endl;
            continue;
        }
        
        if (fs::is_directory(realPath)) {
            // Add directory recursively
            try {
                for (const auto& entry : fs::recursive_directory_iterator(realPath)) {
                    if (entry.is_regular_file()) {
                        string entryPath = entry.path().string();
                        string relativePath = fs::relative(entryPath, PathUtils::getVFSRoot()).string();
                        
                        // Convert to forward slashes for zip
                        replace(relativePath.begin(), relativePath.end(), PATH_SEPARATOR, '/');
                        if (relativePath[0] != '/') {
                            relativePath = "/" + relativePath;
                        }
                        
                        ifstream file(entryPath, ios::binary);
                        if (!file.is_open()) continue;
                        
                        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
                        file.close();
                        
                        // Write local file header
                        uint32_t headerOffset = zipFile.tellp();
                        ZipLocalFileHeader header;
                        header.signature = 0x04034b50;
                        header.version = 20;
                        header.flags = 0;
                        header.compression = 8;  // DEFLATE
                        header.modTime = 0;
                        header.modDate = 0;
                        header.crc32 = calculateCRC32(content);
                        
                        string compressed = compressData(content);
                        header.compressedSize = compressed.length();
                        header.uncompressedSize = content.length();
                        header.filenameLength = relativePath.length();
                        header.extraFieldLength = 0;
                        
                        writeUint32(zipFile, header.signature);
                        writeUint16(zipFile, header.version);
                        writeUint16(zipFile, header.flags);
                        writeUint16(zipFile, header.compression);
                        writeUint16(zipFile, header.modTime);
                        writeUint16(zipFile, header.modDate);
                        writeUint32(zipFile, header.crc32);
                        writeUint32(zipFile, header.compressedSize);
                        writeUint32(zipFile, header.uncompressedSize);
                        writeUint16(zipFile, header.filenameLength);
                        writeUint16(zipFile, header.extraFieldLength);
                        zipFile.write(relativePath.c_str(), relativePath.length());
                        zipFile.write(compressed.c_str(), compressed.length());
                        
                        centralDir.push_back({relativePath, headerOffset});
                    }
                }
            } catch (const exception& e) {
                cerr << "Error processing directory: " << e.what() << endl;
            }
        } else if (fs::is_regular_file(realPath)) {
            // Add single file
            ifstream file(realPath, ios::binary);
            if (!file.is_open()) {
                cerr << "Warning: Cannot open file: " << path << endl;
                continue;
            }
            
            string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            file.close();
            
            string relativePath = path;
            if (relativePath[0] != '/') {
                relativePath = "/" + relativePath;
            }
            
            // Write local file header
            uint32_t headerOffset = zipFile.tellp();
            ZipLocalFileHeader header;
            header.signature = 0x04034b50;
            header.version = 20;
            header.flags = 0;
            header.compression = 8;  // DEFLATE
            header.modTime = 0;
            header.modDate = 0;
            header.crc32 = calculateCRC32(content);
            
            string compressed = compressData(content);
            header.compressedSize = compressed.length();
            header.uncompressedSize = content.length();
            header.filenameLength = relativePath.length();
            header.extraFieldLength = 0;
            
            writeUint32(zipFile, header.signature);
            writeUint16(zipFile, header.version);
            writeUint16(zipFile, header.flags);
            writeUint16(zipFile, header.compression);
            writeUint16(zipFile, header.modTime);
            writeUint16(zipFile, header.modDate);
            writeUint32(zipFile, header.crc32);
            writeUint32(zipFile, header.compressedSize);
            writeUint32(zipFile, header.uncompressedSize);
            writeUint16(zipFile, header.filenameLength);
            writeUint16(zipFile, header.extraFieldLength);
            zipFile.write(relativePath.c_str(), relativePath.length());
            zipFile.write(compressed.c_str(), compressed.length());
            
            centralDir.push_back({relativePath, headerOffset});
        }
    }
    
    // Write central directory
    streampos centralDirPos = zipFile.tellp();
    uint32_t centralDirOffset = static_cast<uint32_t>(centralDirPos);
    for (const auto& entry : centralDir) {
        ZipCentralDirHeader cdHeader;
        cdHeader.signature = 0x02014b50;
        cdHeader.versionMadeBy = 20;
        cdHeader.versionNeeded = 20;
        cdHeader.flags = 0;
        cdHeader.compression = 8;
        cdHeader.modTime = 0;
        cdHeader.modDate = 0;
        cdHeader.crc32 = 0;  // Will be filled from local header
        cdHeader.compressedSize = 0;
        cdHeader.uncompressedSize = 0;
        cdHeader.filenameLength = entry.first.length();
        cdHeader.extraFieldLength = 0;
        cdHeader.commentLength = 0;
        cdHeader.diskNumber = 0;
        cdHeader.internalAttribs = 0;
        cdHeader.externalAttribs = 0;
        cdHeader.localHeaderOffset = entry.second;
        
        writeUint32(zipFile, cdHeader.signature);
        writeUint16(zipFile, cdHeader.versionMadeBy);
        writeUint16(zipFile, cdHeader.versionNeeded);
        writeUint16(zipFile, cdHeader.flags);
        writeUint16(zipFile, cdHeader.compression);
        writeUint16(zipFile, cdHeader.modTime);
        writeUint16(zipFile, cdHeader.modDate);
        writeUint32(zipFile, cdHeader.crc32);
        writeUint32(zipFile, cdHeader.compressedSize);
        writeUint32(zipFile, cdHeader.uncompressedSize);
        writeUint16(zipFile, cdHeader.filenameLength);
        writeUint16(zipFile, cdHeader.extraFieldLength);
        writeUint16(zipFile, cdHeader.commentLength);
        writeUint16(zipFile, cdHeader.diskNumber);
        writeUint16(zipFile, cdHeader.internalAttribs);
        writeUint32(zipFile, cdHeader.externalAttribs);
        writeUint32(zipFile, cdHeader.localHeaderOffset);
        zipFile.write(entry.first.c_str(), entry.first.length());
    }
    
    streampos endPos = zipFile.tellp();
    uint32_t centralDirSize = static_cast<uint32_t>(endPos) - centralDirOffset;
    
    // Write end of central directory record
    writeUint32(zipFile, 0x06054b50);  // EOCD signature
    writeUint16(zipFile, 0);  // disk number
    writeUint16(zipFile, 0);  // disk with central dir
    writeUint16(zipFile, centralDir.size());  // entries in this disk
    writeUint16(zipFile, centralDir.size());  // total entries
    writeUint32(zipFile, centralDirSize);  // central dir size
    writeUint32(zipFile, centralDirOffset);  // central dir offset
    writeUint16(zipFile, 0);  // comment length
    
    zipFile.close();
    return true;
}

bool CompressionManager::decompressFromZip(const string& zipPath, const string& destDir) {
    string realZipPath = PathUtils::virtualToRealPath(zipPath);
    string realDestDir = PathUtils::virtualToRealPath(destDir);
    
    if (!PathUtils::isPathSafe(realZipPath) || !PathUtils::isPathSafe(realDestDir)) {
        cerr << "Error: Invalid paths" << endl;
        return false;
    }
    
    ifstream zipFile(realZipPath, ios::binary);
    if (!zipFile.is_open()) {
        cerr << "Error: Cannot open zip file: " << zipPath << endl;
        return false;
    }
    
    // Create destination directory
    if (!fs::exists(realDestDir)) {
        fs::create_directories(realDestDir);
    }
    
    // Find end of central directory
    zipFile.seekg(0, ios::end);
    size_t fileSize = zipFile.tellg();
    
    // Search backwards for EOCD signature
    bool found = false;
    uint32_t centralDirOffset = 0;
    uint16_t numEntries = 0;
    
    for (size_t i = fileSize - 22; i > 0 && i < fileSize; --i) {
        zipFile.seekg(i);
        uint32_t sig = readUint32(zipFile);
        if (sig == 0x06054b50) {
            readUint16(zipFile);  // disk number
            readUint16(zipFile);  // disk with central dir
            numEntries = readUint16(zipFile);
            readUint16(zipFile);  // total entries
            readUint32(zipFile);  // central dir size
            centralDirOffset = readUint32(zipFile);
            found = true;
            break;
        }
    }
    
    if (!found) {
        cerr << "Error: Invalid zip file format" << endl;
        return false;
    }
    
    // Read central directory
    zipFile.seekg(centralDirOffset);
    for (uint16_t i = 0; i < numEntries; ++i) {
        uint32_t sig = readUint32(zipFile);
        if (sig != 0x02014b50) break;
        
        readUint16(zipFile);  // version made by
        readUint16(zipFile);  // version needed
        readUint16(zipFile);  // flags
        readUint16(zipFile);  // compression
        readUint16(zipFile);  // mod time
        readUint16(zipFile);  // mod date
        readUint32(zipFile);  // crc32 (not used in extraction)
        uint32_t compressedSize = readUint32(zipFile);
        uint32_t uncompressedSize = readUint32(zipFile);
        uint16_t filenameLength = readUint16(zipFile);
        uint16_t extraFieldLength = readUint16(zipFile);
        uint16_t commentLength = readUint16(zipFile);
        readUint16(zipFile);  // disk number
        readUint16(zipFile);  // internal attribs
        readUint32(zipFile);  // external attribs
        uint32_t localHeaderOffset = readUint32(zipFile);
        
        string filename(filenameLength, '\0');
        zipFile.read(&filename[0], filenameLength);
        zipFile.seekg(extraFieldLength + commentLength, ios::cur);
        
        // Read local file header
        zipFile.seekg(localHeaderOffset);
        uint32_t localSig = readUint32(zipFile);
        if (localSig != 0x04034b50) continue;
        
        readUint16(zipFile);  // version
        readUint16(zipFile);  // flags
        uint16_t compression = readUint16(zipFile);
        readUint16(zipFile);  // mod time
        readUint16(zipFile);  // mod date
        readUint32(zipFile);  // crc32
        readUint32(zipFile);  // compressed size
        readUint32(zipFile);  // uncompressed size
        uint16_t localFilenameLength = readUint16(zipFile);
        uint16_t localExtraLength = readUint16(zipFile);
        
        string localFilename(localFilenameLength, '\0');
        zipFile.read(&localFilename[0], localFilenameLength);
        zipFile.seekg(localExtraLength, ios::cur);
        
        // Read compressed data
        string compressed(compressedSize, '\0');
        zipFile.read(&compressed[0], compressedSize);
        
        // Decompress
        string content;
        if (compression == 0) {
            content = compressed;  // Stored (no compression)
        } else if (compression == 8) {
            content = decompressData(compressed, uncompressedSize);
        } else {
            cerr << "Warning: Unsupported compression method for: " << filename << endl;
            continue;
        }
        
        // Convert path separators
        string destPath = filename;
        replace(destPath.begin(), destPath.end(), '/', PATH_SEPARATOR);
        
        // Build full destination path
        fs::path fullPath = fs::path(realDestDir) / destPath;
        
        // Create parent directories
        fs::create_directories(fullPath.parent_path());
        
        // Write file
        ofstream outFile(fullPath.string(), ios::binary);
        if (outFile.is_open()) {
            outFile.write(content.c_str(), content.length());
            outFile.close();
        }
    }
    
    zipFile.close();
    return true;
}

bool CompressionManager::isZipFile(const string& path) {
    string realPath = PathUtils::virtualToRealPath(path);
    if (!PathUtils::isPathSafe(realPath) || !fs::is_regular_file(realPath)) {
        return false;
    }
    
    ifstream file(realPath, ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    uint32_t sig;
    file.read(reinterpret_cast<char*>(&sig), 4);
    file.close();
    
    // Check for ZIP local file header signature or PKZIP signature
    return (sig == 0x04034b50 || sig == 0x504b0304);
}

vector<string> CompressionManager::listZipContents(const string& zipPath) {
    vector<string> contents;
    string realZipPath = PathUtils::virtualToRealPath(zipPath);
    
    if (!isZipFile(zipPath)) {
        return contents;
    }
    
    ifstream zipFile(realZipPath, ios::binary);
    if (!zipFile.is_open()) {
        return contents;
    }
    
    // Find end of central directory (simplified - just read first entries)
    zipFile.seekg(0, ios::end);
    size_t fileSize = zipFile.tellg();
    
    for (size_t i = fileSize - 22; i > 0 && i < fileSize; --i) {
        zipFile.seekg(i);
        uint32_t sig = readUint32(zipFile);
        if (sig == 0x06054b50) {
            readUint16(zipFile);
            readUint16(zipFile);
            uint16_t numEntries = readUint16(zipFile);
            readUint16(zipFile);
            readUint32(zipFile);
            uint32_t centralDirOffset = readUint32(zipFile);
            
            zipFile.seekg(centralDirOffset);
            for (uint16_t j = 0; j < numEntries && j < 1000; ++j) {  // Limit to 1000 entries
                uint32_t cdSig = readUint32(zipFile);
                if (cdSig != 0x02014b50) break;
                
                readUint16(zipFile);
                readUint16(zipFile);
                readUint16(zipFile);
                readUint16(zipFile);
                readUint16(zipFile);
                readUint16(zipFile);
                readUint32(zipFile);
                readUint32(zipFile);
                readUint32(zipFile);
                uint16_t filenameLength = readUint16(zipFile);
                uint16_t extraFieldLength = readUint16(zipFile);
                uint16_t commentLength = readUint16(zipFile);
                readUint16(zipFile);
                readUint16(zipFile);
                readUint32(zipFile);
                readUint32(zipFile);
                
                string filename(filenameLength, '\0');
                zipFile.read(&filename[0], filenameLength);
                zipFile.seekg(extraFieldLength + commentLength, ios::cur);
                
                contents.push_back(filename);
            }
            break;
        }
    }
    
    zipFile.close();
    return contents;
}