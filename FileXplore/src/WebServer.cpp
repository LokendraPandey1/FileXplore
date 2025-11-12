#include "../include/WebServer.h"
#include "../include/CommandParser.h"
#include "../include/PathUtils.h"
#include "../include/FileManager.h"
#include "../include/DirManager.h"
#include "../include/HistoryManager.h"
#include "../include/SystemInfo.h"
#include "../include/CompressionManager.h"
#include <sstream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

// URL decode function
std::string urlDecode(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value = 0;
            std::istringstream is(str.substr(i + 1, 2));
            if (is >> std::hex >> value) {
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += str[i];
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    
    return result;
}

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
    CROW_ROUTE((*app_), "/api/command").methods("POST"_method)([this](const crow::request& req) {
        return handleCommand(req);
    });

    CROW_ROUTE((*app_), "/api/filesystem").methods("GET"_method)([this](const crow::request& req) {
        return handleFileSystem(req);
    });

    CROW_ROUTE((*app_), "/api/file/<string>").methods("GET"_method)([this](const crow::request& req, const std::string& path) {
        return handleFileContent(req, path);
    });

    CROW_ROUTE((*app_), "/api/file/<string>").methods("POST"_method)([this](const crow::request& req, const std::string& path) {
        return handleFileUpload(req, path);
    });

    CROW_ROUTE((*app_), "/api/history").methods("GET"_method)([this](const crow::request& req) {
        return handleHistory(req);
    });

    CROW_ROUTE((*app_), "/api/system").methods("GET"_method)([this](const crow::request& req) {
        return handleSystemInfo(req);
    });

    CROW_ROUTE((*app_), "/api/compress").methods("POST"_method)([this](const crow::request& req) {
        return handleCompress(req);
    });

    CROW_ROUTE((*app_), "/api/decompress").methods("POST"_method)([this](const crow::request& req) {
        return handleDecompress(req);
    });

    // Static file serving
    CROW_ROUTE((*app_), "/").methods("GET"_method)([this](const crow::request& req) {
        return handleStaticFile("index.html");
    });

    CROW_ROUTE((*app_), "/<string>").methods("GET"_method)([this](const crow::request& req, const std::string& filename) {
        return handleStaticFile(filename);
    });
}

