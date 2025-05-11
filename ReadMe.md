# ColorDir - A Colorful Directory Listing Tool for Linux

ColorDir is a Linux command-line tool that provides a colorful and emoji-enhanced directory listing, inspired by the classic HDIR tool for DOS. It categorizes files by type and displays them with appropriate colors and icons for better visual clarity.

## About

```
  ____      _            ____  _      _
 / ___|___ | | ___  _ __|  _ \(_)_ __| |
| |   / _ \| |/ _ \| '__| | | | | '__| |
| |__| (_) | | (_) | |  | |_| | | |  |_|
 \____\___/|_|\___/|_|  |____/|_|_|  (_)
           !!About ColorDir. v. beta 0.3
```

This program lists directory contents with color coding.

**History**:  
About 30 years ago, I discovered HDIR, a simple tool that brought color to my directory listings in DOS.  
I loved it then, and today, I set out to create a tribute to it: ColorDir.  
Not as a replacement for `ls` and its deeper functionalities, but as both a nostalgic homage and an aesthetically pleasing way to view files, wrapped in the colors of the past.

Best regards 💌 [Contact me on GitHub](https://github.com/Curvez)

## Features

- **Color-coded file types**: Different colors for programming files, text files, videos, pictures, compressed files, executables, and more.
- **Emoji support**: Adds emojis to represent file types and directories.
- **Recursive listing**: Option to list files and directories recursively.
- **Human-readable sizes**: Displays file and directory sizes in a readable format (e.g., KB, MB, GB).
- **Multi-column view**: Automatically adjusts to terminal width for a compact display.
- **Customizable patterns**: Supports wildcard patterns (`*`, `?`) for filtering files.
- **Error handling**: Graceful handling of invalid directories and unknown flags.

## Installation

To install, run the following command:

```bash
sudo wget -O /bin/c https://kervels.net/colordir/c && sudo chmod a+rx /bin/c
```

## Download

You can download the latest version of ColorDir from the [Releases page](https://github.com/CurveZ/Linux-ColorDir/releases/latest). Or
```bash
sudo wget -O c https://kervels.net/colordir/c
```

## Usage

```bash
c [flags] [directory] [pattern]
```

### Flags

- `-l, --list`       Force list view.
- `-w, --wide`       Force columns view.
- `-t, --total`      Display total size of directories and subdirectories.
- `-r, --recursive`  Recursive listing.
- `-p, --pause`      Pause after each screen of output.
- `-h, --help`       Display help information.

## Tested Systems

- Developed and tested on **Ubuntu Server 24.04 LTS**.  
- Likely compatible with other Linux distributions.

## To-Do

### Short-Term Goals

- Improve error handling:
  - Handle permission errors gracefully.
  - Ensure accurate file size retrieval.
- Visual feedback:
  - Display a red line above the summary when an error occurs.
  - Log detailed errors to the system journal.
  - Add explanation of journal usage in the `--help` section.
- Pattern matching:
  - Scan inside directories for matching files and folders, even if the directory name itself does not match the search pattern.
- `-p` flag:
  - Review or refine behavior as needed.
- Emoji support in filenames:
  - Investigate and adjust string handling for emoji characters.

### Long-Term Goals

- Windows version.
- macOS compatibility.
- WSL support or dedicated version.


## Acknowledgments

ColorDir is a tribute to the classic MS-DOS HotDir tool, bringing its nostalgic charm to modern Linux systems.

---

Visit the project webpage: [https://kervels.net/colordir](https://kervels.net/colordir)