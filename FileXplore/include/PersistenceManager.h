using namespace std;
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
    static string persistence_file;
    static string config_file;
    
    // Internal helper methods
    static bool createPersistenceDirectory();
    static string getPersistenceDirectory();
    static bool writeToFile(const string& filename, const string& content);
    static string readFromFile(const string& filename);
    
public:
    // Initialize persistence system
    static bool initialize(const string& vfs_root);
    
    // Save application state
    static bool saveState();
    
    // Load application state
    static bool loadState();
    
    // Save command history
    static bool saveHistory(const vector<string>& history);
    
    // Load command history
    static vector<string> loadHistory();
    
    // Save VFS metadata (current directory, etc.)
    static bool saveVFSState(const string& current_dir, const string& vfs_root);
    
    // Load VFS metadata
    static map<string, string> loadVFSState();
    
    // Save user settings/preferences
    static bool saveSettings(const map<string, string>& settings);
    
    // Load user settings/preferences
    static map<string, string> loadSettings();
    
    // Clear all persistent data
    static bool clearPersistentData();
    
    // Check if persistence is available
    static bool isPersistenceAvailable();
    
    // Get persistence file paths
    static string getHistoryFile();
    static string getVFSStateFile();
    static string getSettingsFile();
};