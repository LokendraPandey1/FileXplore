# FileXplore Makefile
# Simple build system for FileXplore GUI and CLI

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2
INCLUDES = -Iinclude
LIBS = -pthread

# Source files
SOURCES = src/PathUtils.cpp \
          src/FileManager.cpp \
          src/DirManager.cpp \
          src/CommandParser.cpp \
          src/HistoryManager.cpp \
          src/SystemInfo.cpp \
          src/WebServer.cpp \
          main.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Target executables
CLI_TARGET = FileXplore-CLI
GUI_TARGET = FileXplore-GUI

# Default target
all: $(CLI_TARGET) $(GUI_TARGET)

# CLI target (without web server)
$(CLI_TARGET): $(OBJECTS:.o=_cli.o)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

# GUI target (with web server)
$(GUI_TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# CLI version (excludes WebServer.cpp)
%_cli.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -DNO_WEB_SERVER -c $< -o $@

# Special rule for main.cpp (no WebServer in CLI)
main_cli.o: main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -DNO_WEB_SERVER -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(OBJECTS:.o=_cli.o) $(CLI_TARGET) $(GUI_TARGET)

# Install targets
install: $(CLI_TARGET) $(GUI_TARGET)
	cp $(CLI_TARGET) /usr/local/bin/
	cp $(GUI_TARGET) /usr/local/bin/
	mkdir -p /usr/local/share/filexplore/web
	cp -r web/* /usr/local/share/filexplore/web/

# Run targets
run-cli: $(CLI_TARGET)
	./$(CLI_TARGET)

run-gui: $(GUI_TARGET)
	./$(GUI_TARGET) --gui

# Help target
help:
	@echo "Available targets:"
	@echo "  all       - Build both CLI and GUI versions"
	@echo "  cli       - Build CLI version only"
	@echo "  gui       - Build GUI version only"
	@echo "  clean     - Remove build files"
	@echo "  install   - Install to system"
	@echo "  run-cli   - Build and run CLI version"
	@echo "  run-gui   - Build and run GUI version"
	@echo "  help      - Show this help"

.PHONY: all clean install run-cli run-gui help