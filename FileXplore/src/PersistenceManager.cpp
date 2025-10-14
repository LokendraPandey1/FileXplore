using namespace std;
#include "../include/PersistenceManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
    #include <direct.h>
    #include <sys/stat.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <unistd.h>
    #include <pwd.h>
    #include <sys/stat.h>
#endif

// Static member definitions
string PersistenceManager::persistence_file = "";
string PersistenceManager::config_file = "";

bool PersistenceManager::initialize(const string& vfs_root) {
    try {
        string persist_dir = getPersistenceDirectory();
        if (persist_dir.empty()) {
            cerr << "Warning: Could not determine persistence directory" << endl;
            return false;
        }
        
        if (!createPersistenceDirectory()) {
            cerr << "Warning: Could not create persistence directory" << endl;
            return false;
        }
        
        persistence_file = persist_dir + "/filexplore_state.json";
        config_file = persist_dir + "/filexplore_config.json";
        
        return true;
    } catch (const exception& e) {
        cerr << "Error initializing persistence: " << e.what() << endl;
        return false;
    }
}

string PersistenceManager::getPersistenceDirectory() {
    string home_dir;
    
#ifdef _WIN32
    const char* appdata = getenv("APPDATA");
    if (appdata != nullptr) {
        home_dir = string(appdata) + "\\FileXplore";
    }
#else
    const char* home = getenv("HOME");
    if (home) {
        home_dir = string(home) + "/.filexplore";
    }
#endif
    
    return home_dir;
}

bool PersistenceManager::createPersistenceDirectory() {
    try {
        string persist_dir = getPersistenceDirectory();
        if (persist_dir.empty()) {
            return false;
        }
        
        // Check if directory exists
        struct stat info;
        if (stat(persist_dir.c_str(), &info) != 0) {
            // Directory doesn't exist, try to create it
            if (mkdir(persist_dir.c_str(), 0755) != 0) {
                return false;
            }
        } else if (!(info.st_mode & S_IFDIR)) {
            // Path exists but is not a directory
            return false;
        }
        
        return true;
    } catch (const exception& e) {
        cerr << "Error creating persistence directory: " << e.what() << endl;
        return false;
    }
}

bool PersistenceManager::writeToFile(const string& filename, const string& content) {
    try {
        ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        file << content;
        file.close();
        return true;
    } catch (const exception& e) {
        cerr << "Error writing to file " << filename << ": " << e.what() << endl;
        return false;
    }
}

string PersistenceManager::readFromFile(const string& filename) {
    try {
        ifstream file(filename);
        if (!file.is_open()) {
            return "";
        }
        
        stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        return buffer.str();
    } catch (const exception& e) {
        cerr << "Error reading from file " << filename << ": " << e.what() << endl;
        return "";
    }
}

bool PersistenceManager::saveHistory(const vector<string>& history) {
    if (persistence_file.empty()) {
        return false;
    }
    
    try {
        stringstream json;
        json << "{\n";
        json << "  \"history\": [\n";
        
        for (size_t i = 0; i < history.size(); ++i) {
            json << "    \"" << history[i] << "\"";
            if (i < history.size() - 1) {
                json << ",";
            }
            json << "\n";
        }
        
        json << "  ],\n";
        json << "  \"timestamp\": " << time(nullptr) << "\n";
        json << "}\n";
        
        return writeToFile(getHistoryFile(), json.str());
    } catch (const exception& e) {
        cerr << "Error saving history: " << e.what() << endl;
        return false;
    }
}

vector<string> PersistenceManager::loadHistory() {
    vector<string> history;
    
    if (persistence_file.empty()) {
        return history;
    }
    
    try {
        string content = readFromFile(getHistoryFile());
        if (content.empty()) {
            return history;
        }
        
        // Simple JSON parsing for history array
        size_t start = content.find("\"history\": [");
        if (start == string::npos) {
            return history;
        }
        
        start = content.find('[', start);
        size_t end = content.find(']', start);
        if (start == string::npos || end == string::npos) {
            return history;
        }
        
        string history_section = content.substr(start + 1, end - start - 1);
        stringstream ss(history_section);
        string line;
        
        while (getline(ss, line)) {
            // Remove whitespace and quotes
            line.erase(0, line.find_first_not_of(" \t\n\r"));
            line.erase(line.find_last_not_of(" \t\n\r,") + 1);
            
            if (line.length() >= 2 && line.front() == '\"' && line.back() == '\"') {
                line = line.substr(1, line.length() - 2);
                if (!line.empty()) {
                    history.push_back(line);
                }
            }
        }
        
    } catch (const exception& e) {
        cerr << "Error loading history: " << e.what() << endl;
    }
    
    return history;
}

