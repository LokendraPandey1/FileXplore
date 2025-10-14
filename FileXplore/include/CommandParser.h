using namespace std;
#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

/**
 * CommandParser - Parses and executes CLI commands
 * Handles command parsing, argument extraction, and command execution
 */
class CommandParser {
public:
    // Command execution result
    struct CommandResult {
        bool success;
        string message;
        
        CommandResult(bool s = true, const string& msg = "") : success(s), message(msg) {}
    };
    
    // Initialize command parser
    static void initialize();
    
    // Parse and execute a command
    static CommandResult executeCommand(const string& input);
    
    // Get list of available commands
    static vector<string> getAvailableCommands();
    
    // Display help information
    static void displayHelp();
    
private:
    // Command function type
    using CommandFunction = function<CommandResult(const vector<string>&)>;
    
    // Map of command names to functions
    static map<string, CommandFunction> commands;
    
    // Parse input into command and arguments
    static vector<string> parseInput(const string& input);
    
    // Extract quoted strings from arguments
    static string extractQuotedString(const vector<string>& args, size_t start_index);
    
    // Command implementations
    static CommandResult cmdMkdir(const vector<string>& args);
    static CommandResult cmdRmdir(const vector<string>& args);
    static CommandResult cmdLs(const vector<string>& args);
    static CommandResult cmdTree(const vector<string>& args);
    static CommandResult cmdCd(const vector<string>& args);
    static CommandResult cmdPwd(const vector<string>& args);
    static CommandResult cmdCreate(const vector<string>& args);
    static CommandResult cmdWrite(const vector<string>& args);
    static CommandResult cmdAppend(const vector<string>& args);
    static CommandResult cmdRead(const vector<string>& args);
    static CommandResult cmdDelete(const vector<string>& args);
    static CommandResult cmdHelp(const vector<string>& args);
    static CommandResult cmdClear(const vector<string>& args);
    static CommandResult cmdHistory(const vector<string>& args);
    static CommandResult cmdDf(const vector<string>& args);
    static CommandResult cmdExit(const vector<string>& args);
};