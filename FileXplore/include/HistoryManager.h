using namespace std;
#pragma once

#include <string>
#include <vector>
#include <deque>

/**
 * HistoryManager - Manages command history
 * Maintains the last 20 executed commands and provides history functionality
 * Supports persistence through PersistenceManager
 */
class HistoryManager {
private:
    static deque<string> command_history;
    static const size_t MAX_HISTORY_SIZE = 20;

public:
    // Add command to history
    static void addCommand(const string& command);
    
    // Get command history
    static vector<string> getHistory();
    
    // Display command history
    static void displayHistory();
    
    // Clear command history
    static void clearHistory();
    
    // Get history size
    static size_t getHistorySize();
    
    // Get command at specific index (0 = most recent)
    static string getCommand(size_t index);
    
    // Persistence methods
    static bool saveHistory();
    static bool loadHistory();
};