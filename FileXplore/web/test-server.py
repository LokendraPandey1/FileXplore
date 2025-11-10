#!/usr/bin/env python3
"""
Simple HTTP server for testing FileXplore GUI frontend
This allows testing the frontend without the C++ backend
"""

import json
import os
import sys
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs

class FileXploreTestHandler(BaseHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        self.vfs_root = os.path.join(os.path.dirname(__file__), '..', 'filexplore_root')
        super().__init__(*args, **kwargs)

    def do_GET(self):
        parsed_path = urlparse(self.path)

        # Serve static files
        if parsed_path.path.startswith('/api/'):
            self.handle_api_request(parsed_path)
        else:
            self.serve_static_file(parsed_path.path)

    def do_POST(self):
        parsed_path = urlparse(self.path)

        if parsed_path.path.startswith('/api/'):
            self.handle_api_request(parsed_path)
        else:
            self.send_error(404, "Not Found")

    def serve_static_file(self, path):
        # Default to index.html for root
        if path == '/':
            path = '/index.html'

        # Remove leading slash
        file_path = os.path.join(os.path.dirname(__file__), path[1:])

        # Security check
        if '..' in path or not os.path.abspath(file_path).startswith(os.path.dirname(__file__)):
            self.send_error(403, "Forbidden")
            return

        if os.path.exists(file_path) and os.path.isfile(file_path):
            # Determine content type
            if file_path.endswith('.html'):
                content_type = 'text/html'
            elif file_path.endswith('.css'):
                content_type = 'text/css'
            elif file_path.endswith('.js'):
                content_type = 'application/javascript'
            else:
                content_type = 'application/octet-stream'

            self.send_response(200)
            self.send_header('Content-Type', content_type)
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()

            with open(file_path, 'rb') as f:
                self.wfile.write(f.read())
        else:
            self.send_error(404, "File Not Found")

    def handle_api_request(self, parsed_path):
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()

        if parsed_path.path == '/api/filesystem':
            # Mock file system data
            response = {
                "success": True,
                "message": "File system data retrieved",
                "data": {
                    "currentPath": "/",
                    "parentPath": "",
                    "files": [
                        {
                            "name": "documents",
                            "type": "directory",
                            "size": 0,
                            "modified": "2024-01-01T12:00:00Z",
                            "permissions": "rwxr-xr-x"
                        },
                        {
                            "name": "test.txt",
                            "type": "file",
                            "size": 1024,
                            "modified": "2024-01-01T12:00:00Z",
                            "permissions": "rw-r--r--"
                        },
                        {
                            "name": "images",
                            "type": "directory",
                            "size": 0,
                            "modified": "2024-01-01T12:00:00Z",
                            "permissions": "rwxr-xr-x"
                        },
                        {
                            "name": "readme.md",
                            "type": "file",
                            "size": 2048,
                            "modified": "2024-01-01T12:00:00Z",
                            "permissions": "rw-r--r--"
                        }
                    ]
                }
            }
        elif parsed_path.path == '/api/system':
            # Mock system info
            response = {
                "success": True,
                "message": "System information retrieved",
                "data": {
                    "disk_usage": {
                        "total": 1000000000,
                        "free": 500000000,
                        "available": 500000000,
                        "used": 500000000
                    },
                    "file_count": 42,
                    "directory_count": 8,
                    "current_path": "/",
                    "vfs_root": "/tmp/filexplore"
                }
            }
        elif parsed_path.path.startswith('/api/file/'):
            # Mock file content
            filename = parsed_path.path.split('/')[-1]
            response = {
                "success": True,
                "message": f"File content: {filename}",
                "data": f"This is the content of {filename}\n\nLorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat."
            }
        elif parsed_path.path == '/api/command':
            # Get command from POST data
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)

            try:
                command_data = json.loads(post_data.decode('utf-8'))
                command = command_data.get('command', '')
                args = command_data.get('args', [])

                # Mock command execution
                if command == 'ls':
                    response = {
                        "success": True,
                        "message": "Directory listed successfully",
                        "data": ""
                    }
                elif command == 'pwd':
                    response = {
                        "success": True,
                        "message": "/",
                        "data": ""
                    }
                elif command == 'help':
                    response = {
                        "success": True,
                        "message": "Available commands: ls, pwd, mkdir, create, read, write, delete, help",
                        "data": ""
                    }
                else:
                    response = {
                        "success": True,
                        "message": f"Command '{command}' executed with args {args}",
                        "data": ""
                    }
            except json.JSONDecodeError:
                response = {
                    "success": False,
                    "message": "Invalid JSON in request",
                    "data": ""
                }
        else:
            response = {
                "success": False,
                "message": "API endpoint not found",
                "data": ""
            }

        self.wfile.write(json.dumps(response).encode('utf-8'))

    def log_message(self, format, *args):
        """Suppress default logging"""
        pass

def main():
    port = 8080

    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            print("Invalid port number. Using default port 8080.")

    server = HTTPServer(('localhost', port), FileXploreTestHandler)

    print(f"FileXplore Test Server")
    print(f"======================")
    print(f"Server running at: http://localhost:{port}")
    print(f"Serving files from: {os.path.dirname(__file__)}")
    print(f"Press Ctrl+C to stop the server")
    print()
    print("Note: This is a test server with mock data.")
    print("The real FileXplore backend requires compilation with C++17, Crow, and nlohmann::json.")
    print()

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped.")
        server.shutdown()

if __name__ == '__main__':
    main()