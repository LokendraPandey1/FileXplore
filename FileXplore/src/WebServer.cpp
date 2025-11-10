#include "../include/WebServer.h"
#include "../include/CommandParser.h"
#include "../include/PathUtils.h"
#include "../include/FileManager.h"
#include "../include/DirManager.h"
#include "../include/HistoryManager.h"
#include "../include/SystemInfo.h"
#include <crow.h>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

WebServer::WebServer(int port) : port_(port), running_(false) {
    app_ = std::make_unique<crow::SimpleApp>();
}

WebServer::~WebServer() {
    stop();
}

bool WebServer::start() {
    if (running_) {
        return true;
    }

    try {
        setupRoutes();

        // Start server in a separate thread
        server_thread_ = std::make_unique<std::thread>([this]() {
            app_->port(port_).multithreaded().run();
        });

        running_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start web server: " << e.what() << std::endl;
        return false;
    }
}

void WebServer::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    app_->stop();

    if (server_thread_ && server_thread_->joinable()) {
        server_thread_->join();
    }
}

bool WebServer::isRunning() const {
    return running_;
}

int WebServer::getPort() const {
    return port_;
}

void WebServer::setupRoutes() {
    // API Routes
    CROW_ROUTE(*app_, "/api/command").methods("POST"_method)([this](const crow::request& req) {
        return handleCommand(req);
    });

    CROW_ROUTE(*app_, "/api/filesystem").methods("GET"_method)([this](const crow::request& req) {
        return handleFileSystem(req);
    });

    CROW_ROUTE(*app_, "/api/file/<string>").methods("GET"_method)([this](const crow::request& req, const std::string& path) {
        return handleFileContent(req, path);
    });

    CROW_ROUTE(*app_, "/api/file/<string>").methods("POST"_method)([this](const crow::request& req, const std::string& path) {
        return handleFileUpload(req, path);
    });

    CROW_ROUTE(*app_, "/api/history").methods("GET"_method)([this](const crow::request& req) {
        return handleHistory(req);
    });

    CROW_ROUTE(*app_, "/api/system").methods("GET"_method)([this](const crow::request& req) {
        return handleSystemInfo(req);
    });

    // Static file serving
    CROW_ROUTE(*app_, "/").methods("GET"_method)([this](const crow::request& req) {
        return handleStaticFile("index.html");
    });

    CROW_ROUTE(*app_, "/<string>").methods("GET"_method)([this](const crow::request& req, const std::string& filename) {
        return handleStaticFile(filename);
    });
}

void WebServer::handleCommand() {
    try {
        // Parse JSON request
        json request_data = json::parse(req.body);
        std::string command = request_data["command"];
        std::vector<std::string> args = request_data["args"];

        // Execute command using existing CommandParser
        ApiResponse response = executeCommandAPI(command, args);

        // Create JSON response
        json response_json;
        response_json["success"] = response.success;
        response_json["message"] = response.message;
        response_json["data"] = response.data;

        crow::response res(response_json.dump());
        addCorsHeaders(res);
        return res;

    } catch (const std::exception& e) {
        json error_json;
        error_json["success"] = false;
        error_json["message"] = "Invalid request format: " + std::string(e.what());
        error_json["data"] = "";

        crow::response res(400, error_json.dump());
        addCorsHeaders(res);
        return res;
    }
}

void WebServer::handleFileSystem() {
    try {
        std::string path = req.url_params.get("path") ? req.url_params.get("path") : ".";
        FileSystemData fs_data = getFileSystemData(path);

        json response_json;
        response_json["success"] = true;
        response_json["message"] = "File system data retrieved";
        response_json["data"] = json::parse(generateJSON(fs_data));

        crow::response res(response_json.dump());
        addCorsHeaders(res);
        return res;

    } catch (const std::exception& e) {
        json error_json;
        error_json["success"] = false;
        error_json["message"] = "Error retrieving file system data: " + std::string(e.what());
        error_json["data"] = "";

        crow::response res(500, error_json.dump());
        addCorsHeaders(res);
        return res;
    }
}

