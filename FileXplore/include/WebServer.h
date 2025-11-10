#pragma once

#include <string>
#include <thread>
#include <memory>
#include <functional>

// Forward declaration for Crow (will be included in implementation)
namespace crow {
    class SimpleApp;
    class response;
}

/**
 * WebServer - HTTP server for GUI communication
 * Provides RESTful API endpoints for FileXplore commands and serves static files
 */
class WebServer {
public:
    // Structure for API responses
    struct ApiResponse {
        bool success;
        std::string message;
        std::string data;  // JSON string

        ApiResponse(bool s = true, const std::string& msg = "", const std::string& d = "")
            : success(s), message(msg), data(d) {}
    };

    // Structure for file information
    struct FileInfo {
        std::string name;
        std::string type;  // "file" or "directory"
        size_t size;
        std::string modified;
        std::string permissions;

        FileInfo(const std::string& n = "", const std::string& t = "file",
                size_t s = 0, const std::string& m = "", const std::string& p = "")
            : name(n), type(t), size(s), modified(m), permissions(p) {}
    };

    // Structure for file system data
    struct FileSystemData {
        std::string currentPath;
        std::string parentPath;
        std::vector<FileInfo> files;

        FileSystemData(const std::string& curr = "", const std::string& parent = "")
            : currentPath(curr), parentPath(parent) {}
    };

public:
    // Constructor
    WebServer(int port = 8080);

    // Destructor
    ~WebServer();

    // Start the web server
    bool start();

    // Stop the web server
    void stop();

    // Check if server is running
    bool isRunning() const;

    // Get server port
    int getPort() const;

private:
    // Server instance
    std::unique_ptr<crow::SimpleApp> app_;

    // Server thread
    std::unique_ptr<std::thread> server_thread_;

    // Server configuration
    int port_;
    bool running_;

    // Setup API routes
    void setupRoutes();

    // API endpoint handlers
    crow::response handleCommand(const crow::request& req);
    crow::response handleFileSystem(const crow::request& req);
    crow::response handleFileContent(const crow::request& req, const std::string& path);
    crow::response handleFileUpload(const crow::request& req, const std::string& path);
    crow::response handleHistory(const crow::request& req);
    crow::response handleSystemInfo(const crow::request& req);

    // Static file serving
    crow::response handleStaticFile(const std::string& filename);

    // Utility methods
    std::string generateJSON(const FileSystemData& data);
    std::string generateJSON(const std::vector<std::string>& history);
    std::string formatError(const std::string& error);
    std::string getMimeType(const std::string& filepath);

    // Convert command results to API responses
    ApiResponse executeCommandAPI(const std::string& command, const std::vector<std::string>& args);
    FileSystemData getFileSystemData(const std::string& path = ".");

    // CORS headers
    void addCorsHeaders(crow::response& res);
};