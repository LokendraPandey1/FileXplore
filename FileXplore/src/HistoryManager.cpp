using namespace std;
#include "../include/HistoryManager.h"
#include "../include/PersistenceManager.h"
#include <iostream>
#include <iomanip>

// Static member definition
deque<string> HistoryManager::command_history;

void HistoryManager::addCommand(const string& command) {
    if (command.empty()) {
        return;
    }
    
    // Don't add duplicate consecutive commands
    if (!command_history.empty() && command_history.back() == command) {
        return;
    }
    
    // Add command to history
    command_history.push_back(command);
    
    // Maintain maximum history size
    while (command_history.size() > MAX_HISTORY_SIZE) {
        command_history.pop_front();
    }
}

vector<string> HistoryManager::getHistory() {
    return vector<string>(command_history.begin(), command_history.end());
}

void HistoryManager::displayHistory() {
    if (command_history.empty()) {
        cout << "No command history available." << endl;
        return;
    }
    
    cout << "Command History (last " << command_history.size() << " commands):" << endl;
    cout << string(50, '-') << endl;
    
    for (size_t i = 0; i < command_history.size(); ++i) {
        cout << setw(3) << (i + 1) << ". " << command_history[i] << endl;
    }
    
    cout << string(50, '-') << endl;
}

void HistoryManager::clearHistory() {
    command_history.clear();
    cout << "Command history cleared." << endl;
}

size_t HistoryManager::getHistorySize() {
    return command_history.size();
}

string HistoryManager::getCommand(size_t index) {
    if (index >= command_history.size()) {
        return "";
    }
    
    // Index 0 = most recent, so we need to reverse the index
    size_t actual_index = command_history.size() - 1 - index;
    return command_history[actual_index];
}

bool HistoryManager::saveHistory() {
    vector<string> history = getHistory();
    return PersistenceManager::saveHistory(history);
}

bool HistoryManager::loadHistory() {
    vector<string> history = PersistenceManager::loadHistory();
    
    // Clear current history and load from persistence
    command_history.clear();
    
    for (const string& command : history) {
        if (command_history.size() >= MAX_HISTORY_SIZE) {
            break;
        }
        command_history.push_back(command);
    }
    
    return !history.empty();
}