void WebServer::handleFileContent(const std::string& path) {
    try {
        std::string content = FileManager::readFile(path);

        if (!content.empty() || FileManager::fileExists(path)) {
            crow::response res(200, content);
            res.add_header("Content-Type", getMimeType(path));
            addCorsHeaders(res);
            return res;
        } else {
            json error_json;
            error_json["success"] = false;
            error_json["message"] = "File not found: " + path;
            error_json["data"] = "";

            crow::response res(404, error_json.dump());
            addCorsHeaders(res);
            return res;
        }
    } catch (const std::exception& e) {
        json error_json;
        error_json["success"] = false;
        error_json["message"] = "Error reading file: " + std::string(e.what());
        error_json["data"] = "";

        crow::response res(500, error_json.dump());
        addCorsHeaders(res);
        return res;
    }
}

void WebServer::handleFileUpload(const std::string& path) {
    try {
        std::string content = req.body;
        std::string result = FileManager::writeFile(path, content);

        if (result.find("Error:") == 0) {
            json error_json;
            error_json["success"] = false;
            error_json["message"] = result;
            error_json["data"] = "";

            crow::response res(400, error_json.dump());
            addCorsHeaders(res);
            return res;
        } else {
            json response_json;
            response_json["success"] = true;
            response_json["message"] = result;
            response_json["data"] = "";

            crow::response res(response_json.dump());
            addCorsHeaders(res);
            return res;
        }
    } catch (const std::exception& e) {
        json error_json;
        error_json["success"] = false;
        error_json["message"] = "Error uploading file: " + std::string(e.what());
        error_json["data"] = "";

        crow::response res(500, error_json.dump());
        addCorsHeaders(res);
        return res;
    }
}

void WebServer::handleHistory() {
    try {
        std::vector<std::string> history = HistoryManager::getHistory();

        json response_json;
        response_json["success"] = true;
        response_json["message"] = "Command history retrieved";
        response_json["data"] = json::parse(generateJSON(history));

        crow::response res(response_json.dump());
        addCorsHeaders(res);
        return res;

    } catch (const std::exception& e) {
        json error_json;
        error_json["success"] = false;
        error_json["message"] = "Error retrieving history: " + std::string(e.what());
        error_json["data"] = "";

        crow::response res(500, error_json.dump());
        addCorsHeaders(res);
        return res;
    }
}

void WebServer::handleSystemInfo() {
    try {
        // Get disk usage information
        std::string vfs_root = PathUtils::getVFSRoot();
        space_info space = fs::space(vfs_root);

        // Get file count
        size_t file_count = 0;
        size_t dir_count = 0;
        for (const auto& entry : fs::recursive_directory_iterator(vfs_root)) {
            if (entry.is_regular_file()) {
                file_count++;
            } else if (entry.is_directory()) {
                dir_count++;
            }
        }

        json system_data;
        system_data["disk_usage"] = {
            {"total", space.capacity},
            {"free", space.free},
            {"available", space.available},
            {"used", space.capacity - space.free}
        };
        system_data["file_count"] = file_count;
        system_data["directory_count"] = dir_count;
        system_data["current_path"] = PathUtils::getCurrentVirtualPath();
        system_data["vfs_root"] = vfs_root;

        json response_json;
        response_json["success"] = true;
        response_json["message"] = "System information retrieved";
        response_json["data"] = system_data;

        crow::response res(response_json.dump());
        addCorsHeaders(res);
        return res;

    } catch (const std::exception& e) {
        json error_json;
        error_json["success"] = false;
        error_json["message"] = "Error retrieving system info: " + std::string(e.what());
        error_json["data"] = "";

        crow::response res(500, error_json.dump());
        addCorsHeaders(res);
        return res;
    }
}

void WebServer::handleStaticFiles() {
    try {
        std::string requested_path = req.url_params.get("path") ? req.url_params.get("path") : "index.html";
        std::string full_path = "web/" + requested_path;

        // Security check - prevent directory traversal
        if (requested_path.find("..") != std::string::npos) {
            crow::response res(403, "Access denied");
            return res;
        }

        if (fs::exists(full_path) && fs::is_regular_file(full_path)) {
            std::ifstream file(full_path, std::ios::binary);
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            crow::response res(200, content);
            res.add_header("Content-Type", getMimeType(full_path));
            addCorsHeaders(res);
            return res;
        } else {
            crow::response res(404, "File not found");
            return res;
        }
    } catch (const std::exception& e) {
        crow::response res(500, "Internal server error");
        return res;
    }
}