bool PersistenceManager::saveVFSState(const string& current_dir, const string& vfs_root) {
    if (persistence_file.empty()) {
        return false;
    }
    
    try {
        stringstream json;
        json << "{\n";
        json << "  \"current_directory\": \"" << current_dir << "\",\n";
        json << "  \"vfs_root\": \"" << vfs_root << "\",\n";
        json << "  \"timestamp\": " << time(nullptr) << "\n";
        json << "}\n";
        
        return writeToFile(getVFSStateFile(), json.str());
    } catch (const exception& e) {
        cerr << "Error saving VFS state: " << e.what() << endl;
        return false;
    }
}

map<string, string> PersistenceManager::loadVFSState() {
    map<string, string> state;
    
    if (persistence_file.empty()) {
        return state;
    }
    
    try {
        string content = readFromFile(getVFSStateFile());
        if (content.empty()) {
            return state;
        }
        
        // Simple JSON parsing for key-value pairs
        auto extractValue = [&content](const string& key) -> string {
            string search = "\"" + key + "\": \"";
            size_t start = content.find(search);
            if (start == string::npos) {
                return "";
            }
            start += search.length();
            size_t end = content.find('\"', start);
            if (end == string::npos) {
                return "";
            }
            return content.substr(start, end - start);
        };
        
        state["current_directory"] = extractValue("current_directory");
        state["vfs_root"] = extractValue("vfs_root");
        
    } catch (const exception& e) {
        cerr << "Error loading VFS state: " << e.what() << endl;
    }
    
    return state;
}

bool PersistenceManager::saveSettings(const map<string, string>& settings) {
    if (config_file.empty()) {
        return false;
    }
    
    try {
        stringstream json;
        json << "{\n";
        
        size_t count = 0;
        for (const auto& pair : settings) {
            json << "  \"" << pair.first << "\": \"" << pair.second << "\"";
            if (count < settings.size() - 1) {
                json << ",";
            }
            json << "\n";
            count++;
        }
        
        json << "}\n";
        
        return writeToFile(getSettingsFile(), json.str());
    } catch (const exception& e) {
        cerr << "Error saving settings: " << e.what() << endl;
        return false;
    }
}

map<string, string> PersistenceManager::loadSettings() {
    map<string, string> settings;
    
    if (config_file.empty()) {
        return settings;
    }
    
    try {
        string content = readFromFile(getSettingsFile());
        if (content.empty()) {
            return settings;
        }
        
        // Simple JSON parsing for settings
        stringstream ss(content);
        string line;
        
        while (getline(ss, line)) {
            size_t colon = line.find(':');
            if (colon == string::npos) {
                continue;
            }
            
            string key = line.substr(0, colon);
            string value = line.substr(colon + 1);
            
            // Remove quotes and whitespace
            auto clean = [](string& str) {
                str.erase(0, str.find_first_not_of(" \t\n\r\""));
                str.erase(str.find_last_not_of(" \t\n\r\",") + 1);
            };
            
            clean(key);
            clean(value);
            
            if (!key.empty() && !value.empty()) {
                settings[key] = value;
            }
        }
        
    } catch (const exception& e) {
        cerr << "Error loading settings: " << e.what() << endl;
    }
    
    return settings;
}

bool PersistenceManager::saveState() {
    // This method can be used to save all state at once
    // For now, it's a placeholder that returns true
    return true;
}

bool PersistenceManager::loadState() {
    // This method can be used to load all state at once
    // For now, it's a placeholder that returns true
    return true;
}

bool PersistenceManager::clearPersistentData() {
    try {
        string history_file = getHistoryFile();
        string vfs_file = getVFSStateFile();
        string settings_file = getSettingsFile();
        
        // Remove files if they exist
        if (!history_file.empty()) {
            remove(history_file.c_str());
        }
        if (!vfs_file.empty()) {
            remove(vfs_file.c_str());
        }
        if (!settings_file.empty()) {
            remove(settings_file.c_str());
        }
        
        return true;
    } catch (const exception& e) {
        cerr << "Error clearing persistent data: " << e.what() << endl;
        return false;
    }
}

bool PersistenceManager::isPersistenceAvailable() {
    return !persistence_file.empty() && !config_file.empty();
}

string PersistenceManager::getHistoryFile() {
    string persist_dir = getPersistenceDirectory();
    return persist_dir + "/history.json";
}

string PersistenceManager::getVFSStateFile() {
    string persist_dir = getPersistenceDirectory();
    return persist_dir + "/vfs_state.json";
}

string PersistenceManager::getSettingsFile() {
    return config_file;
}