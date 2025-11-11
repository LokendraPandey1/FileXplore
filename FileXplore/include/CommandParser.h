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
        std::string message;
        
        CommandResult(bool s = true, const std::string& msg = "") : success(s), message(msg) {}
    };
    
    // Initialize command parser
    static void initialize();
    
    // Parse and execute a command
    static CommandResult executeCommand(const std::string& input);
    
    // Get list of available commands
    static std::vector<std::string> getAvailableCommands();
    
    // Display help information
    static void displayHelp();
    
private:
    // Command function type
    using CommandFunction = std::function<CommandResult(const std::vector<std::string>&)>;
    
    // Map of command names to functions
    static std::map<std::string, CommandFunction> commands;
    
    // Parse input into command and arguments
    static std::vector<std::string> parseInput(const std::string& input);
    
    // Extract quoted strings from arguments
    static std::string extractQuotedString(const std::vector<std::string>& args, size_t start_index);
    
    // Command implementations
    static CommandResult cmdMkdir(const std::vector<std::string>& args);
    static CommandResult cmdRmdir(const std::vector<std::string>& args);
    static CommandResult cmdLs(const std::vector<std::string>& args);
    static CommandResult cmdTree(const std::vector<std::string>& args);
    static CommandResult cmdCd(const std::vector<std::string>& args);
    static CommandResult cmdPwd(const std::vector<std::string>& args);
    static CommandResult cmdCreate(const std::vector<std::string>& args);
    static CommandResult cmdWrite(const std::vector<std::string>& args);
    static CommandResult cmdAppend(const std::vector<std::string>& args);
    static CommandResult cmdRead(const std::vector<std::string>& args);
    static CommandResult cmdDelete(const std::vector<std::string>& args);
    static CommandResult cmdHelp(const std::vector<std::string>& args);
    static CommandResult cmdClear(const std::vector<std::string>& args);
    static CommandResult cmdHistory(const std::vector<std::string>& args);
    static CommandResult cmdDf(const std::vector<std::string>& args);
    static CommandResult cmdExit(const std::vector<std::string>& args);
};