std::string WebServer::generateJSON(const FileSystemData& data) {
    json j;
    j["currentPath"] = data.currentPath;
    j["parentPath"] = data.parentPath;

    json files = json::array();
    for (const auto& file : data.files) {
        json file_obj;
        file_obj["name"] = file.name;
        file_obj["type"] = file.type;
        file_obj["size"] = file.size;
        file_obj["modified"] = file.modified;
        file_obj["permissions"] = file.permissions;
        files.push_back(file_obj);
    }
    j["files"] = files;

    return j.dump();
}

std::string WebServer::generateJSON(const std::vector<std::string>& history) {
    json j = json::array();
    for (const auto& cmd : history) {
        j.push_back(cmd);
    }
    return j.dump();
}

std::string WebServer::formatError(const std::string& error) {
    json j;
    j["success"] = false;
    j["message"] = error;
    j["data"] = "";
    return j.dump();
}

std::string WebServer::getMimeType(const std::string& filepath) {
    std::string extension = fs::path(filepath).extension().string();

    if (extension == ".html") return "text/html";
    if (extension == ".css") return "text/css";
    if (extension == ".js") return "application/javascript";
    if (extension == ".json") return "application/json";
    if (extension == ".txt") return "text/plain";
    if (extension == ".png") return "image/png";
    if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
    if (extension == ".gif") return "image/gif";
    if (extension == ".svg") return "image/svg+xml";

    return "application/octet-stream";
}

WebServer::ApiResponse WebServer::executeCommandAPI(const std::string& command, const std::vector<std::string>& args) {
    // Build command string for existing CommandParser
    std::string full_command = command;
    for (const auto& arg : args) {
        full_command += " " + arg;
    }

    // Execute using existing CommandParser
    CommandParser::CommandResult result = CommandParser::executeCommand(full_command);

    if (result.success) {
        // For commands that return data (ls, pwd, etc.), capture output
        std::string data = "";
        if (command == "ls" || command == "tree") {
            // Get file system data for these commands
            FileSystemData fs_data = getFileSystemData(args.empty() ? "." : args[0]);
            data = generateJSON(fs_data);
        } else if (command == "pwd") {
            data = "\"" + PathUtils::getCurrentVirtualPath() + "\"";
        } else if (command == "read") {
            if (!args.empty()) {
                std::string content = FileManager::readFile(args[0]);
                data = "\"" + content + "\"";
            }
        }

        return ApiResponse(true, result.message, data);
    } else {
        return ApiResponse(false, result.message, "");
    }
}

WebServer::FileSystemData WebServer::getFileSystemData(const std::string& path) {
    FileSystemData data;

    // Get current and parent paths
    std::string real_path = PathUtils::resolveVirtualPath(path);
    data.currentPath = PathUtils::getVirtualPath(real_path);

    fs::path parent_path = fs::path(real_path).parent_path();
    if (parent_path >= fs::path(PathUtils::getVFSRoot())) {
        data.parentPath = PathUtils::getVirtualPath(parent_path.string());
    } else {
        data.parentPath = "";
    }

    // List directory contents
    std::vector<std::string> entries = DirManager::listDirectory(path);
    for (const auto& entry : entries) {
        FileInfo file_info;
        file_info.name = entry;

        std::string entry_path = PathUtils::resolveVirtualPath((path == ".") ? entry : path + "/" + entry);

        if (fs::is_directory(entry_path)) {
            file_info.type = "directory";
            file_info.size = 0;
        } else {
            file_info.type = "file";
            file_info.size = FileManager::getFileSize(entry);
        }

        // Get modification time
        auto ftime = fs::last_write_time(entry_path);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&cftime), "%Y-%m-%dT%H:%M:%SZ");
        file_info.modified = ss.str();

        file_info.permissions = "rw-r--r--";  // Simplified permissions

        data.files.push_back(file_info);
    }

    return data;
}

void WebServer::addCorsHeaders(crow::response& res) {
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
}