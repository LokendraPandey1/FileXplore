#pragma once

#include <string>
#include <vector>
#include <map>

/**
 * PersistenceManager - Handles saving and loading application state
 * Manages persistence of command history, VFS metadata, and user settings
 */
class PersistenceManager {
private:
    static std::string persistence_file;
    static std::string config_file;
    
    // Internal helper methods
    static bool createPersistenceDirectory();
    static std::string getPersistenceDirectory();
    static bool writeToFile(const std::string& filename, const std::string& content);
    static std::string readFromFile(const std::string& filename);
    
public:
    // Initialize persistence system
    static bool initialize(const std::string& vfs_root);
    
    // Save application state
    static bool saveState();
    
    // Load application state
    static bool loadState();
    
    // Save command history
    static bool saveHistory(const std::vector<std::string>& history);
    
    // Load command history
    static std::vector<std::string> loadHistory();
    
    // Save VFS metadata (current directory, etc.)
    static bool saveVFSState(const std::string& current_dir, const std::string& vfs_root);
    
    // Load VFS metadata
    static std::map<std::string, std::string> loadVFSState();
    
    // Save user settings/preferences
    static bool saveSettings(const std::map<std::string, std::string>& settings);
    
    // Load user settings/preferences
    static std::map<std::string, std::string> loadSettings();
    
    // Clear all persistent data
    static bool clearPersistentData();
    
    // Check if persistence is available
    static bool isPersistenceAvailable();
    
    // Get persistence file paths
    static std::string getHistoryFile();
    static std::string getVFSStateFile();
    static std::string getSettingsFile();
};