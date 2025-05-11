//
// Author: CurveZero
// Version: 0.3 (Beta)
// Date: May 2025
// Description: 
//   This program provides a colorful and emoji-enhanced directory listing tool for Linux.
//   It categorizes files by type, displays them with appropriate colors and icons, 
//   and supports features like recursive listing, pattern matching, and multi-column display.
//
// Features:
//   - Color-coded file types (e.g., programming files, text files, videos, pictures, etc.)
//   - Emoji support for file types and directories
//   - Recursive directory listing
//   - Human-readable file sizes
//   - Multi-column or detailed list view
//   - Pattern matching with wildcards (*, ?)
//
// Usage:
//   c [flags] [directory] [pattern]
//   Flags:
//     -r, --recursive   Recursive listing
//     -t, --total       Display total size of directories
//     -l, --list        Force detailed list view
//     -w, --wide        Force multi-column view
//     -p, --pause       Pause after each screen of output
//     -h, --help        Display help information

#include "hdir.h"
#include <unordered_set> // Unique to c.cpp

// Retrieve file permissions as a string (e.g., "rwxr-xr-x")
std::string getPermissions(const fs::path& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return "?????????"; // Return placeholder if stat fails
    }

    std::string permissions;
    permissions += (S_ISDIR(info.st_mode)) ? 'd' : '-';
    permissions += (info.st_mode & S_IRUSR) ? 'r' : '-';
    permissions += (info.st_mode & S_IWUSR) ? 'w' : '-';
    permissions += (info.st_mode & S_IXUSR) ? 'x' : '-';
    permissions += (info.st_mode & S_IRGRP) ? 'r' : '-';
    permissions += (info.st_mode & S_IWGRP) ? 'w' : '-';
    permissions += (info.st_mode & S_IXGRP) ? 'x' : '-';
    permissions += (info.st_mode & S_IROTH) ? 'r' : '-';
    permissions += (info.st_mode & S_IWOTH) ? 'w' : '-';
    permissions += (info.st_mode & S_IXOTH) ? 'x' : '-';

    return permissions;
}

// Categorize a file based on its extension or attributes
FileType categorizeFile(const fs::directory_entry& entry) {
    // Define common programming file extensions
    static const std::unordered_set<std::string> programmingExtensions = {
        ".cpp", ".h", ".py", ".java", ".cs", ".js", ".php", ".hs", ".rs", ".clj", ".sh", ".pl", ".lua",
        ".erl", ".ex", ".exs", ".scala", ".d", ".go", ".nim", ".lisp", ".cl", ".f90", ".f95", ".vhdl", ".verilog", 
        ".coffee", ".racket", ".dart", ".tcl", ".hlsl"
    };

    // Define common text file extensions
    static const std::unordered_set<std::string> textExtensions = {
        ".txt", ".md", ".rtf", ".log", ".ini", ".conf", ".config", ".nfo", ".readme", 
        ".html", ".htm", ".bak", ".asc", ".diff", ".lst", ".srt", ".mdown", ".text", 
        ".out", ".memo", ".patch", ".logfile", ".po", ".dat", ".env", ".sh", ".doc"
    };

    // Define common and less common video file extensions
    static const std::unordered_set<std::string> videoExtensions = {
        // Common video formats
        ".mp4", ".mkv", ".avi", ".mov", ".wmv", 
        ".flv", ".webm", ".mpeg", ".mpg", ".m4v", 
        ".3gp", ".ogv", ".vob", ".ts", ".m2ts", 
        // Less common but explicitly video-related formats
        ".divx", ".rm", ".rmvb", ".asf", ".swf", 
        ".mxf", ".hevc", ".avchd", ".mts", ".ogm", 
        ".amv", ".drc", ".yuv", ".h264", ".h265"
    };

    // Define common picture file extensions
    static const std::unordered_set<std::string> pictureExtensions = {
        ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".tiff", ".tif", ".webp", 
        ".svg", ".ico", ".raw", ".xpm", ".ppm", ".pgm", ".pbm", ".heic", ".heif"
    };

    // Define common compressed file extensions
    static const std::unordered_set<std::string> compressedExtensions = {
        ".zip", ".tar", ".gz", ".bz2", ".xz", ".7z", ".rar", ".zst", ".lz4", 
        ".tar.gz", ".tar.bz2", ".tar.xz", ".tgz", ".tbz2", ".txz", ".tar.zst", 
        ".tzst", ".tar.lz4", ".tlz4", ".jar", ".war", ".ear", ".cab", ".deb", 
        ".rpm", ".apk", ".dmg", ".iso", ".img", ".appimage"
    };

    // Get file extension and convert to lowercase
    std::string extension = entry.path().extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (entry.is_regular_file()) {
        if (programmingExtensions.count(extension)) return FileType::Programming;
        if (textExtensions.count(extension)) return FileType::Text;
        if (videoExtensions.count(extension)) return FileType::Video;
        if (pictureExtensions.count(extension)) return FileType::Picture;
        if (compressedExtensions.count(extension)) return FileType::Compressed;
        
        // Check if the file is executable
        auto perms = entry.status().permissions();
        if ((perms & fs::perms::owner_exec) != fs::perms::none) {
            return FileType::Executable;
        }
    }

    return FileType::Other;
}

