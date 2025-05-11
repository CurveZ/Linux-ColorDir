// hdir.h 🐧
//
// Header file for ColorDir, a colorful and emoji-enhanced directory listing tool for Linux.
// This file contains function declarations, type definitions, and utility functions used 
// throughout the program.
//
// Author: CurveZero
// Version: 0.3 (Beta)
// Date: May 2025
//

#ifndef HDIR_H
#define HDIR_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <fnmatch.h>
#include <sys/stat.h>
#include <algorithm>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace fs = std::filesystem;

// Enumeration for file types
// Used to categorize files based on their extensions or attributes
enum class FileType {
    Programming,  // Source code or programming-related files
    Text,         // Text documents or configuration files
    Video,        // Video files
    Picture,      // Image files
    Hidden,       // Hidden files or directories
    Executable,   // Executable files
    Compressed,   // Compressed or archive files
    Other         // Files that do not fit into the above categories
};

// Function declarations

// Retrieve file permissions as a string (e.g., "rwxr-xr-x")
std::string getPermissions(const fs::path& path);

// Categorize a file based on its extension or attributes
FileType categorizeFile(const fs::directory_entry& entry);

// Get the appropriate color for a file type or directory
std::string getColor(FileType type);

// Convert a filesystem time to a time_t object
time_t to_time_t(const fs::file_time_type& ftime);

// Print a single file or directory entry with details
void printFile(const fs::directory_entry& entry, const std::string& pattern, bool showDetails);

// List files in a directory with optional recursive and pattern matching
void listFiles(const fs::path& path, const std::string& pattern, bool recursive, bool showDetails);

// Format file size into a human-readable string (e.g., "1.2 MB")
std::string formatSize(uintmax_t size);

// Display the "About" screen with program information and usage examples
void showAboutScreen();

// Display a summary of the total files, directories, and size
void displaySummary(int totalFiles, int totalDirs, std::uintmax_t totalSizeShown);

// Capture a single keypress from the user (used for pause functionality)
char getKeyStroke();

// Function implementations

// Function to format file size into a human-readable string
std::string formatSize(uintmax_t size) {
    if (size < 1024) return std::to_string(size) + " B";
    // Base units (up to YB explicitly)
    const char* baseUnits[] = {"KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    int unitIndex = -1;
    double reducedSize = size;
    // Reduce size iteratively, dynamically adding larger units if needed
    while (reducedSize >= 1024) {
        reducedSize /= 1024;
        unitIndex++;
    }
    // If unitIndex exceeds predefined units, compute extended prefixes
    std::string unit;
    if (unitIndex < 7) {
        unit = baseUnits[unitIndex];
    } else {
        char prefix = 'Z' + (unitIndex - 6); // Beyond ZB: [ZB, AB, BB...]
        unit = std::string(1, prefix) + "B";
    }
    // Format reduced size with 4 significant digits
    return std::to_string(reducedSize).substr(0, 5) + " " + unit;
}

// Function to display the about screen
void showAboutScreen() {
    std::cout << "\033[0;31m"; // Red
    std::cout << "  ____      _            ____  _      _ " << std::endl;
    std::cout << "\033[0;33m"; // Yellow
    std::cout << " / ___|___ | | ___  _ __|  _ \\(_)_ __| |" << std::endl;
    std::cout << "\033[0;32m"; // Green
    std::cout << "| |   / _ \\| |/ _ \\| '__| | | | | '__| |" << std::endl;
    std::cout << "\033[0;36m"; // Cyan
    std::cout << "| |__| (_) | | (_) | |  | |_| | | |  |_|" << std::endl;
    std::cout << "\033[1;35m"; // Bright Magenta
    std::cout << " \\____\\___/|_|\\___/|_|  |____/|_|_|  (_)" << std::endl;
    std::cout << "\033[1;34m"; // Bright Blue
    std::cout << "           !!About ColorDir. v. beta 0.3" << std::endl;
    std::cout << "\033[0;36m"; // Cyan
    std::cout << "This program lists directory contents with color coding." << std::endl;
    std::cout << "History:" << std::endl;
    std::cout << "About 30 years ago, I discovered HDIR, a simple tool that brought color to my directory listings in DOS." << std::endl;
    std::cout << "I loved it then, and today, I set out to create a tribute to it: ColorDir." << std::endl;
    std::cout << "Not as a replacement for ls and its deeper functionalities," << std::endl;
    std::cout << "but as both a nostalgic homage and an aesthetically pleasing way to view files, wrapped in the colors of the past." << std::endl;
    std::cout << "Best regards 💌 endre@neset.love" << std::endl;
    std::cout << std::endl;
    std::cout << " -l, --list       Force list view." << std::endl;
    std::cout << " -w, --wide       Force columns view." << std::endl;
    std::cout << " -t, --total      Display total size of directories, and subdirectories." << std::endl;
    std::cout << " -r, --recursive  Recursive listing." << std::endl;
    std::cout << " -p, --pause      Pause after each screen of output." << std::endl;
    std::cout << " -h, --help       Display this screen." << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: c [flags] [directory] [pattern, must be inside quotes \"\" and must contain at least one * or ?]" << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "1. List all files in the current directory (default):       c" << std::endl;
    std::cout << "2. List all files recursively with detailed listing:        c -r -l" << std::endl;
    std::cout << "3. List files in wide format, recursively:                  c -r -w" << std::endl;
    std::cout << "4. List all .txt files recursively:                         c -r \"*.txt\"" << std::endl;
    std::cout << "5. List .txt files in /home/user/docs directory:            c /home/user/docs \"*.txt\"" << std::endl;
    std::cout << "6. List .log files in /var/log directory:                   c /var/log \"*.log\"" << std::endl;
    std::cout << "7. List files in /usr, paused for viewing, recursively:     c -p -r /usr" << std::endl;
    std::cout << "8. List .config files in /etc directory recursively:        c -r /etc \"*.config\"" << std::endl;
    std::cout << "9. List all files containing an x:                          c  \"*[x]*\"" << std::endl;
    std::cout << "10. List files that do not contain a number:                c \"*[!0-9]*\"" << std::endl;
    std::cout << "\033[0m"; // Reset to default
}

// Function to display a summary of the total files, directories, and size
void displaySummary(int totalFiles, int totalDirs, std::uintmax_t totalSizeShown) {
    std::string totalSummaryString = "Total: Files: " + std::to_string(totalFiles) + 
                                     " | Dirs: " + std::to_string(totalDirs) + 
                                     " | Size: " + formatSize(totalSizeShown) + "\n";

    std::cout << "\033[1;33m"; // Bright Yellow
    for (size_t i = 1; i < totalSummaryString.length(); ++i) {
        std::cout << "─";
    }
    std::cout << "\033[0m" << std::endl; // ANSI Reset
    std::cout << totalSummaryString;
}

// Function to capture a single keypress from the user
char getKeyStroke() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); // Get current terminal settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new settings
    char ch = getchar(); // Capture a single keypress
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore original settings
    return ch;
}

#endif // HDIR_H
