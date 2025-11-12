using namespace std;
#include "../include/CommandParser.h"
#include "../include/PathUtils.h"
#include "../include/FileManager.h"
#include "../include/DirManager.h"
#include "../include/HistoryManager.h"
#include "../include/SystemInfo.h"
#include "../include/CompressionManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

// Static member definition
map<string, CommandParser::CommandFunction> CommandParser::commands;

void CommandParser::initialize() {
    commands["mkdir"] = cmdMkdir;
    commands["rmdir"] = cmdRmdir;
    commands["ls"] = cmdLs;
    commands["tree"] = cmdTree;
    commands["cd"] = cmdCd;
    commands["pwd"] = cmdPwd;
    commands["create"] = cmdCreate;
    commands["write"] = cmdWrite;
    commands["append"] = cmdAppend;
    commands["read"] = cmdRead;
    commands["delete"] = cmdDelete;
    commands["help"] = cmdHelp;
    commands["clear"] = cmdClear;
    commands["history"] = cmdHistory;
    commands["df"] = cmdDf;
    commands["zip"] = cmdZip;
    commands["unzip"] = cmdUnzip;
    commands["exit"] = cmdExit;
}

CommandParser::CommandResult CommandParser::executeCommand(const string& input) {
    if (input.empty()) {
        return CommandResult(true, "");
    }
    
    vector<string> tokens = parseInput(input);
    if (tokens.empty()) {
        return CommandResult(true, "");
    }
    
    string command = tokens[0];
    transform(command.begin(), command.end(), command.begin(), ::tolower);
    
    // Add command to history (except for history command itself)
    if (command != "history") {
        HistoryManager::addCommand(input);
    }
    
    auto it = commands.find(command);
    if (it != commands.end()) {
        return it->second(tokens);
    } else {
        return CommandResult(false, "Unknown command: " + command + ". Type 'help' for available commands.");
    }
}

vector<string> CommandParser::getAvailableCommands() {
    vector<string> cmd_list;
    for (const auto& pair : commands) {
        cmd_list.push_back(pair.first);
    }
    sort(cmd_list.begin(), cmd_list.end());
    return cmd_list;
}

void CommandParser::displayHelp() {
    cout << string(70, '=') << endl;
    cout << "FileXplore - Virtual File System Simulator" << endl;
    cout << string(70, '=') << endl;
    cout << "Available Commands:" << endl;
    cout << string(70, '-') << endl;
    
    cout << "Directory Operations:" << endl;
    cout << "  mkdir <path>        - Create directory" << endl;
    cout << "  rmdir <path>        - Remove empty directory" << endl;
    cout << "  ls [path]           - List directory contents" << endl;
    cout << "  tree [path]         - Display directory tree" << endl;
    cout << "  cd <path>           - Change current directory" << endl;
    cout << "  pwd                 - Show current directory" << endl;
    
    cout << endl << "File Operations:" << endl;
    cout << "  create <path>       - Create empty file" << endl;
    cout << "  write <path> \"text\" - Write content to file (overwrite)" << endl;
    cout << "  append <path> \"text\"- Append content to file" << endl;
    cout << "  read <path>         - Display file content" << endl;
    cout << "  delete <path>       - Delete file" << endl;
    
    cout << endl << "Compression:" << endl;
    cout << "  zip <output.zip> <path1> [path2] ... - Compress files/directories to zip" << endl;
    cout << "  unzip <input.zip> [dest_dir]         - Extract zip file to directory" << endl;
    
    cout << endl << "System & Utility:" << endl;
    cout << "  df                  - Show disk usage statistics" << endl;
    cout << "  history             - Show command history" << endl;
    cout << "  clear               - Clear terminal screen" << endl;
    cout << "  help                - Show this help message" << endl;
    cout << "  exit                - Exit FileXplore" << endl;
    
    cout << string(70, '-') << endl;
    cout << "Path Examples:" << endl;
    cout << "  Absolute: /home/user/documents/file.txt" << endl;
    cout << "  Relative: documents/file.txt" << endl;
    cout << "  Current:  ./file.txt or file.txt" << endl;
    cout << "  Parent:   ../file.txt" << endl;
    cout << string(70, '=') << endl;
}

vector<string> CommandParser::parseInput(const string& input) {
    vector<string> tokens;
    istringstream iss(input);
    string token;
    bool in_quotes = false;
    string current_token;
    
    for (char c : input) {
        if (c == '"' && !in_quotes) {
            in_quotes = true;
        } else if (c == '"' && in_quotes) {
            in_quotes = false;
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        } else if (c == ' ' && !in_quotes) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        } else {
            current_token += c;
        }
    }
    
    if (!current_token.empty()) {
        tokens.push_back(current_token);
    }
    
    return tokens;
}

string CommandParser::extractQuotedString(const vector<string>& args, size_t start_index) {
    if (start_index >= args.size()) {
        return "";
    }
    
    string result;
    for (size_t i = start_index; i < args.size(); ++i) {
        if (i > start_index) {
            result += " ";
        }
        result += args[i];
    }
    
    return result;
}

// Command implementations
CommandParser::CommandResult CommandParser::cmdMkdir(const vector<string>& args) {
    if (args.size() < 2) {
        return CommandResult(false, "Usage: mkdir <path>");
    }
    
    if (DirManager::createDirectory(args[1])) {
        return CommandResult(true, "Directory created: " + args[1]);
    } else {
        return CommandResult(false, "Failed to create directory: " + args[1]);
    }
}

CommandParser::CommandResult CommandParser::cmdRmdir(const vector<string>& args) {
    if (args.size() < 2) {
        return CommandResult(false, "Usage: rmdir <path>");
    }
    
    if (DirManager::removeDirectory(args[1])) {
        return CommandResult(true, "Directory removed: " + args[1]);
    } else {
        return CommandResult(false, "Failed to remove directory: " + args[1]);
    }
}