// Get the appropriate color for a file type or directory
std::string getColor(FileType type, bool isDirectory = false) {
    if (isDirectory) return "\033[1;34m"; // Blue for directories

    switch (type) {
        case FileType::Programming: return "\033[0;36m";  // Cyan
        case FileType::Text: return "\033[0;32m";         // Green
        case FileType::Video: return "\033[0;35m";        // Magenta
        case FileType::Picture: return "\033[0;33m";      // Yellow
        case FileType::Compressed: return "\033[1;31m";   // Red
        case FileType::Hidden: return "\033[1;30m";       // Dark Gray
        case FileType::Executable: return "\033[1;36m";   // Bright Cyan
        default: return "\033[0m";                        // Default (white)
    }
}

// Convert a filesystem time to a time_t object
time_t to_time_t(const fs::file_time_type& ftime) {
    auto systemTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(systemTime);
}

// Calculate the total size of a directory (recursively)
std::uintmax_t calculateDirectorySize(const fs::path& path) {
    std::uintmax_t totalSize = 0;
    for (const auto& entry : fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
        if (entry.is_regular_file()) {
            totalSize += entry.file_size();
        }
    }
    return totalSize;
}

// Print a single file or directory entry with details
std::uintmax_t printEntry(const fs::directory_entry& entry, bool showTotalSize) {
    bool isDirectory = entry.is_directory();
    FileType type = isDirectory ? FileType::Other : categorizeFile(entry);

    // Check if the file or directory is hidden
    bool isHidden = (entry.path().filename().string()[0] == '.');

    // Assign emojis based on file type or folder status
    std::string emoji;
    if (isDirectory) {
        emoji = "📂 "; // Folder emoji
    } else {
        switch (type) {
            case FileType::Programming: emoji = "💻 "; break;
            case FileType::Text: emoji = "📜 "; break;
            case FileType::Video: emoji = "🎬 "; break;
            case FileType::Picture: emoji = "🖼️ "; break;
            case FileType::Executable: emoji = "⚙️ "; break;
            case FileType::Compressed: emoji = "🎁 "; break;
            default: emoji = "📄 "; break; // Other/Unknown files
        }
    }

    // Use red color for hidden files/directories
    std::string color = isHidden ? "\033[1;30m" : getColor(type, isDirectory);

    std::cout << emoji; // Print emoji before filename
    std::cout << color; // Use red color for hidden files/directories
    std::cout << std::setw(20) << std::left << entry.path().filename().string();

    std::uintmax_t size = 0;

    // Always show details
    std::cout << " " << getPermissions(entry.path());

    if (entry.is_regular_file()) {
        size = entry.file_size();
        std::cout << " " << std::setw(10) << formatSize(size);
    }

    if (isDirectory && showTotalSize) {
        // Only show directory size if -t is used and -r is NOT used
        size = calculateDirectorySize(entry.path()); // Calculate directory size here
        std::cout << " " << std::setw(10) << formatSize(size) << " (total)";
    }

    if (!isDirectory) {
        auto ftime = entry.last_write_time();
        auto sctp = to_time_t(ftime);
        std::cout << " " << std::put_time(std::localtime(&sctp), "%Y-%m-%d %H:%M:%S");
    }

    std::cout << "\033[0m" << std::endl; // Reset color
    return size; // Return the size of the entry
}