crow::response WebServer::handleCommand(const crow::request& req) {
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

crow::response WebServer::handleFileSystem(const crow::request& req) {
    try {
        std::string path = req.url_params.get("path") ? std::string(req.url_params.get("path")) : ".";
        // Handle URL-encoded "/" which comes as "%2F"
        if (path == "%2F" || path.empty()) {
            path = "/";
        }
        
        FileSystemData fs_data = getFileSystemData(path);

        json response_json;
        response_json["success"] = true;
        response_json["message"] = "File system data retrieved";
        // generateJSON returns a JSON string, keep it as a string (frontend will parse it)
        response_json["data"] = generateJSON(fs_data);

        crow::response res(response_json.dump());
        addCorsHeaders(res);
        return res;

    } catch (const std::exception& e) {
        std::cerr << "Error in handleFileSystem: " << e.what() << std::endl;
        json error_json;
        error_json["success"] = false;
        error_json["message"] = "Error retrieving file system data: " + std::string(e.what());
        error_json["data"] = "";

        crow::response res(500, error_json.dump());
        addCorsHeaders(res);
        return res;
    }
}

crow::response WebServer::handleFileContent(const crow::request& req, const std::string& path) {
    try {
        // Decode URL-encoded path (Crow doesn't automatically decode route parameters)
        std::string decoded_path = urlDecode(path);
        
        // Check if file exists first
        bool exists = FileManager::fileExists(decoded_path);
        
        if (!exists) {
            json error_json;
            error_json["success"] = false;
            error_json["message"] = "File not found: " + decoded_path;
            error_json["data"] = "";

            crow::response res(404, error_json.dump());
            addCorsHeaders(res);
            return res;
        }

        // Read file content
        std::string content = FileManager::readFile(decoded_path);

        // Check if readFile returned an error message (starts with "Error:")
        if (content.find("Error:") == 0) {
            json error_json;
            error_json["success"] = false;
            error_json["message"] = content;  // Use the error message from FileManager
            error_json["data"] = "";

            crow::response res(500, error_json.dump());
            addCorsHeaders(res);
            return res;
        }

        // Return JSON response with file content
        json response_json;
        response_json["success"] = true;
        response_json["message"] = "File content retrieved";
        response_json["data"] = content;

        crow::response res(response_json.dump());
        res.add_header("Content-Type", "application/json");
        addCorsHeaders(res);
        return res;

    } catch (const std::exception& e) {
        std::cerr << "Error in handleFileContent: " << e.what() << std::endl;
        json error_json;
        error_json["success"] = false;
        error_json["message"] = "Error reading file: " + std::string(e.what());
        error_json["data"] = "";

        crow::response res(500, error_json.dump());
        addCorsHeaders(res);
        return res;
    }
}

crow::response WebServer::handleFileUpload(const crow::request& req, const std::string& path) {
    try {
        // Decode URL-encoded path
        std::string decoded_path = urlDecode(path);
        std::string content = req.body;
        std::string result = FileManager::writeFile(decoded_path, content);

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
        std::cerr << "Error in handleFileUpload: " << e.what() << std::endl;
        json error_json;
        error_json["success"] = false;
        error_json["message"] = "Error uploading file: " + std::string(e.what());
        error_json["data"] = "";

        crow::response res(500, error_json.dump());
        addCorsHeaders(res);
        return res;
    }
}

crow::response WebServer::handleHistory(const crow::request& req) {
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

crow::response WebServer::handleSystemInfo(const crow::request& req) {
    try {
        // Get disk usage information
        std::string vfs_root = PathUtils::getVFSRoot();
        fs::space_info space = fs::space(vfs_root);

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

crow::response WebServer::handleCompress(const crow::request& req) {
    try {
        json request_data = json::parse(req.body);
        std::string zipPath = request_data["zipPath"];
        std::vector<std::string> paths = request_data["paths"];

        if (paths.empty()) {
            json error_json;
            error_json["success"] = false;
            error_json["message"] = "No paths specified for compression";
            error_json["data"] = "";

            crow::response res(400, error_json.dump());
            addCorsHeaders(res);
            return res;
        }

        if (CompressionManager::compressToZip(zipPath, paths)) {
            json response_json;
            response_json["success"] = true;
            response_json["message"] = "Files compressed successfully";
            response_json["data"] = "";

            crow::response res(response_json.dump());
            addCorsHeaders(res);
            return res;
        } else {
            json error_json;
            error_json["success"] = false;
            error_json["message"] = "Failed to compress files";
            error_json["data"] = "";

            crow::response res(500, error_json.dump());
            addCorsHeaders(res);
            return res;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in handleCompress: " << e.what() << std::endl;
        json error_json;
        error_json["success"] = false;
        error_json["message"] = "Error compressing files: " + std::string(e.what());
        error_json["data"] = "";

        crow::response res(500, error_json.dump());
        addCorsHeaders(res);
        return res;
    }
}

crow::response WebServer::handleDecompress(const crow::request& req) {
    try {
        json request_data = json::parse(req.body);
        std::string zipPath = request_data["zipPath"];
        std::string destDir = request_data.value("destDir", ".");

        if (!CompressionManager::isZipFile(zipPath)) {
            json error_json;
            error_json["success"] = false;
            error_json["message"] = "Not a valid zip file: " + zipPath;
            error_json["data"] = "";

            crow::response res(400, error_json.dump());
            addCorsHeaders(res);
            return res;
        }

        if (CompressionManager::decompressFromZip(zipPath, destDir)) {
            json response_json;
            response_json["success"] = true;
            response_json["message"] = "Zip file extracted successfully";
            response_json["data"] = "";

            crow::response res(response_json.dump());
            addCorsHeaders(res);
            return res;
        } else {
            json error_json;
            error_json["success"] = false;
            error_json["message"] = "Failed to extract zip file";
            error_json["data"] = "";

            crow::response res(500, error_json.dump());
            addCorsHeaders(res);
            return res;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in handleDecompress: " << e.what() << std::endl;
        json error_json;
        error_json["success"] = false;
        error_json["message"] = "Error extracting zip file: " + std::string(e.what());
        error_json["data"] = "";

        crow::response res(500, error_json.dump());
        addCorsHeaders(res);
        return res;
    }
}

crow::response WebServer::handleStaticFile(const std::string& filename) {
    try {
        std::string requested_path = filename;
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

    // Normalize path: convert "/" or empty to "." for internal use, but keep "/" for virtual path representation
    std::string virtual_path = (path == "/" || path.empty()) ? "/" : path;
    std::string list_path = (path == "/" || path.empty()) ? "." : path;

    // Get current and parent paths using the virtual path
    std::string real_path = PathUtils::resolveVirtualPath(list_path);
    data.currentPath = virtual_path;  // Return "/" for root, not "."

    fs::path parent_path = fs::path(real_path).parent_path();
    if (parent_path >= fs::path(PathUtils::getVFSRoot())) {
        data.parentPath = PathUtils::getVirtualPath(parent_path.string());
    } else {
        data.parentPath = "";
    }

    // List directory contents using the normalized path
    std::vector<std::string> entries = DirManager::listDirectory(list_path);
    for (const auto& entry : entries) {
        try {
            FileInfo file_info;
            file_info.name = entry;

            // Build full virtual path for the entry
            std::string entry_virtual_path;
            if (virtual_path == "/") {
                entry_virtual_path = "/" + entry;
            } else {
                entry_virtual_path = virtual_path + "/" + entry;
            }

            std::string entry_real_path = PathUtils::resolveVirtualPath(entry_virtual_path);

            if (fs::is_directory(entry_real_path)) {
                file_info.type = "directory";
                file_info.size = 0;
            } else {
                file_info.type = "file";
                // Use the full virtual path for getFileSize
                try {
                    file_info.size = FileManager::getFileSize(entry_virtual_path);
                } catch (...) {
                    file_info.size = 0;  // Default to 0 if size cannot be determined
                }
            }

            // Get modification time
            try {
                auto ftime = fs::last_write_time(entry_real_path);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
                std::stringstream ss;
                ss << std::put_time(std::localtime(&cftime), "%Y-%m-%dT%H:%M:%SZ");
                file_info.modified = ss.str();
            } catch (...) {
                file_info.modified = "";  // Default to empty if time cannot be determined
            }

            file_info.permissions = "rw-r--r--";  // Simplified permissions

            data.files.push_back(file_info);
        } catch (const std::exception& e) {
            // Skip this entry if there's an error, but continue with others
            std::cerr << "Warning: Error processing file entry '" << entry << "': " << e.what() << std::endl;
        }
    }

    return data;
}

void WebServer::addCorsHeaders(crow::response& res) {
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
}