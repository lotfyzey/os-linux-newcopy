# newcopy

A file-copy system program written in C.
OS Course 2027 - Task#1.

## Build

    gcc -Wall -Wextra -O2 newcopy.c -o newcopy

## Usage

    newcopy <SourceFile> <Destination>

The destination can be a new file name, an existing file, or a directory.

## Examples

    newcopy a.txt b.txt        # copy to a new file
    newcopy a.txt mydir/       # copy into a directory
    newcopy a.txt a.txt        # refused: same file

## How it works

1. Read the arguments (argc / argv).
2. Check the files with stat().
3. Open both files with open().
4. Copy in 64 KB chunks with read() and write().
5. Close both files and return the exit code.

## Safety checks

- Refuses to copy a file onto itself (device and inode are compared).
- Handles partial writes and interrupted calls (EINTR).
- Checks the result of close().
- Prints a clear error message and exits with code 1 on failure.

## Files

- `newcopy.c` - source code
- `newcopy - OS Task#1.pptx` - presentation

## Author

Zeyad Lotfy Abd Elghany
Faculty of Electronic Engineering, Minoufia University
