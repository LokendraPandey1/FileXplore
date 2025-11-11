#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>
using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::left;
using std::setw;
using std::ostringstream;
using std::ifstream;
using std::fixed;
using std::setprecision;
using std::ios;
#include "../include/SystemInfo.h"
#include "../include/PathUtils.h"

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
#else
    #include <sys/statvfs.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

SystemInfo::DiskUsage SystemInfo::getDiskUsage() {
    DiskUsage usage;
    
    string vfs_root = PathUtils::getVFSRoot();
    if (vfs_root.empty()) {
        return usage;
    }
    
    if (PathUtils::pathExists(vfs_root) && PathUtils::isDirectory(vfs_root)) {
        calculateDirectoryStats(vfs_root, usage);
        usage.formatted_size = formatBytes(usage.total_size_bytes);
    }
    
    return usage;
}

void SystemInfo::displayDiskUsage() {
    DiskUsage usage = getDiskUsage();
    
    cout << string(60, '=') << endl;
    cout << "FileXplore Virtual File System Statistics" << endl;
    cout << string(60, '=') << endl;
    
    cout << left << setw(20) << "VFS Root:" 
              << PathUtils::getVFSRoot() << endl;
    cout << left << setw(20) << "Current Directory:" 
              << PathUtils::getCurrentVirtualPath() << endl;
    
    cout << string(60, '-') << endl;
    
    cout << left << setw(20) << "Total Files:" 
              << usage.total_files << endl;
    cout << left << setw(20) << "Total Directories:" 
              << usage.total_directories << endl;
    cout << left << setw(20) << "Total Size:" 
              << usage.formatted_size << " (" << usage.total_size_bytes << " bytes)" << endl;
    
    cout << string(60, '=') << endl;
}

string SystemInfo::formatBytes(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    const size_t num_units = sizeof(units) / sizeof(units[0]);
    
    double size = static_cast<double>(bytes);
    size_t unit_index = 0;
    
    while (size >= 1024.0 && unit_index < num_units - 1) {
        size /= 1024.0;
        unit_index++;
    }
    
    ostringstream oss;
    if (unit_index == 0) {
        oss << static_cast<size_t>(size) << " " << units[unit_index];
    } else {
        oss << fixed << setprecision(2) << size << " " << units[unit_index];
    }
    
    return oss.str();
}

string SystemInfo::getVFSInfo() {
    ostringstream info;
    DiskUsage usage = getDiskUsage();
    
    info << "VFS Root: " << PathUtils::getVFSRoot() << "\n";
    info << "Current Directory: " << PathUtils::getCurrentVirtualPath() << "\n";
    info << "Files: " << usage.total_files << ", Directories: " << usage.total_directories;
    info << ", Size: " << usage.formatted_size;
    
    return info.str();
}

void SystemInfo::calculateDirectoryStats(const string& path, DiskUsage& usage) {
    if (!PathUtils::pathExists(path) || !PathUtils::isDirectory(path)) {
        return;
    }
    
    // Use DirManager to list directory contents
    string virtual_path = "/"; // Convert to virtual path if needed
    vector<string> entries;
    
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    string searchPath = path + "\\*";
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            string filename = findFileData.cFileName;
            if (filename != "." && filename != "..") {
                string fullPath = path + "\\" + filename;
                if (PathUtils::isDirectory(fullPath)) {
                    usage.total_directories++;
                    calculateDirectoryStats(fullPath, usage);
                } else if (PathUtils::isFile(fullPath)) {
                    usage.total_files++;
                    // Get file size using ifstream
                    ifstream file(fullPath, ios::binary | ios::ate);
                    if (file.is_open()) {
                        usage.total_size_bytes += file.tellg();
                        file.close();
                    }
                }
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(path.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            string filename = entry->d_name;
            if (filename != "." && filename != "..") {
                string fullPath = path + "/" + filename;
                if (PathUtils::isDirectory(fullPath)) {
                    usage.total_directories++;
                    calculateDirectoryStats(fullPath, usage);
                } else if (PathUtils::isFile(fullPath)) {
                    usage.total_files++;
                    // Get file size using ifstream
                    ifstream file(fullPath, ios::binary | ios::ate);
                    if (file.is_open()) {
                        usage.total_size_bytes += file.tellg();
                        file.close();
                    }
                }
            }
        }
        closedir(dir);
    }
#endif
}