// Function to check if a file or directory is hidden
bool isHidden(const std::filesystem::directory_entry& entry) {
    return entry.path().filename().string().front() == '.';
}

// Helper function to convert a string to lowercase
std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

// Display directory contents in a multi-column format
void displayMultiColumn(const std::vector<std::filesystem::directory_entry>& entries) {
    // Get terminal width dynamically
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int maxWidth = w.ws_col > 0 ? w.ws_col : 80; // Fallback to 80 if detection fails

    const int columnWidth = 17; // Fixed width for each column
    int numColumns = maxWidth / (columnWidth + 1); // Calculate number of columns
    int numEntries = entries.size();
    int numRows = (numEntries + numColumns - 1) / numColumns; // Round up to fit all entries

    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numColumns; ++col) {
            int index = row * numColumns + col; // Calculate index for left-to-right order
            if (index < numEntries) {
                const auto& entry = entries[index];
                bool isDirectory = entry.is_directory();
                bool hidden = isHidden(entry); // Check if the file is hidden

                FileType type = isDirectory ? FileType::Other : categorizeFile(entry);

                // Assign emoji based on file type
                std::string emoji;
                if (isDirectory) emoji = "📂 ";
                else {
                    switch (type) {
                        case FileType::Programming: emoji = "💻 "; break;
                        case FileType::Text: emoji = "📜 "; break;
                        case FileType::Video: emoji = "🎬 "; break;
                        case FileType::Picture: emoji = "🖼️ "; break;
                        case FileType::Executable: emoji = "⚙️ "; break;
                        case FileType::Compressed: emoji = "🎁 "; break;
                        default: emoji = "📄 ";
                    }
                }

                // Apply color: Red for hidden files and directories
                std::string color = hidden ? "\033[1;30m" : getColor(type, isDirectory);

                // Truncate filenames that are too long (max 15 chars, excluding symbol)
                std::string name = entry.path().filename().string();
                const int maxNameLength = 15; // Fixed max length for filenames
                if (name.length() > maxNameLength) {
                    name = name.substr(0, maxNameLength - 1) + "\033[1;33m>\033[0m"; // Bright yellow ">"
                }

                // Print with color, emoji, and reset color, ensuring fixed column width
                std::cout << color << emoji << std::setw(columnWidth - 2) << std::left << name << "\033[0m";
            }
        }
        std::cout << std::endl; // Move to the next row
    }
}

// List directory contents with optional recursive and pattern matching
void listDirectoryContents(const fs::path& path, const std::string& pattern, 
    bool recursive, bool showTotalSize, 
    bool forceList, bool forceWide, 
    int& totalFiles, int& totalDirs, std::uintmax_t& totalSizeShown, const int& screenHeight) {

    std::vector<fs::directory_entry> directories; // Store directories
    std::vector<fs::directory_entry> files;       // Store files

    for (const auto& entry : fs::directory_iterator(path)) {
        // Skip entries that don't match the pattern
        if (!pattern.empty() && fnmatch(pattern.c_str(), entry.path().filename().c_str(), FNM_PATHNAME) != 0) {
            continue;
        }

        if (entry.is_directory()) {
            directories.push_back(entry); // Add directory to the list
            totalDirs++; // Increment directory count
            if (showTotalSize && !recursive) {
                // Calculate directory size only if -t is used and -r is NOT used
                std::uintmax_t dirSize = calculateDirectorySize(entry.path());
                totalSizeShown += dirSize; // Add directory size to total
            }
        } else {
            files.push_back(entry); // Add file to the list
            totalFiles++; // Increment file count
            totalSizeShown += entry.file_size(); // Add file size to total
        }
    }

    // Sort directories alphabetically (case-insensitive)
    std::sort(directories.begin(), directories.end(), [](const auto& a, const auto& b) {
        return toLower(a.path().filename().string()) < toLower(b.path().filename().string());
    });

    // Sort files by category and then alphabetically (case-insensitive)
    std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
        FileType typeA = categorizeFile(a);
        FileType typeB = categorizeFile(b);
        if (typeA != typeB) {
            return typeA < typeB; // Sort by category first
        }
        return toLower(a.path().filename().string()) < toLower(b.path().filename().string()); // Then alphabetically
    });

    // Combine directories and files into a single list for multi-column display
    std::vector<fs::directory_entry> allEntries;
    allEntries.insert(allEntries.end(), directories.begin(), directories.end());
    allEntries.insert(allEntries.end(), files.begin(), files.end());

    // Display in multi-column format if conditions are met
    if (!forceList && (forceWide || allEntries.size() > (screenHeight - 3))) {
        displayMultiColumn(allEntries);
    } else {
        // Display in detailed list format
        for (const auto& entry : allEntries) {
            printEntry(entry, showTotalSize && !recursive);
        }
    }

    // Recursively list subdirectories
    if (recursive) {
        for (const auto& dir : directories) {
            std::cout << "\n" << dir.path().string() << ":\n";
            listDirectoryContents(dir, pattern, recursive, showTotalSize, forceList, forceWide, totalFiles, totalDirs, totalSizeShown, screenHeight);
        }
    }
}