CommandParser::CommandResult CommandParser::cmdLs(const vector<string>& args) {
    string path = (args.size() > 1) ? args[1] : ".";
    
    vector<string> entries = DirManager::listDirectory(path);
    if (entries.empty()) {
        return CommandResult(true, "Directory is empty or does not exist.");
    }
    
    cout << "Contents of " << path << ":" << endl;
    for (const auto& entry : entries) {
        cout << "  " << entry << endl;
    }
    
    return CommandResult(true, "");
}

CommandParser::CommandResult CommandParser::cmdTree(const vector<string>& args) {
    string path = (args.size() > 1) ? args[1] : ".";
    
    DirManager::displayTree(path);
    return CommandResult(true, "");
}

CommandParser::CommandResult CommandParser::cmdCd(const vector<string>& args) {
    if (args.size() < 2) {
        return CommandResult(false, "Usage: cd <path>");
    }
    
    if (DirManager::changeDirectory(args[1])) {
        return CommandResult(true, "Changed directory to: " + args[1]);
    } else {
        return CommandResult(false, "Failed to change directory to: " + args[1]);
    }
}

CommandParser::CommandResult CommandParser::cmdPwd(const vector<string>& args) {
    cout << DirManager::getCurrentDirectory() << endl;
    return CommandResult(true, "");
}

CommandParser::CommandResult CommandParser::cmdCreate(const vector<string>& args) {
    if (args.size() < 2) {
        return CommandResult(false, "Usage: create <path>");
    }
    
    string result = FileManager::createFile(args[1]);
    if (result.find("Error:") == 0) {
        return CommandResult(false, result);
    } else {
        return CommandResult(true, result);
    }
}

CommandParser::CommandResult CommandParser::cmdWrite(const vector<string>& args) {
    if (args.size() < 3) {
        return CommandResult(false, "Usage: write <path> \"content\"");
    }
    
    string content = extractQuotedString(args, 2);
    
    string result = FileManager::writeFile(args[1], content);
    if (result.find("Error:") == 0) {
        return CommandResult(false, result);
    } else {
        return CommandResult(true, result);
    }
}

CommandParser::CommandResult CommandParser::cmdAppend(const vector<string>& args) {
    if (args.size() < 3) {
        return CommandResult(false, "Usage: append <path> <content>");
    }
    
    string content = extractQuotedString(args, 2);
    string result = FileManager::appendFile(args[1], content);
    if (result.find("Error:") == 0) {
        return CommandResult(false, result);
    } else {
        return CommandResult(true, result);
    }
}

CommandParser::CommandResult CommandParser::cmdRead(const vector<string>& args) {
    if (args.size() < 2) {
        return CommandResult(false, "Usage: read <path>");
    }
    
    string content = FileManager::readFile(args[1]);
    if (!content.empty() || FileManager::fileExists(args[1])) {
        cout << "Content of " << args[1] << ":" << endl;
        cout << string(50, '-') << endl;
        cout << content << endl;
        cout << string(50, '-') << endl;
        return CommandResult(true, "");
    } else {
        return CommandResult(false, "Failed to read file: " + args[1]);
    }
}

CommandParser::CommandResult CommandParser::cmdDelete(const vector<string>& args) {
    if (args.size() < 2) {
        return CommandResult(false, "Usage: delete <path>");
    }
    
    string result = FileManager::deleteFile(args[1]);
    if (result.find("Error:") == 0) {
        return CommandResult(false, result);
    } else {
        return CommandResult(true, result);
    }
}

CommandParser::CommandResult CommandParser::cmdHelp(const vector<string>& args) {
    displayHelp();
    return CommandResult(true, "");
}

CommandParser::CommandResult CommandParser::cmdClear(const vector<string>& args) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    return CommandResult(true, "");
}

CommandParser::CommandResult CommandParser::cmdHistory(const vector<string>& args) {
    HistoryManager::displayHistory();
    return CommandResult(true, "");
}

CommandParser::CommandResult CommandParser::cmdDf(const vector<string>& args) {
    SystemInfo::displayDiskUsage();
    return CommandResult(true, "");
}

CommandParser::CommandResult CommandParser::cmdZip(const vector<string>& args) {
    if (args.size() < 3) {
        return CommandResult(false, "Usage: zip <output.zip> <path1> [path2] ...");
    }
    
    string zipPath = args[1];
    vector<string> pathsToZip;
    for (size_t i = 2; i < args.size(); ++i) {
        pathsToZip.push_back(args[i]);
    }
    
    if (CompressionManager::compressToZip(zipPath, pathsToZip)) {
        return CommandResult(true, "Files compressed to: " + zipPath);
    } else {
        return CommandResult(false, "Failed to create zip file: " + zipPath);
    }
}

CommandParser::CommandResult CommandParser::cmdUnzip(const vector<string>& args) {
    if (args.size() < 2) {
        return CommandResult(false, "Usage: unzip <input.zip> [dest_dir]");
    }
    
    string zipPath = args[1];
    string destDir = (args.size() > 2) ? args[2] : ".";
    
    if (!CompressionManager::isZipFile(zipPath)) {
        return CommandResult(false, "Error: Not a valid zip file: " + zipPath);
    }
    
    if (CompressionManager::decompressFromZip(zipPath, destDir)) {
        return CommandResult(true, "Zip file extracted to: " + destDir);
    } else {
        return CommandResult(false, "Failed to extract zip file: " + zipPath);
    }
}

CommandParser::CommandResult CommandParser::cmdExit(const vector<string>& args) {
    cout << "Goodbye! Exiting FileXplore..." << endl;
    return CommandResult(true, "EXIT");
}