// Display an error message and usage instructions
void showError(const std::string& errorMessage) {
    const std::string red = "\033[31m";  // ANSI escape code for red text
    const std::string reset = "\033[0m"; // Reset to default color
    std::cout << red << "Error: " << reset << errorMessage << ". Try: c -h" << std::endl;
    std::exit(1); // Exit with error code
}

// Check if a directory exists
bool directoryExists(const std::string& path) {
    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

// Parse command-line arguments and handle errors
void parseTargets(int argc, char* argv[], std::string& dir, std::string& pattern, std::vector<std::string>& flags) {
    dir = ".";      // Default directory
    pattern = "*";  // Default pattern (match everything)

    if (argc == 1) {
        // If no arguments are provided, assume default behavior (like 'ls')
        return;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg[0] == '-') { 
            // Handle flags
            if (arg == "-r" || arg == "--recursive") flags.push_back(arg);
            else if (arg == "-t" || arg == "--total") flags.push_back(arg);
            else if (arg == "-l" || arg == "--list") flags.push_back(arg);
            else if (arg == "-w" || arg == "--wide") flags.push_back(arg);
            else if (arg == "-p" || arg == "--pause") flags.push_back(arg);
            else if (arg == "-h" || arg == "--help") flags.push_back(arg);
            else showError("Unknown flag: " + arg);
        } else if (arg.find('*') != std::string::npos || arg.find('?') != std::string::npos) { 
            // Handle patterns with wildcards
            if (pattern == "*") pattern = arg;
            else showError("Multiple patterns are not allowed.");
        } else { 
            // Handle directory or file path
            if (dir == ".") dir = arg;
            else showError("Multiple directories are not allowed.");
        }
    }

    // Validate the provided directory
    if (!directoryExists(dir)) {
        showError("Directory does not exist: " + dir);
    }
}

// Main function
int main(int argc, char* argv[]) {
    // Detect terminal height
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int screenHeight = w.ws_row > 0 ? w.ws_row : 24; // Default to 24 if detection fails

    std::string dir, pattern;
    std::vector<std::string> flags;

    parseTargets(argc, argv, dir, pattern, flags);

    // Initialize feature variables based on flags
    bool recursive = false, showTotalSize = false, forceList = false, forceWide = false, screenPause = false;

    for (const auto& flag : flags) {
        if (flag == "-r" || flag == "--recursive") recursive = true;
        else if (flag == "-t" || flag == "--total") showTotalSize = true;
        else if (flag == "-l" || flag == "--list") forceList = true;
        else if (flag == "-w" || flag == "--wide") forceWide = true;
        else if (flag == "-p" || flag == "--pause") screenPause = true;
        else if (flag == "-h" || flag == "--help") {
            showAboutScreen();
            return 0;
        }
    }

    // Initialize counters
    int totalFiles = 0, totalDirs = 0;
    std::uintmax_t totalSizeShown = 0;

    // List files in the specified directory
    listDirectoryContents(dir, pattern, recursive, showTotalSize, forceList, forceWide, totalFiles, totalDirs, totalSizeShown, screenHeight);

    // Display summary
    displaySummary(totalFiles, totalDirs, totalSizeShown);

    return 0;
}